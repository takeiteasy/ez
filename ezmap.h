/* ezmap.h - Simple key value map + dictionary 
   https://github.com/takeiteasy/
  
 The MIT License (MIT)
 
 Copyright (c) 2023 George Watson
 
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
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 This library is based off https://github.com/billziss-gh/imap/
 Copyright (c) 2023 Bill Zissimopoulos. All rights reserved.
 
 MurmurHash3 was written by Austin Appleby, and is placed in the public
 domain. The author hereby disclaims copyright to this source code.
 
 */

#ifndef EZMAP_HEADER
#define EZMAP_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if !defined(EZ_MALLOC)
#define EZ_MALLOC malloc
#endif
#if !defined(EZ_FREE)
#define EZ_FREE free
#endif

#ifndef EZMAP_DEFAULT_CAPACITY
#define EZMAP_DEFAULT_CAPACITY 8
#endif

typedef struct imap_node_t imap_node_t;

typedef struct {
    imap_node_t *tree;
    size_t count, capacity;
} ezKeyMap;

typedef struct {
    uint64_t key;
    void *val;
} ezKeyValuePair;

ezKeyMap* ezKeyMapNew(ezKeyMap *old, size_t capacity);
int ezKeyMapSet(ezKeyMap *map, uint64_t key, void *value);
void* ezKeyMapGet(ezKeyMap *map, uint64_t key);
void* ezKeyMapDel(ezKeyMap *map, uint64_t key);
int ezKeyMapEach(ezKeyMap *map, int(*callback)(ezKeyValuePair *pair, uint64_t, void*), void *userdata);
void ezKeyMapDestroy(ezKeyMap *map);

typedef ezKeyMap ezDictionary;
typedef ezKeyMap ezDict;

#define ezDictNew(...) ezKeyMapNew(__VA_ARGS__)
int ezDictSet(ezDict *dict, const char *key, void *value);
void* ezDictGet(ezDict *dict, const char *key);
void* ezDictDel(ezDict *dict, const char *key);
#define ezDictEach(...) ezKeyMapIter(__VA_ARGS__)
#define ezDictDestroy(...) ezKeyMapDestroy(__VA_ARGS__)

uint64_t MurmurHash(const void *data, size_t len, uint32_t seed);

#if !defined(EZMAP_DISABLE_GENERICS)
typedef ezKeyMap ezMap;

#define ezMapNew(...) ezKeyMapNew(__VA_ARGS__)
#define ezMapSet(Hash, KEY, VAL) _Generic((KEY),                \
                                         int: ezKeyMapSet,      \
                                         uint64_t: ezKeyMapSet, \
                                         char*: ezDictSet,      \
                                         const char*: ezDictSet \
                                        )(Hash, KEY, VAL)
#define ezMapGet(Hash, KEY) _Generic((KEY),                     \
                                    int: ezKeyMapGet,           \
                                    uint64_t: ezKeyMapGet,      \
                                    char*: ezDictGet,           \
                                    const char*: ezDictGet      \
                                   )(Hash, KEY)
#define ezMapDel(Hash, KEY) _Generic((KEY),                     \
                                    int: ezKeyMapDel,           \
                                    uint64_t: ezKeyMapDel,      \
                                    char*: ezDictDel,           \
                                    const char*: ezDictDel      \
                                   )(Hash, KEY)
#define ezMapEach(...) ezKeyMapEach(__VA_ARGS__)
#define ezMapDestroy(...) ezKeyMapDestroy(__VA_ARGS__)
#endif

#if defined(__cplusplus)
}
#endif
#endif // EZMAP_HEADER

#if defined(EZMAP_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
#include <assert.h>

struct imap_node_t {
    union {
        uint32_t vec32[16];
        uint64_t vec64[8];
    };
};

#define imap__tree_root__           0
#define imap__tree_resv__           1
#define imap__tree_mark__           2
#define imap__tree_size__           3
#define imap__tree_nfre__           4
#define imap__tree_vfre__           5

