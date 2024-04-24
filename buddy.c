//
// Created by JohnLin on 2024/4/24.
//

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "buddy.h"

struct buddy {
    void *buf;
    uint32_t buf_size;
    uint16_t unit_shift;
    uint16_t split_shift;
    uint8_t info[1];
};

#define LEFT_LEAF(x)                (((x) << 1))
#define RIGHT_LEAF(x)               (((x) << 1) | 1)
#define PARENT(x)                   ((x) >> 1)

#define IS_POWER_OF_2(x)            (!((x) & ((x) - 1)))
#define NEXT_POWER_OF_2_SHIFT(x)    (32 - clz((x) - 1))
#define NEXT_POWER_OF_2(x)          (1 << NEXT_POWER_OF_2_SHIFT(x))
#define MAX(a, b)                   ((a) > (b)) ? (a) : (b);


uint32_t clz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t n = 0;
    if (x <= 0x0000FFFF) {
        n += 16;
        x <<= 16;
    }
    if (x <= 0x00FFFFFF) {
        n += 8;
        x <<= 8;
    }
    if (x <= 0x0FFFFFFF) {
        n += 4;
        x <<= 4;
    }
    if (x <= 0x3FFFFFFF) {
        n += 2;
        x <<= 2;
    }
    if (x <= 0x7FFFFFFF) { n += 1; }
    return n;
}

void *buddy_new(uint32_t split, uint32_t buf_size, void *buf, uint32_t align) {
    struct buddy *bd;

    if (buf_size <= 0) {
        return NULL;
    }

    uint32_t split_shift = NEXT_POWER_OF_2_SHIFT(split);
    uint32_t unit_shift = NEXT_POWER_OF_2_SHIFT(buf_size) - split_shift;
    uint32_t need = buf_size >> unit_shift;
    uint32_t n;

    bd = (struct buddy *) malloc(sizeof(struct buddy) + (1 << (split_shift + 1)));
    bd->buf = buf;
    bd->buf_size = buf_size;
    bd->split_shift = split_shift;

    n = (1 << (split_shift + 1));

    if (!IS_POWER_OF_2(need) && align) {
        bd->unit_shift = unit_shift;
    } else {
        need = split;
        bd->unit_shift = 0;
    }

    bd->info[0] = 0xff;

    for (int i = (n >> 1); i < n; i++) {
        if (need > 0) {
            bd->info[i] = 1;
            need--;
        } else {
            bd->info[i] = 0;
        }
    }

    uint32_t match = 1;
    for (int i = (n >> 1) - 1; i >= 1; i--) {
        if (bd->info[LEFT_LEAF(i)] == match && bd->info[RIGHT_LEAF(i)] == match) {
            bd->info[i] = match + 1;
        } else {
            bd->info[i] = MAX(bd->info[LEFT_LEAF(i)], bd->info[RIGHT_LEAF(i)]);
        }

        if (IS_POWER_OF_2(i)) {
            match++;
        }
    }

    return bd;
}


void *buddy_alloc(void *ctx, uint32_t size) {
    struct buddy *bd = ctx;
    void *ptr = NULL;
    uint32_t index = 1;
    uint32_t offset;
    uint32_t unit, need;
    uint32_t remain;
    uint32_t min_index;

    if (ctx == NULL || size == 0)
        return ptr;

    unit = bd->unit_shift ? (1 << bd->unit_shift) : (bd->buf_size >> bd->split_shift);
    need = (size + unit - 1) / unit;
    need = NEXT_POWER_OF_2_SHIFT(need) + 1;
    if (bd->info[index] < need)
        return ptr;

    for (remain = bd->split_shift + 1; remain > need; remain--) {
        min_index = bd->info[LEFT_LEAF(index)] >= bd->info[RIGHT_LEAF(index)] ? RIGHT_LEAF(index) : LEFT_LEAF(index);

        if (bd->info[min_index] >= need) {
            index = min_index;
        } else {
            index = min_index ^ 1;
        }
    }

    assert(index > 0);
    assert(remain > 0);

    bd->info[index] = 0;
    offset = index * (1 << (remain - 1)) - (1 << bd->split_shift);

    while (index) {
        index = PARENT(index);
        bd->info[index] = MAX(bd->info[LEFT_LEAF(index)], bd->info[RIGHT_LEAF(index)]);
    }

    ptr = (void *) ((uint8_t *) (bd->buf) + (offset * unit));

    return ptr;
}

void buddy_free(void *ctx, void *ptr) {
    struct buddy *bd = ctx;
    uint32_t unit = bd->unit_shift ? (1 << bd->unit_shift) : (bd->buf_size >> bd->split_shift);
    uint32_t offset = ((uintptr_t) (ptr) - (uintptr_t) bd->buf) / unit;
    uint32_t match = 1;
    uint32_t index = offset + (1 << bd->split_shift);

    assert(bd && offset >= 0 && offset < (1 << bd->split_shift));

    for (; bd->info[index]; index = PARENT(index)) {
        match++;

        if (index == 1) {
            printf("double free\n");
            return;
        }
    }

    bd->info[index] = match;
    while (index > 1) {
        index = PARENT(index);

        if (bd->info[LEFT_LEAF(index)] == match && bd->info[RIGHT_LEAF(index) == match]) {
            bd->info[index] = match + 1;
        } else {
            bd->info[index] = MAX(bd->info[LEFT_LEAF(index)], bd->info[RIGHT_LEAF(index)]);
        }

        match++;
    }
}

void buddy2_dump(void *ctx) {
    struct buddy *bd = ctx;

    char canvas[129];
    int i, j;
    uint32_t node_shift, offset;

    if (bd == NULL) {
        printf("buddy2_dump: (struct buddy2*)self == NULL");
        return;
    }

    memset(canvas, '_', sizeof(canvas));
    node_shift = bd->split_shift + 1;
    uint32_t half = (1 << bd->split_shift);
    for (i = 1; i < 1 << (bd->split_shift + 1); ++i) {
        if (IS_POWER_OF_2(i))
            node_shift--;

        if (bd->info[i] == 0) {
            if (i >= half) {
                canvas[i - half] = '*';
            } else {
                offset = i * (1 << node_shift) - half;
                for (j = offset; j < offset + (1 << node_shift); ++j)
                    canvas[j] = '*';
            }
        }
    }

    canvas[1 << bd->split_shift] = '\0';
    puts(canvas);
}
