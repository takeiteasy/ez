/* ezmap.h - Simple key value map + dictionary (WIP)
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
 
 This library is based off https://github.com/tidwall/hashmap.c
 Copyright 2020 Joshua J Baker. All rights reserved. */

#ifndef EZMAP_HEADER
#define EZMAP_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <stddef.h>

typedef struct ezKVM {
    size_t capacity;
    size_t sizeOfBuckets;
    size_t bucketsCount;
    size_t count;
    size_t mask;
    size_t growAt;
    size_t shrinkAt;
    uint8_t loadFactor;
    void *buckets;
    void *spare;
    void *extra;
} ezKVM;

ezKVM* ezKVMNew(size_t capacity);
int ezKVMSet(ezKVM *kvm, uint64_t key, void *value);
void* ezKVMGet(ezKVM *kvm, uint64_t key);
void* ezKVMDel(ezKVM *kvm, uint64_t key);
void ezKVMScan(ezKVM *kvm, int(*callback)(uint64_t, void*, void*), void *userdata);
int ezKVMIter(ezKVM *kvm, size_t *i, uint64_t *key, void **item);
void ezKVMClear(ezKVM *kvm, int adjustCapacity);
void ezKVMFree(ezKVM *kvm);

typedef ezKVM ezDictionary;

#define ezDictNew(...) ezKVMNew(__VA_ARGS__)
int ezDictSet(ezDictionary *dict, const char *key, void *value);
void* ezDictGet(ezDictionary *dict, const char *key);
void* ezDictDel(ezDictionary *dict, const char *key);
#define ezDictScan(...) ezKVMScan(__VA_ARGS__)
#define ezDictIter(...) ezKVMIter(__VA_ARGS__)
#define ezDictClear(...) ezKVMClear(__VA_ARGS__)
#define ezDictFree(...) ezKVMFree(__VA_ARGS__)

uint64_t MurmurHash(const void *data, size_t len, uint32_t seed);

#ifdef __has_extension
#if !__has_extension(c_generic_selections)
#define EZ_MAP_DISABLE
#endif
#else
#if !defined(__STDC__) || !__STDC__ || __STDC_VERSION__ < 201112L
#define EZ_MAP_DISABLE
#endif
#endif

#if !defined(EZ_MAP_DISABLE)
typedef ezKVM ezMap;

#define ezMapNew(...) ezKVMNew(__VA_ARGS__)
#define ezMapSet(KVM, KEY, VAL) _Generic((KEY),                 \
                                         uint64_t: ezKVMSet,    \
                                         const char*: ezDictSet \
                                        )(KVM, KEY, VAL)
#define ezMapGet(KVM, KEY) _Generic((KEY),                 \
                                    uint64_t: ezKVMGet,    \
                                    const char*: ezDictGet \
                                   )(KVM, KEY)
#define ezMapDel(KVM, KEY) _Generic((KEY),                 \
                                    uint64_t: ezKVMDel,    \
                                    const char*: ezDictDel \
                                   )(KVM, KEY)
#define ezMapScan(...) ezKVMScan(__VA_ARGS__)
#define ezMapIter(...) ezKVMIter(__VA_ARGS__)
#define ezMapClear(...) ezKVMClear(__VA_ARGS__)
#define ezMapFree(...) ezKVMFree(__VA_ARGS__)
#endif

#if defined(__cplusplus)
}
#endif
#endif // EZMAP_HEADER

#if defined(EZMAP_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
typedef struct Bucket {
    uint64_t hash:48;
    uint64_t dib:16;
} Bucket;

#define BUCKET_AT(M, I) ((Bucket*)(((char*)((M)->buckets))+((M)->sizeOfBuckets*(I))))
#define BUCKET_ITEM(B) ((char*)(B) + sizeof(Bucket*))

#define GROW_AT   0.60 /* 60% */
#define SHRINK_AT 0.10 /* 10% */

ezKVM* ezKVMNew(size_t capacity) {
    size_t sizeOfBuckets = sizeof(Bucket) + sizeof(void*);
    while (sizeOfBuckets & (sizeof(uintptr_t)-1))
        sizeOfBuckets++;
    size_t sizeOfMap = sizeof(ezKVM) + sizeOfBuckets * 2;
    ezKVM *result = malloc(sizeOfMap);
    result->capacity = Max((int)capacity, 16);
    result->bucketsCount = result->capacity;
    result->mask = result->bucketsCount - 1;
    result->count = 0;
    result->loadFactor = GROW_AT * 100;
    result->sizeOfBuckets = sizeOfBuckets;
    result->buckets = malloc(result->sizeOfBuckets * result->bucketsCount);
    memset(result->buckets, 0, result->sizeOfBuckets * result->bucketsCount);
    result->growAt = result->bucketsCount * .75f;
    result->shrinkAt = result->bucketsCount * .1f;
    result->spare = (char*)result + sizeof(ezKVM);
    result->extra = (char*)result->spare + result->sizeOfBuckets;
    return result;
}

static void ResizeKVM(ezKVM *kvm, size_t newCapacity) {
    ezKVM *new = ezKVMNew(newCapacity);
    for (int i = 0; i < kvm->bucketsCount; i++) {
        Bucket *entry = BUCKET_AT(kvm, i);
        if (!entry->dib)
            continue;
        
        entry->dib = 1;
        size_t j = entry->hash & new->mask;
        for (;;) {
            Bucket *bucket = BUCKET_AT(new, j);
            if (!bucket->dib) {
                memcpy(bucket, entry, kvm->sizeOfBuckets);
                break;
            }
            if (bucket->dib < entry->dib) {
                memcpy(new->spare, bucket, kvm->sizeOfBuckets);
                memcpy(bucket, entry, kvm->sizeOfBuckets);
                memcpy(entry, new->spare, kvm->sizeOfBuckets);
            }
            j = (j + 1) & new->mask;
            entry->dib += 1;
        }
    }
    
    free(kvm->buckets);
    kvm->buckets = new->buckets;
    kvm->bucketsCount = new->bucketsCount;
    kvm->mask = new->mask;
    kvm->growAt = new->growAt;
    kvm->shrinkAt = new->shrinkAt;
    free(new);
}

