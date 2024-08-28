/* ezvector.h -- Stretchy buffer implementation
 https://github.com/takeiteasy/ez
 
 Based off stretchy_buffer.h by Sean Barrett (nothings/sbt)
 Map macro taken from https://github.com/swansontec/map-macro
 
 Some functionality relies on Clang + GCC extensions
 
 --------------------------------------------------------------------
 
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

#ifndef EZVECTOR_HEADER
#define EZVECTOR_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

#ifndef EZ_REALLOC
#define EZ_REALLOC realloc
#endif
#ifndef EZ_FREE
#define EZ_FREE free
#endif

#define __ezvector_raw(a)         ((int *) (void *) (a) - 2)
#define __ezvector_m(a)           __ezvector_raw(a)[0]
#define __ezvector_n(a)           __ezvector_raw(a)[1]
#define __ezvector_needgrow(a,n)  ((a)==0 || __ezvector_n(a)+(n) >= __ezvector_m(a))
#define __ezvector_maybegrow(a,n) (__ezvector_needgrow(a,(n)) ? __ezvector_grow(a,n) : 0)
#define __ezvector_grow(a,n)      (*((void **)&(a)) = __ezvector_growf((a), (n), sizeof(*(a))))

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0_UD(f, userdata, x, peek, ...) f(x, userdata) MAP_LIST_NEXT(peek, MAP_LIST1_UD)(f, userdata, peek, __VA_ARGS__)
#define MAP_LIST1_UD(f, userdata, x, peek, ...) f(x, userdata) MAP_LIST_NEXT(peek, MAP_LIST0_UD)(f, userdata, peek, __VA_ARGS__)

#define MAP_LIST_UD(f, userdata, ...) EVAL(MAP_LIST1_UD(f, userdata, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Free vector and assign to NULL
#define ezVectorFree(a)        ((a) ? EZ_FREE(__ezvector_raw(a)),((a)=NULL) : 0)
// Append an element to the end of a vector
#define ezVectorPush(a,v)      (__ezvector_maybegrow(a,1), (a)[__ezvector_n(a)++] = (v))
#define MAP_VECTOR_PUSH(v, a)  (ezVectorPush(a, v))
// Append N elements to the end of a vector
#define ezVectorAppend(a, ...) (MAP_LIST_UD(MAP_VECTOR_PUSH, a, __VA_ARGS__))
// Number of elements in a vector
#define ezVectorCount(a)       ((a) ? __ezvector_n(a) : 0)
// Reserve empty spaces in a vector
#define ezVectorReserve(a,n)   (__ezvector_maybegrow(a,n), __ezvector_n(a)+=(n), &(a)[__ezvector_n(a)-(n)])
// Last element of vector
#define ezVectorLast(a)        ((a)[__ezvector_n(a)-1])
// Remove an element of vector at index
#define ezVectorRemove(a, idx)      \
    do                              \
    {                               \
        (a)[idx] = ezVectorLast(a); \
        --__ezvector_n(a);          \
    } while (0)
// Remove last element of vector
#define ezVectorPop(a) --__ezvector_n(a)
// Clear all elements from vector but don't free
#define ezVectorClear(a) ((a) ? (__ezvector_n(a) = 0) : 0)

void *__ezvector_growf(void *arr, int increment, int itemsize);

#if defined(__cplusplus)
}
#endif
#endif // EZVECTOR_HEADER

#if defined(EZVECTOR_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
void *__ezvector_growf(void *arr, int increment, int itemsize) {
    int dbl_cur = arr ? 2 * __ezvector_m(arr) : 0;
    int min_needed = ezVectorCount(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = EZ_REALLOC(arr ? __ezvector_raw(arr) : 0, itemsize * m + sizeof(int) * 2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p + 2;
    } else
        return (void*)(2 * sizeof(int));
}
#endif