#define imap__slot_pmask__          0x0000000f
#define imap__slot_node__           0x00000010
#define imap__slot_scalar__         0x00000020
#define imap__slot_value__          0xffffffe0
#define imap__slot_shift__          6
#define imap__slot_boxed__(sval)    (!((sval) & imap__slot_scalar__) && ((sval) >> imap__slot_shift__))

typedef struct {
    uint32_t stack[16];
    uint32_t stackp;
} imap_iter_t;

typedef struct {
    uint64_t x;
    uint32_t *slot;
} imap_pair_t;

#define imap__pair_zero__           ((imap_pair_t){0})
#define imap__pair__(x, slot)       ((imap_pair_t){(x), (slot)})
#define imap__node_zero__           ((imap_node_t){{{0}}})

#if defined(_MSC_VER)
static inline uint32_t imap__bsr__(uint64_t x) {
    return _BitScanReverse64((unsigned long *)&x, x | 1), (unsigned long)x;
}
#else
static inline uint32_t imap__bsr__(uint64_t x) {
    return 63 - __builtin_clzll(x | 1);
}
#endif

static inline uint32_t imap__xpos__(uint64_t x) {
    return imap__bsr__(x) >> 2;
}

static inline uint64_t imap__ceilpow2__(uint64_t x) {
    return 1ull << (imap__bsr__(x - 1) + 1);
}

static inline void *imap__aligned_alloc__(uint64_t alignment, uint64_t size) {
    void *p = EZ_MALLOC(size + sizeof(void *) + alignment - 1);
    if (!p)
        return p;
    void **ap = (void**)(((uint64_t)p + sizeof(void *) + alignment - 1) & ~(alignment - 1));
    ap[-1] = p;
    return ap;
}

static inline void imap__aligned_free__(void *p) {
    if (p)
        EZ_FREE(((void**)p)[-1]);
}

#define IMAP_ALIGNED_ALLOC(a, s)    (imap__aligned_alloc__(a, s))
#define IMAP_ALIGNED_FREE(p)        (imap__aligned_free__(p))

static inline imap_node_t* imap__node__(imap_node_t *tree, uint32_t val) {
    return (imap_node_t*)((uint8_t*)tree + val);
}

static inline uint32_t imap__node_pos__(imap_node_t *node) {
    return node->vec32[0] & 0xf;
}

static inline uint64_t imap__extract_lo4_port__(uint32_t vec32[16]) {
    union {
        uint32_t *vec32;
        uint64_t *vec64;
    } u;
    u.vec32 = vec32;
    return
        ((u.vec64[0] & 0xf0000000full)) |
        ((u.vec64[1] & 0xf0000000full) << 4) |
        ((u.vec64[2] & 0xf0000000full) << 8) |
        ((u.vec64[3] & 0xf0000000full) << 12) |
        ((u.vec64[4] & 0xf0000000full) << 16) |
        ((u.vec64[5] & 0xf0000000full) << 20) |
        ((u.vec64[6] & 0xf0000000full) << 24) |
        ((u.vec64[7] & 0xf0000000full) << 28);
}

static inline void imap__deposit_lo4_port__(uint32_t vec32[16], uint64_t value) {
    union {
        uint32_t *vec32;
        uint64_t *vec64;
    } u;
    u.vec32 = vec32;
    u.vec64[0] = (u.vec64[0] & ~0xf0000000full) | ((value) & 0xf0000000full);
    u.vec64[1] = (u.vec64[1] & ~0xf0000000full) | ((value >> 4) & 0xf0000000full);
    u.vec64[2] = (u.vec64[2] & ~0xf0000000full) | ((value >> 8) & 0xf0000000full);
    u.vec64[3] = (u.vec64[3] & ~0xf0000000full) | ((value >> 12) & 0xf0000000full);
    u.vec64[4] = (u.vec64[4] & ~0xf0000000full) | ((value >> 16) & 0xf0000000full);
    u.vec64[5] = (u.vec64[5] & ~0xf0000000full) | ((value >> 20) & 0xf0000000full);
    u.vec64[6] = (u.vec64[6] & ~0xf0000000full) | ((value >> 24) & 0xf0000000full);
    u.vec64[7] = (u.vec64[7] & ~0xf0000000full) | ((value >> 28) & 0xf0000000full);
}

