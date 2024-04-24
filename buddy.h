//
// Created by JohnLin on 2024/4/24.
//

#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>

void *buddy_new(uint32_t split, uint32_t buf_size, void *buf, uint32_t align);
void *buddy_alloc(struct buddy *ctx, uint32_t size);
void buddy_free(void *ctx, void *ptr);
void buddy2_dump(void *ctx);

#endif //BUDDY_H
