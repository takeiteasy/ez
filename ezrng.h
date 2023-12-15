/* ezrng.h - Simple pseudo random number generation
   https://github.com/takeiteasy/
  
 The MIT License (MIT)
 Copyright (c) 2022 George Watson
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef EZRNG_HEADER
#define EZRNG_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <time.h>

typedef struct ezRandom {
    unsigned int seed;
    int p1, p2;
    unsigned int buffer[17];
} ezRandom;

ezRandom* ezRandomNew(unsigned int s);
#define ezRandomFree(R) free((R))

unsigned int ezRandomBits(ezRandom *r);
float ezRandomFloat(ezRandom *r);
double ezRandomDouble(ezRandom *r);
#define ezRandomInt(R, __MAX) (ezRandomBits((R)) % __MAX)

#define ezRandomFloatRange(R, __MIN, __MAX) (ezRandomFloat((R)) * ((__MAX) - (__MIN)) + (__MIN))
#define ezRandomDoubleRange(R, __MIN, __MAX) (ezRandomDouble((R)) * ((__MAX) - (__MIN)) + (__MIN))
#define ezRandomIntRange(R, __MIN, __MAX) (ezRandomBits((R)) % ((__MAX) + 1 - (__MIN)) + (__MIN))

#if defined(__cplusplus)
}
#endif
#endif // EZRNG_HEADER

#if defined(EZRNG_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
ezRandom* ezRandomNew(unsigned int s) {
    ezRandom *r = malloc(sizeof(ezRandom));
    if (!s)
        s = (unsigned int)time(NULL);
    r->seed = s;
    for (int i = 0; i < 17; i++) {
        s = s * 0xAC564B05 + 1;
        r->buffer[i] = s;
    }
    r->p1 = 0;
    r->p2 = 10;
    return r;
}

#if defined(ROTL)
#undef ROTL
#endif
#define ROTL(N, R) (((N) << (R)) | ((N) >> (32 - (R))))

unsigned int ezRandomBits(ezRandom *r) {
    unsigned int result = r->buffer[r->p1] = ROTL(r->buffer[r->p2], 13) + ROTL(r->buffer[r->p1], 9);

    if (--r->p1 < 0)
        r->p1 = 16;
    if (--r->p2 < 0)
        r->p2 = 16;

    return result;
}

float ezRandomFloat(ezRandom *r) {
    union {
        float value;
        unsigned int word;
    } convert = {
        .word = (ezRandomBits(r) >> 9) | 0x3F800000};
    return convert.value - 1.f;
}

double ezRandomDouble(ezRandom *r) {
    unsigned int bits = ezRandomBits(r);
    union {
        double value;
        unsigned int words[2];
    } convert = {
        .words = {
            bits << 20,
            (bits >> 12) | 0x3FF00000}};
    return convert.value - 1.0;
}
#endif