static inline void imap__node_setprefix__(imap_node_t *node, uint64_t prefix) {
    imap__deposit_lo4_port__(node->vec32, prefix);
}

static inline uint64_t imap__node_prefix__(imap_node_t *node) {
    return imap__extract_lo4_port__(node->vec32);
}

static inline uint32_t imap__xdir__(uint64_t x, uint32_t pos) {
    return (x >> (pos << 2)) & 0xf;
}

static inline uint32_t imap__popcnt_hi28_port__(uint32_t vec32[16], uint32_t *p) {
    uint32_t pcnt = 0, sval, dirn;
    *p = 0;
    for (dirn = 0; 16 > dirn; dirn++)
    {
        sval = vec32[dirn];
        if (sval & ~0xf)
        {
            *p = sval;
            pcnt++;
        }
    }
    return pcnt;
}

static inline uint32_t imap__node_popcnt__(imap_node_t *node, uint32_t *p) {
    return imap__popcnt_hi28_port__(node->vec32, p);
}

static inline uint32_t imap__alloc_node__(imap_node_t *tree) {
    uint32_t mark = tree->vec32[imap__tree_nfre__];
    if (mark)
        tree->vec32[imap__tree_nfre__] = *(uint32_t*)((uint8_t *)tree + mark);
    else {
        mark = tree->vec32[imap__tree_mark__];
        assert(mark + sizeof(imap_node_t) <= tree->vec32[imap__tree_size__]);
        tree->vec32[imap__tree_mark__] = mark + sizeof(imap_node_t);
    }
    return mark;
}

static inline void imap__free_node__(imap_node_t *tree, uint32_t mark) {
    *(uint32_t *)((uint8_t *)tree + mark) = tree->vec32[imap__tree_nfre__];
    tree->vec32[imap__tree_nfre__] = mark;
}

static inline uint64_t imap__xpfx__(uint64_t x, uint32_t pos) {
    return x & (~0xfull << (pos << 2));
}

static imap_node_t* imap_ensure(imap_node_t *tree, size_t capacity) {
    if (!capacity)
        return NULL;
    imap_node_t *newtree;
    uint32_t hasnfre, hasvfre, newmark, oldsize, newsize;
    uint64_t newsize64;
    if (!tree) {
        hasnfre = 0;
        hasvfre = 1;
        newmark = sizeof(imap_node_t);
        oldsize = 0;
    } else {
        hasnfre = !!tree->vec32[imap__tree_nfre__];
        hasvfre = !!tree->vec32[imap__tree_vfre__];
        newmark = tree->vec32[imap__tree_mark__];
        oldsize = tree->vec32[imap__tree_size__];
    }
    newmark += (capacity * 2 - hasnfre) * sizeof(imap_node_t) + (capacity - hasvfre) * sizeof(uint64_t);
    if (newmark <= oldsize)
        return tree;
    newsize64 = imap__ceilpow2__(newmark);
    if (0x20000000 < newsize64)
        return NULL;
    newsize = (uint32_t)newsize64;
    newtree = (imap_node_t*)IMAP_ALIGNED_ALLOC(sizeof(imap_node_t), newsize);
    if (!newtree)
        return newtree;
    if (tree) {
        memcpy(newtree, tree, tree->vec32[imap__tree_mark__]);
        IMAP_ALIGNED_FREE(tree);
        newtree->vec32[imap__tree_size__] = newsize;
    } else {
        newtree->vec32[imap__tree_root__] = 0;
        newtree->vec32[imap__tree_resv__] = 0;
        newtree->vec32[imap__tree_mark__] = sizeof(imap_node_t);
        newtree->vec32[imap__tree_size__] = newsize;
        newtree->vec32[imap__tree_nfre__] = 0;
        newtree->vec32[imap__tree_vfre__] = 3 << imap__slot_shift__;
        newtree->vec64[3] = 4 << imap__slot_shift__;
        newtree->vec64[4] = 5 << imap__slot_shift__;
        newtree->vec64[5] = 6 << imap__slot_shift__;
        newtree->vec64[6] = 7 << imap__slot_shift__;
        newtree->vec64[7] = 0;
    }
    return newtree;
}

