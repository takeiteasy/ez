/* ezvector.h -- Stretchy buffer implementation
 https://github.com/takeiteasy/
 
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

#define __vector_raw(a)         ((int *) (void *) (a) - 2)
#define __vector_m(a)           __vector_raw(a)[0]
#define __vector_n(a)           __vector_raw(a)[1]
#define __vector_needgrow(a,n)  ((a)==0 || __vector_n(a)+(n) >= __vector_m(a))
#define __vector_maybegrow(a,n) (__vector_needgrow(a,(n)) ? __vector_grow(a,n) : 0)
#define __vector_grow(a,n)      (*((void **)&(a)) = __vector_growf((a), (n), sizeof(*(a))))

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
#define ezVectorFree(a)        ((a) ? free(__vector_raw(a)),((a)=NULL) : 0)
// Append an element to the end of a vector
#define ezVectorPush(a,v)      (__vector_maybegrow(a,1), (a)[__vector_n(a)++] = (v))
#define MAP_VECTOR_PUSH(v, a)  (ezVectorPush(a, v))
// Append N elements to the end of a vector
#define ezVectorAppend(a, ...) (MAP_LIST_UD(MAP_VECTOR_PUSH, a, __VA_ARGS__))
// Number of elements in a vector
#define ezVectorCount(a)       ((a) ? __vector_n(a) : 0)
// Reserve empty spaces in a vector
#define ezVectorReserve(a,n)   (__vector_maybegrow(a,n), __vector_n(a)+=(n), &(a)[__vector_n(a)-(n)])
// Last element of vector
#define ezVectorLast(a)        ((a)[__vector_n(a)-1])
// Remove an element of vector at index
#define ezVectorRemove(a, idx)      \
    do                              \
    {                               \
        (a)[idx] = ezVectorLast(a); \
        --__vector_n(a);            \
    } while (0)
// Remove last element of vector
#define ezVectorPop(a) --__vector_n(a)
// Clear all elements from vector but don't free
#define ezVectorClear(a) ((a) ? (__vector_n(a) = 0) : 0)
// Iterator through each element of array, v = element, i = index
#define ezVectorEach(t, a, ...)                           \
    do {                                                  \
        t v = a[0];                                       \
        for (int i = 0; i < ezVectorCount(a); v = a[++i]) \
            __VA_ARGS__                                   \
    } while (0)
// Insert element into vector at index
#define ezVectorInsert(a, v, _index)      \
    do                                    \
    {                                     \
        if ((_index) >= ezVectorCount(a)) \
            ezVectorPush(a, v);           \
        else                              \
            (a)[(_index)] = (v);          \
    } while (0)
// Randomly shuffle vector (uses rand(), call srand before use)
#define ezVectorShuffle(t, a)       \
do {                                \
    int i, j, n = ezVectorCount(a); \
    t tmp;                          \
    for (i = n - 1; i > 0; i--) {   \
        j = rand() % (i + 1);       \
        tmp  = a[j];                \
        a[j] = a[i];                \
        a[i] = tmp;                 \
    }                               \
} while(0)

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
// Create a new vector with N elements
#define ezVectorInit(t, ...)                 \
    ^(void) {                                \
        t* result = NULL;                    \
        ezVectorAppend(result, __VA_ARGS__); \
        return result;                       \
    }()

// Iterate through vector and apply closure to each, return new vector
#define ezVectorMap(t, a, f)                        \
    ^(t* _a) {                                      \
        t* result = NULL;                           \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            ezVectorPush(result, ^(t v)f(_a[i]));   \
        return result;                              \
    }(a)

// Same as ezVectorMap but includes an index (i)
#define ezVectorMapWithIndex(t, a, f)                         \
    ^(t* _a) {                                                \
        t* result = NULL;                                     \
        for (int _i = 0; _i < ezVectorCount(_a); _i++)        \
            ezVectorPush(result, ^(t v, int i)f(_a[_i], _i)); \
        return result;                                        \
    }(a)

// Reduce vector down to single value
#define ezVectorReduce(t, i, a, f)                      \
    ^(t* _a) {                                          \
        t result = i;                                   \
        for (int _i = 0; _i < ezVectorCount(_a); _i++)  \
            result = ^(t result, t v)f(result, _a[_i]); \
        return result;                                  \
    }(a)

// Returns index of first matching element (-1 == not found)
#define ezVectorFind(t, a, f)                       \
    ^(t* _a) {                                      \
        int result = -1;                            \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (^(t v)f(_a[i]) == 1) {              \
                result = i;                         \
                break;                              \
            }                                       \
        return result;                              \
    }(a)

// Filter vector with closure, returns matching elements
#define ezVectorFilter(t, a, f)                     \
    ^(t* _a) {                                      \
        t* result = NULL;                           \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (^(t v)f(_a[i]) == 1)                \
                ezVectorPush(result, _a[i]);        \
        return result;                              \
    }(a)

// Opposite of ezVectorFilter
#define ezVectorReject(t, a, f)                     \
    ^(t* _a) {                                      \
        t* result = NULL;                           \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (^(t v)f(_a[i]) == 0)                \
                ezVectorPush(result, _a[i]);        \
        return result;                              \
    }(a)

// Returns true only if all elements that pass closure test
#define ezVectorEvery(t, a, f)                      \
    ^(t* _a) {                                      \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (^(t v)f(_a[i]) == 0)                \
                return 0;                           \
        return 1;                                   \
    }(a)

// Returns true/false if vector contains value (== check)
#define ezVectorIncludes(t, a, v)                   \
    ^(t* _a) {                                      \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (_a[i] == (v))                       \
                return 1;                           \
        return 0;                                   \
    }(a)

// Returns true/false if vector contains value (closure match)
#define ezVectorIncludesWhere(t, a, f)              \
    ^(t* _a) {                                      \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (^(t v)f(_a[i]) == 1)                \
                return 1;                           \
        return 0;                                   \
    }(a)

// Returns new vector with N random samples (no dupes)
#define ezVectorSample(t, a, n)                    \
    ^(t* _a) {                                     \
        t* result = NULL;                          \
        int l = ezVectorCount(a);                  \
        if (n > l || !n || !l)                     \
            return result;                         \
        if (n == l)                                \
            return a;                              \
        int x[n], j = 0;                           \
        for (int i = 0; i < n; i++)                \
            x[i] = 0;                              \
        for (int i = 0; i < n; i++) {              \
            int y = rand() / (RAND_MAX / n + 1);   \
            int alreadyFound = 0;                  \
            for (int k = 0; k < j; k++)            \
                if (x[k] == y) {                   \
                    alreadyFound = 1;              \
                    break;                         \
                }                                  \
            if (alreadyFound)                      \
                i--;                               \
            else {                                 \
                ezVectorPush(result, _a[y]);       \
                x[j++] = y;                        \
            }                                     \
        }                                         \
        return result;                            \
    }(a)

// Return new sub-vector from min-max range
#define ezVectorSlice(t, a, min, max)       \
    ^(t* _a, int _min, int _max) {          \
        t* result = NULL;                   \
        if (_min < 0) _min = 0;             \
        int l = ezVectorCount(_a);          \
        if (_max > l) _max = l;             \
        if (_min >= _max)                   \
            return result;                  \
        for (int i = _min; i < _max; i++)   \
            ezVectorPush(result, _a[i]);    \
        return result;                      \
    }(a, min, max)

// Return a new copy of a vector
#define ezVectorCopy(t, a)          \
    ^(t* _a) {                      \
        t* result = NULL;           \
        int l = ezVectorCount(_a);  \
        ezVectorReserve(result, l); \
        for (int i = 0; i < l; i++) \
            result[i] = _a[i];      \
        return result;              \
    }(a)

// Return new vector without any duplicates (== check)
#define ezVectorUniq(t, a)                        \
    ^(t* _a) {                                    \
        t* result = NULL;                         \
        int i, j;                                 \
        for (i = 0; i < ezVectorCount(_a); i++) { \
            for (j = 0; j < i; j++)               \
                if (_a[i] == _a[j])               \
                    break;                        \
            if (i == j)                           \
                ezVectorPush(result, _a[i]);      \
        }                                         \
        return result;                            \
    }(a)

// Returns new vector without any duplicates (closure check)
#define ezVectorUniqWhere(t, a, f)                     \
    ^(t* _a) {                                         \
        t* result = NULL;                              \
        int i, j;                                      \
        for (i = 0; i < ezVectorCount(_a); i++) {      \
            for (j = 0; j < i; j++)                    \
                if (^(t v1, t v2)f(_a[i], _a[j]) == 1) \
                    break;                             \
            if (i == j)                                \
                ezVectorPush(result, _a[i]);           \
        }                                              \
        return result;                                 \
    }(a)

// Returns new vector without last element
#define ezVectorInitial(t, a)        \
    ^(t* _a) {                       \
        t* result = NULL;            \
        int l = ezVectorCount(_a)-1; \
        ezVectorReserve(result, l);  \
        for (int i = 0; i < l; i++)  \
            result[i] = _a[i];       \
        return result;               \
    }(a)

// Returns new vector without first element
#define ezVectorRest(t, a)            \
    ^(t* _a) {                        \
        t* result = NULL;             \
        int l = ezVectorCount(_a);    \
        ezVectorReserve(result, l-1); \
        for (int i = 1; i < l; i++)   \
            result[i-1] = _a[i];      \
        return result;                \
    }(a)

// Return new vector with elements that are not present in other array (== check)
#define ezVectorDifference(t, a, b)                 \
    ^(t* _a, t* _b) {                               \
        t* result = NULL;                           \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (!ezVectorIncludes(t, _b, _a[i])     \
                ezVectorPush(result, _a[i]);        \
        return result;                              \
    }(a, b)

// Return new vector with elements that are not present in other array (closure check)
#define ezVectorDifferenceWhere(t, a, b, f)             \
    ^(t* _a, t* _b) {                                   \
        t* result = NULL;                               \
        for (int i = 0; i < ezVectorCount(_a); i++)     \
            if (!ezVectorIncludesWhere(t, _b, _a[i], f) \
                ezVectorPush(result, _a[i]);            \
        return result;                                  \
    }(a, b)

// Opposite of ezVectorDifference (== check)
#define ezVectorIntersection(t, a, b)               \
    ^(t* _a, t* _b) {                               \
        t* result = NULL;                           \
        for (int i = 0; i < ezVectorCount(_a); i++) \
            if (ezVectorIncludes(t, _b, _a[i])      \
                ezVectorPush(result, _a[i]);        \
        return result;                              \
    }(a, b)

// Opposite of ezVectorDifference (Closure check)
#define ezVectorIntersectionWhere(t, a, b, f)          \
    ^(t* _a, t* _b) {                                  \
        t* result = NULL;                              \
        for (int i = 0; i < ezVectorCount(_a); i++)    \
            if (ezVectorIncludesWhere(t, _b, _a[i], f) \
                ezVectorPush(result, _a[i]);           \
        return result;                                 \
    }(a, b)

// Aliases
#define ezVectorForEach ezVectorEach
#define ezVectorDupe ezVectorCopy
#define ezVectorClone ezVectorCopy
#define ezVectorCollect ezVectorMap
#define ezVectorCollectWithIndex ezVectorMapWithIndex
#define ezVectorInject ezVectorReduce
#define ezVectorFoldl ezVectorReduce
#define ezVectorDetect ezVectorFind
#define ezVectorSelect ezVectorFilter
#define ezVectorRemoveIf ezVectorReject
#define ezVectorAll ezVectorEvery
#define ezVectorContains ezVectorInclude
#define ezVectorContainsWhere ezVectorIncludesWhere
#define ezVectorUnique ezVectorUniq
#define ezVectorDiff ezVectorDifference
#define ezVectorDiffWhere ezVectorDifferenceWhere
#define ezVectorIntersect ezVectorIntersection
#define ezVectorIntersectWhere ezVectorIntersectionWhere
#endif

#if defined(__cplusplus)
}
#endif
#endif // EZVECTOR_HEADER

#if defined(EZVECTOR_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
static void *__vector_growf(void *arr, int increment, int itemsize) {
    int dbl_cur = arr ? 2 * __vector_m(arr) : 0;
    int min_needed = ezVectorCount(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = realloc(arr ? __vector_raw(arr) : 0, itemsize * m + sizeof(int) * 2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p + 2;
    } else {
#ifdef VECTOR_OUT_OF_MEMORY
        VECTOR_OUT_OF_MEMORY;
#endif
        return (void *)(2 * sizeof(int)); // try to force a NULL pointer exception later
    }
}
#endif
