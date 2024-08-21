#define EZRNG_IMPLEMENTATION
#include "ezrng.h"
#include <stdio.h>
#include <assert.h>

int main(int argc, const char *argv[]) {
    ezRng *a = ezRngNew(500);
    ezRng *b = ezRngNew(500);
    ezRng *c = ezRngNew(100);
    
    for (int i = 0; i < 100; i++) {
#define TEST(TYPE, FN, FMT)                      \
do {                                             \
    TYPE _a = FN(a);                             \
    TYPE _b = FN(b);                             \
    TYPE _c = FN(c);                             \
    assert(_a == _b && _a != _c && _b != _c);    \
    printf("%d: " FMT ", " FMT "\n", i, _a, _c); \
} while(0)
        TEST(unsigned int, ezRngBits, "%d");
        TEST(float, ezRngFloat, "%f");
        TEST(double, ezRngDouble, "%f");
    }
    
    ezRngFree(a);
    ezRngFree(b);
    ezRngFree(c);
    return 0;
}