ezKeyMap* ezKeyMapNew(ezKeyMap *old, size_t capacity) {
    ezKeyMap *result = EZ_MALLOC(sizeof(ezKeyMap));
    if (!capacity)
        capacity = EZMAP_DEFAULT_CAPACITY;
    result->capacity = capacity;
    result->count = 0;
    result->tree = imap_ensure(NULL, capacity);
    return result;
}

static uint32_t* find(ezKeyMap *map, uint64_t x, int ensure) {
    uint32_t *slotstack[16 + 1];
    uint32_t posnstack[16 + 1];
    uint32_t stackp, stacki;
    imap_node_t *newnode, *node = map->tree;
    uint32_t *slot;
    uint32_t newmark, sval, diff, posn = 16, dirn = 0;
    uint64_t prfx;
    stackp = 0;
    for (;;) {
        slot = &node->vec32[dirn];
        sval = *slot;
        slotstack[stackp] = slot;
        posnstack[stackp++] = posn;
        if (!(sval & imap__slot_node__)) {
            prfx = imap__node_prefix__(node);
            if (!ensure || (!posn && prfx == (x & ~0xfull)))
                return slot;
            if (++map->count > map->capacity) {
                map->capacity *= 2;
                map->tree = imap_ensure(map->tree, map->capacity);
            }
            diff = imap__xpos__(prfx ^ x);
            assert(diff < 16);
            for (stacki = stackp; diff > posn;)
                posn = posnstack[--stacki];
            if (stacki != stackp) {
                slot = slotstack[stacki];
                sval = *slot;
                assert(sval & imap__slot_node__);
                newmark = imap__alloc_node__(map->tree);
                *slot = (*slot & imap__slot_pmask__) | imap__slot_node__ | newmark;
                newnode = imap__node__(map->tree, newmark);
                *newnode = imap__node_zero__;
                newmark = imap__alloc_node__(map->tree);
                newnode->vec32[imap__xdir__(prfx, diff)] = sval;
                newnode->vec32[imap__xdir__(x, diff)] = imap__slot_node__ | newmark;
                imap__node_setprefix__(newnode, imap__xpfx__(prfx, diff) | diff);
            } else {
                newmark = imap__alloc_node__(map->tree);
                *slot = (*slot & imap__slot_pmask__) | imap__slot_node__ | newmark;
            }
            newnode = imap__node__(map->tree, newmark);
            *newnode = imap__node_zero__;
            imap__node_setprefix__(newnode, x & ~0xfull);
            return &newnode->vec32[x & 0xfull];
        }
        node = imap__node__(map->tree, sval & imap__slot_value__);
        posn = imap__node_pos__(node);
        dirn = imap__xdir__(x, posn);
    }
}

static inline uint32_t imap__alloc_val__(imap_node_t *tree) {
    uint32_t mark = imap__alloc_node__(tree);
    imap_node_t *node = imap__node__(tree, mark);
    mark <<= 3;
    tree->vec32[imap__tree_vfre__] = mark;
    node->vec64[0] = mark + (1 << imap__slot_shift__);
    node->vec64[1] = mark + (2 << imap__slot_shift__);
    node->vec64[2] = mark + (3 << imap__slot_shift__);
    node->vec64[3] = mark + (4 << imap__slot_shift__);
    node->vec64[4] = mark + (5 << imap__slot_shift__);
    node->vec64[5] = mark + (6 << imap__slot_shift__);
    node->vec64[6] = mark + (7 << imap__slot_shift__);
    node->vec64[7] = 0;
    return mark;
}

static void imap_setval64(imap_node_t *tree, uint32_t *slot, uint64_t y) {
    assert(!(*slot & imap__slot_node__));
    uint32_t sval = *slot;
    if (!(sval >> imap__slot_shift__)) {
        sval = tree->vec32[imap__tree_vfre__];
        if (!sval)
            sval = imap__alloc_val__(tree);
        assert(sval >> imap__slot_shift__);
        tree->vec32[imap__tree_vfre__] = (uint32_t)tree->vec64[sval >> imap__slot_shift__];
    }
    assert(!(sval & imap__slot_node__));
    assert(imap__slot_boxed__(sval));
    *slot = (*slot & imap__slot_pmask__) | sval;
    tree->vec64[sval >> imap__slot_shift__] = y;
}

