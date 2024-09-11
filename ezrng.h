/* ezrng.h -- https://github.com/takeiteasy/ez
 
 ezrng -- Simple pseudo-random number generation
 
 Copyright (C) 2024  George Watson
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef EZRNG_HEADER
#define EZRNG_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <time.h>

#if !defined(EZ_MALLOC)
#define EZ_MALLOC malloc
#endif
#if !defined(EZ_FREE)
#define EZ_FREE free
#endif

typedef struct ezRng {
    unsigned int seed;
    int p1, p2;
    unsigned int buffer[17];
} ezRng;

ezRng* ezRngNew(unsigned int s);
#define ezRngFree(R) EZ_FREE((R))

unsigned int ezRngBits(ezRng *r);
float ezRngFloat(ezRng *r);
double ezRngDouble(ezRng *r);
#define ezRngInt(R, __MAX) (ezRngBits((R)) % __MAX)

#define ezRngFloatRange(R, __MIN, __MAX) (ezRngFloat((R)) * ((__MAX) - (__MIN)) + (__MIN))
#define ezRngDoubleRange(R, __MIN, __MAX) (ezRngDouble((R)) * ((__MAX) - (__MIN)) + (__MIN))
#define ezRngIntRange(R, __MIN, __MAX) (ezRngBits((R)) % ((__MAX) + 1 - (__MIN)) + (__MIN))

#if defined(__cplusplus)
}
#endif
#endif // EZRNG_HEADER

#if defined(EZRNG_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
ezRng* ezRngNew(unsigned int s) {
    ezRng *r = EZ_MALLOC(sizeof(ezRng));
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

unsigned int ezRngBits(ezRng *r) {
    unsigned int result = r->buffer[r->p1] = ROTL(r->buffer[r->p2], 13) + ROTL(r->buffer[r->p1], 9);

    if (--r->p1 < 0)
        r->p1 = 16;
    if (--r->p2 < 0)
        r->p2 = 16;

    return result;
}

float ezRngFloat(ezRng *r) {
    union {
        float value;
        unsigned int word;
    } convert = {
        .word = (ezRngBits(r) >> 9) | 0x3F800000};
    return convert.value - 1.f;
}

double ezRngDouble(ezRng *r) {
    unsigned int bits = ezRngBits(r);
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