int ezKVMSet(ezKVM *kvm, uint64_t key, void *value) {
    if (kvm->count == kvm->growAt)
        ResizeKVM(kvm, kvm->bucketsCount * 2);
    
    Bucket *entry = kvm->extra;
    entry->hash = key;
    entry->dib = 1;
    memcpy(BUCKET_ITEM(entry), value, sizeof(void*));
    
    size_t i = entry->hash & kvm->mask;
    for (;;) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (bucket->dib == 0) {
            memcpy(bucket, entry, kvm->sizeOfBuckets);
            kvm->count++;
            return 2;
        }
        if (entry->hash == bucket->hash) {
            memcpy(kvm->spare, BUCKET_ITEM(bucket), sizeof(void*));
            memcpy(BUCKET_ITEM(bucket), BUCKET_ITEM(entry), sizeof(void*));
            return 1;
        }
        if (bucket->dib < entry->dib) {
            memcpy(kvm->spare, bucket, kvm->sizeOfBuckets);
            memcpy(bucket, entry, kvm->sizeOfBuckets);
            memcpy(entry, kvm->spare, kvm->sizeOfBuckets);
        }
        i = (i + 1) & kvm->mask;
        entry->dib += 1;
    }
    
    return 0;
}

void* ezKVMGet(ezKVM *kvm, uint64_t key) {
    size_t i = key & kvm->mask;
    for (;;) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (!bucket->dib)
            return NULL;
        if (bucket->hash == key)
            return BUCKET_ITEM(bucket);
        i = (i + 1) & kvm->mask;
    }
}

void* ezKVMDel(ezKVM *kvm, uint64_t key) {
    size_t i = key & kvm->mask;
    for (;;) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (!bucket->dib)
            return NULL;
        if (bucket->hash == key) {
            memcpy(kvm->spare, BUCKET_ITEM(bucket), sizeof(void*));
            bucket->dib = 0;
            for (;;) {
                Bucket *prev = bucket;
                i = (i + 1) & kvm->mask;
                bucket = BUCKET_AT(kvm, i);
                if (bucket->dib <= 1) {
                    prev->dib = 0;
                    break;
                }
                memcpy(prev, bucket, kvm->sizeOfBuckets);
                prev->dib--;
            }
            
            // Ignore the return value. It's ok for the resize operation to
            // fail to allocate enough memory because a shrink operation
            // does not change the integrity of the data.
            if (kvm->bucketsCount > kvm->capacity && --kvm->count <= kvm->shrinkAt)
                ResizeKVM(kvm, kvm->bucketsCount / 2);
            return kvm->spare;
        }
        i = (i + 1) & kvm->mask;
    }
}

void ezKVMScan(ezKVM *kvm, int(*callback)(uint64_t, void*, void*), void *userdata) {
    for (size_t i = 0; i < kvm->bucketsCount; i++) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (bucket->dib && !callback((int)bucket->hash, BUCKET_ITEM(bucket), userdata))
            return;
    }
}

int ezKVMIter(ezKVM *kvm, size_t *i, uint64_t *key, void **item) {
    Bucket *bucket = NULL;
    do {
        if (*i >= kvm->bucketsCount)
            return 0;
        bucket = BUCKET_AT(kvm, *i);
        (*i)++;
    } while (!bucket->dib);
    if (key)
        *key = bucket->hash;
    *item = BUCKET_ITEM(bucket);
    return 1;
}

void ezKVMClear(ezKVM *kvm, int adjustCapacity) {
    kvm->count = 0;
    if (adjustCapacity)
        kvm->capacity = kvm->bucketsCount;
    else if (kvm->bucketsCount != kvm->capacity) {
        void *new_buckets = malloc(kvm->sizeOfBuckets * kvm->capacity);
        if (new_buckets) {
            free(kvm->buckets);
            kvm->buckets = new_buckets;
        }
        kvm->bucketsCount = kvm->capacity;
    }
    memset(kvm->buckets, 0, kvm->sizeOfBuckets * kvm->bucketsCount);
    kvm->mask = kvm->bucketsCount-1;
    kvm->growAt = kvm->bucketsCount * (kvm->loadFactor / 100.f);
    kvm->shrinkAt = kvm->bucketsCount * SHRINK_AT;
}

void ezKVMFree(ezKVM *kvm) {
    
}

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//-----------------------------------------------------------------------------
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

// TODO: Make seed cofigurable
#define HASH_WRAPPER(FN, ...)                              \
do {                                                       \
    int64_t hash = MurmurHash((void*)key, strlen(key), 0); \
    return ezKVM##FN((ezKVM*)dict, __VA_ARGS__);           \
} while(0)

int ezDictSet(ezDictionary *dict, const char *key, void *value) {
    HASH_WRAPPER(Set, hash, value);
}

void* ezDictGet(ezDictionary *dict, const char *key) {
    HASH_WRAPPER(Get, hash);
}

void* ezDictDel(ezDictionary *dict, const char *key) {
    HASH_WRAPPER(Del, hash);
}
#endif