int ezKeyMapSet(ezKeyMap *map, uint64_t key, void *item) {
    uint32_t *slot = find(map, key, 1);
    if (!slot)
        return 0;
    imap_setval64(map->tree, slot, (uint64_t)item);
    return 1;
}

static uint64_t imap_getval(imap_node_t *tree, uint32_t *slot) {
    assert(!(*slot & imap__slot_node__));
    uint32_t sval = *slot;
    if (!imap__slot_boxed__(sval))
        return sval >> imap__slot_shift__;
    else
        return tree->vec64[sval >> imap__slot_shift__];
}

void* ezKeyMapGet(ezKeyMap *map, uint64_t key) {
    uint32_t *slot = find(map, key, 0);
    return slot ? (void*)imap_getval(map->tree, slot) : NULL;
}

static void imap_delval(imap_node_t *tree, uint32_t *slot) {
    assert(!(*slot & imap__slot_node__));
    uint32_t sval = *slot;
    if (imap__slot_boxed__(sval)) {
        tree->vec64[sval >> imap__slot_shift__] = tree->vec32[imap__tree_vfre__];
        tree->vec32[imap__tree_vfre__] = sval & imap__slot_value__;
    }
    *slot &= imap__slot_pmask__;
}

static void imap_remove(imap_node_t *tree, uint64_t x) {
    uint32_t *slotstack[16 + 1];
    uint32_t stackp;
    imap_node_t *node = tree;
    uint32_t *slot;
    uint32_t sval, pval, posn = 16, dirn = 0;
    stackp = 0;
    for (;;) {
        slot = &node->vec32[dirn];
        sval = *slot;
        if (!(sval & imap__slot_node__)) {
            if ((sval & imap__slot_value__) && imap__node_prefix__(node) == (x & ~0xfull)) {
                assert(0 == posn);
                imap_delval(tree, slot);
            }
            while (stackp) {
                slot = slotstack[--stackp];
                sval = *slot;
                node = imap__node__(tree, sval & imap__slot_value__);
                posn = imap__node_pos__(node);
                if (!!posn != imap__node_popcnt__(node, &pval))
                    break;
                imap__free_node__(tree, sval & imap__slot_value__);
                *slot = (sval & imap__slot_pmask__) | (pval & ~imap__slot_pmask__);
            }
            return;
        }
        node = imap__node__(tree, sval & imap__slot_value__);
        posn = imap__node_pos__(node);
        dirn = imap__xdir__(x, posn);
        slotstack[stackp++] = slot;
    }
}

void* ezKeyMapDel(ezKeyMap *map, uint64_t key) {
    if (!map->count)
        return NULL;
    uint32_t *slot = find(map, key, 0);
    if (!slot)
        return NULL;
    void* val = (void*)imap_getval(map->tree, slot);
    imap_remove(map->tree, key);
    map->count--;
    return val;
}

static imap_pair_t imap_iterate(imap_node_t *tree, imap_iter_t *iter, int restart) {
    imap_node_t *node;
    uint32_t *slot;
    uint32_t sval, dirn;
    if (restart) {
        iter->stackp = 0;
        sval = dirn = 0;
        goto enter;
    }
    // loop while stack is not empty
    while (iter->stackp) {
        // get slot value and increment direction
        sval = iter->stack[iter->stackp - 1]++;
        dirn = sval & 31;
        if (15 < dirn) {
            // if directions 0-15 have been examined, pop node from stack
            iter->stackp--;
            continue;
        }
    enter:
        node = imap__node__(tree, sval & imap__slot_value__);
        slot = &node->vec32[dirn];
        sval = *slot;
        if (sval & imap__slot_node__)
            // push node into stack
            iter->stack[iter->stackp++] = sval & imap__slot_value__;
        else if (sval & imap__slot_value__)
            return imap__pair__(imap__node_prefix__(node) | dirn, slot);
    }
    return imap__pair_zero__;
}

