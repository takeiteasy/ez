#define EZRNG_IMPLEMENTATION
#include "ezrng.h"
#include <stdio.h>
#include <assert.h>

int main(int argc, const char *argv[]) {
    ezRandom *a = ezRandomNew(500);
    ezRandom *b = ezRandomNew(500);
    ezRandom *c = ezRandomNew(100);
    
    for (int i = 0; i < 100; i++) {
#define TEST(TYPE, FN, FMT)                      \
do {                                             \
    TYPE _a = FN(a);                             \
    TYPE _b = FN(b);                             \
    TYPE _c = FN(c);                             \
    assert(_a == _b && _a != _c && _b != _c);    \
    printf("%d: " FMT ", " FMT "\n", i, _a, _c); \
} while(0)
        TEST(unsigned int, ezRandomBits, "%d");
        TEST(float, ezRandomFloat, "%f");
        TEST(double, ezRandomDouble, "%f");
    }
    
    ezRandomFree(a);
    ezRandomFree(b);
    ezRandomFree(c);
    return 0;
}
