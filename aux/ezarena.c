#define EZ_IMPLEMENTATION
#include "ezarena.h"
#include <stdio.h>

typedef struct {
    int dummy;
} TestStruct;

int main(void) {
    ezArena arena = {
        .head = NULL,
        .tail = NULL
    };

    TestStruct* test = ezArenaAlloc(&arena, sizeof(TestStruct));
    test->dummy = 42;
    assert(test);
    printf("%d\n", test->dummy); // 42
    ezArenaFree(&arena, (void*)test);
    printf("%d\n", test->dummy); // segfault

    return 0;
}