int ezKeyMapEach(ezKeyMap *map, int(*callback)(ezKeyValuePair *pair, uint64_t, void*), void *userdata) {
    imap_iter_t iter;
    imap_pair_t pair = imap_iterate(map->tree, &iter, 1);
    size_t i = 0;
    ezKeyValuePair ezpair;
    for (;;) {
        if (!pair.slot)
            break;
        ezpair.key = pair.x;
        ezpair.val = (void*)imap_getval(map->tree, pair.slot);
        int result = callback(&ezpair, i++, userdata);
        if (!result)
            return result;
        else
            pair = imap_iterate(map->tree, &iter, 0);
    }
    return 0;
}

void ezKeyMapDestroy(ezKeyMap *map) {
    IMAP_ALIGNED_FREE(map->tree);
    EZ_FREE(map);
}

static void MM86128(const void *key, const int len, uint32_t seed, void *out) {
#define ROTL32(x, r) ((x << r) | (x >> (32 - r)))
#define FMIX32(h) h^=h>>16; h*=0x85ebca6b; h^=h>>13; h*=0xc2b2ae35; h^=h>>16;
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;
    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;
    uint32_t c1 = 0x239b961b;
    uint32_t c2 = 0xab0e9789;
    uint32_t c3 = 0x38b34ae5;
    uint32_t c4 = 0xa1e38b93;
    const uint32_t * blocks = (const uint32_t *)(data + nblocks*16);
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i*4+0];
        uint32_t k2 = blocks[i*4+1];
        uint32_t k3 = blocks[i*4+2];
        uint32_t k4 = blocks[i*4+3];
        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
        h1 = ROTL32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;
        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        h2 = ROTL32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;
        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        h3 = ROTL32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;
        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        h4 = ROTL32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
    }
    const uint8_t * tail = (const uint8_t*)(data + nblocks*16);
    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;
    switch(len & 15) {
        case 15:
            k4 ^= tail[14] << 16;
        case 14:
            k4 ^= tail[13] << 8;
        case 13:
            k4 ^= tail[12] << 0;
            k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        case 12:
            k3 ^= tail[11] << 24;
        case 11:
            k3 ^= tail[10] << 16;
        case 10:
            k3 ^= tail[ 9] << 8;
        case 9:
            k3 ^= tail[ 8] << 0;
            k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        case 8:
            k2 ^= tail[ 7] << 24;
        case 7:
            k2 ^= tail[ 6] << 16;
        case 6:
            k2 ^= tail[ 5] << 8;
        case 5:
            k2 ^= tail[ 4] << 0;
            k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        case 4:
            k1 ^= tail[ 3] << 24;
        case 3:
            k1 ^= tail[ 2] << 16;
        case 2:
            k1 ^= tail[ 1] << 8;
        case 1:
            k1 ^= tail[ 0] << 0;
            k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };
    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
    FMIX32(h1); FMIX32(h2); FMIX32(h3); FMIX32(h4);
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
    ((uint32_t*)out)[0] = h1;
    ((uint32_t*)out)[1] = h2;
    ((uint32_t*)out)[2] = h3;
    ((uint32_t*)out)[3] = h4;
}

uint64_t MurmurHash(const void *data, size_t len, uint32_t seed) {
    char out[16];
    MM86128(data, (int)len, (uint32_t)seed, &out);
    return *(uint64_t*)out;
}

#define HASH_WRAPPER(FN, ...)                              \
do {                                                       \
    int64_t hash = MurmurHash((void*)key, strlen(key), 0); \
    return ezKeyMap##FN((ezKeyMap*)dict, __VA_ARGS__);     \
} while(0)

int ezDictSet(ezDict *dict, const char *key, void *value) {
    HASH_WRAPPER(Set, hash, value);
}

void* ezDictGet(ezDict *dict, const char *key) {
    HASH_WRAPPER(Get, hash);
}

void* ezDictDel(ezDict *dict, const char *key) {
    HASH_WRAPPER(Del, hash);
}
#endif
