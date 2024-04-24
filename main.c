#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "buddy.h"

int main(void) {
    void *ptr1, *ptr2, *ptr3;
    void *bd = buddy_new(64, 144 * 1024, NULL, 1);
    ptr1 = buddy_alloc(bd, 4096 * 16);
    buddy2_dump(bd);
    ptr2 = buddy_alloc(bd, 4096 * 1);
    buddy2_dump(bd);
    ptr3 = buddy_alloc(bd, 4096 * 4);
    buddy_free(bd, ptr1);
    buddy2_dump(bd);
    buddy_free(bd, ptr3);
    buddy2_dump(bd);
    buddy_free(bd, ptr2);
    buddy2_dump(bd);
    buddy_free(bd, ptr2);
    buddy2_dump(bd);
    return 0;
}
