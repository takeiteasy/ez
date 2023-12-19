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
#include <string.h>
#include <stddef.h>

#if !defined(EZ_MALLOC)
#define EZ_MALLOC malloc
#endif
#if !defined(EZ_FREE)
#define EZ_FREE free
#endif

typedef struct ezKVM {
    size_t sizeOfElements;
    size_t capacity;
    size_t sizeOfBuckets;
    size_t bucketsCount;
    size_t count;
    size_t mask;
    size_t growAt;
    size_t shrinkAt;
    uint8_t loadFactor;
    uint8_t growPower;
    void *buckets;
    void *spare;
    void *edata;
} ezKVM;

ezKVM* ezKVMNew(size_t capacity, size_t sizeOfElements);
int ezKVMSet(ezKVM *kvm, uint64_t key, void *value);
void* ezKVMGet(ezKVM *kvm, uint64_t key);
void* ezKVMDel(ezKVM *kvm, uint64_t key);
void* ezKVMProbe(ezKVM *kvm, uint64_t position);
int ezKVMScan(ezKVM *kvm, int(*callback)(uint64_t, void*, void*), void *userdata);
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
                                         int: ezKVMSet,         \
                                         uint64_t: ezKVMSet,    \
                                         char*: ezDictSet,      \
                                         const char*: ezDictSet \
                                        )(KVM, KEY, VAL)
#define ezMapGet(KVM, KEY) _Generic((KEY),                 \
                                    int: ezKVMGet,         \
                                    uint64_t: ezKVMGet,    \
                                    char*: ezDictGet,      \
                                    const char*: ezDictGet \
                                   )(KVM, KEY)
#define ezMapDel(KVM, KEY) _Generic((KEY),                 \
                                    int: ezKVMDel,         \
                                    uint64_t: ezKVMDel,    \
                                    char*: ezDictDel,      \
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

#define GROW_AT   0.60 /* 60% */
#define SHRINK_AT 0.10 /* 10% */

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define CLAMP(N, A, B) (MIN(MAX((N), (A)), (B)))

#define BUCKET_AT(B, I) ((Bucket*)(((char*)(B)->buckets)+((B)->sizeOfBuckets*(I))))
#define BUCKET_ITEM(E) (((char*)(E))+sizeof(Bucket))
#define CLIP(H) ((H) & 0xFFFFFFFFFFFF)

ezKVM* ezKVMNew(size_t capacity, size_t sizeOfElements) {
    size_t ncap = 16;
    if (capacity < ncap)
        capacity = ncap;
    else {
        while (ncap < capacity)
            ncap *= 2;
        capacity = ncap;
    }
    
    size_t sizeOfBuckets = sizeof(Bucket) + sizeOfElements;
    while (sizeOfBuckets & (sizeof(uintptr_t)-1))
        sizeOfBuckets++;
    
    size_t size = sizeof(ezKVM) + sizeOfBuckets * 2;
    ezKVM *result = EZ_MALLOC(size);
    memset(result, 0, size);
    result->sizeOfElements = sizeOfElements;
    result->sizeOfBuckets = sizeOfBuckets;
    result->spare = (char*)result + sizeof(ezKVM);
    result->edata = (char*)result->spare + sizeOfBuckets;
    result->capacity = capacity;
    result->bucketsCount = capacity;
    result->mask = capacity - 1;
    result->buckets = EZ_MALLOC(result->sizeOfBuckets * result->bucketsCount);
    memset(result->buckets, 0, result->sizeOfBuckets * result->bucketsCount);
    result->growPower = 1;
    result->loadFactor = GROW_AT * 100;
    result->shrinkAt = capacity * SHRINK_AT;
    return result;
}

static void ResizeKVM(ezKVM *kvm, size_t newCapacity) {
    ezKVM *tmp = ezKVMNew(newCapacity, kvm->sizeOfElements);
    
    for (size_t i = 0; i < kvm->bucketsCount; i++) {
        Bucket *entry = BUCKET_AT(kvm, i);
        if (!entry->dib)
            continue;
        
        entry->dib = 1;
        size_t j = entry->hash & tmp->mask;
        while(1) {
            Bucket *bucket = BUCKET_AT(tmp, j);
            if (bucket->dib == 0) {
                memcpy(bucket, entry, kvm->sizeOfBuckets);
                break;
            }
            if (bucket->dib < entry->dib) {
                memcpy(tmp->spare, bucket, kvm->sizeOfBuckets);
                memcpy(bucket, entry, kvm->sizeOfBuckets);
                memcpy(entry, tmp->spare, kvm->sizeOfBuckets);
            }
            j = (j + 1) & tmp->mask;
            entry->dib += 1;
        }
    }
    
    EZ_FREE(kvm->buckets);
    kvm->buckets = tmp->buckets;
    kvm->bucketsCount = tmp->bucketsCount;
    kvm->mask = tmp->mask;
    kvm->growAt = tmp->growAt;
    kvm->shrinkAt = tmp->shrinkAt;
    EZ_FREE(tmp);
}

int ezKVMSet(ezKVM *kvm, uint64_t key, void *value) {
    uint64_t hash = CLIP(key);
    if (kvm->count >= kvm->growAt)
        ResizeKVM(kvm, kvm->bucketsCount * (1 << kvm->growPower));
    
    Bucket *entry = kvm->edata;
    entry->hash = hash;
    entry->dib = 1;
    void *eitem = BUCKET_ITEM(entry);
    memcpy(eitem, value, kvm->sizeOfElements);
    
    void *bitem;
    size_t i = entry->hash & kvm->mask;
    while(1) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (bucket->dib == 0) {
            memcpy(bucket, entry, kvm->sizeOfBuckets);
            kvm->count++;
            return 1;
        }
        bitem = BUCKET_ITEM(bucket);
        if (entry->hash == bucket->hash)
        {
            memcpy(kvm->spare, bitem, kvm->sizeOfElements);
            memcpy(bitem, eitem, kvm->sizeOfElements);
            return 2;
        }
        if (bucket->dib < entry->dib) {
            memcpy(kvm->spare, bucket, kvm->sizeOfBuckets);
            memcpy(bucket, entry, kvm->sizeOfBuckets);
            memcpy(entry, kvm->spare, kvm->sizeOfBuckets);
            eitem = BUCKET_ITEM(entry);
        }
        i = (i + 1) & kvm->mask;
        entry->dib += 1;
    }
    return 0;
}

void* ezKVMGet(ezKVM *kvm, uint64_t key) {
    uint64_t hash = CLIP(key);
    size_t i = hash & kvm->mask;
    for (;;) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (!bucket->dib)
            return NULL;
        if (bucket->hash == hash)
            return BUCKET_ITEM(bucket);
        i = (i + 1) & kvm->mask;
    }
    return NULL;
}

void* ezKVMDel(ezKVM *kvm, uint64_t key) {
    uint64_t hash = CLIP(key);
    size_t i = hash & kvm->mask;
    while(1) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (!bucket->dib) {
            return NULL;
        }
        void *bitem = BUCKET_ITEM(bucket);
        if (bucket->hash == hash) {
            memcpy(kvm->spare, bitem, kvm->sizeOfElements);
            bucket->dib = 0;
            while(1) {
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
                ResizeKVM(kvm, kvm->bucketsCount/2);
            return kvm->spare;
        }
        i = (i + 1) & kvm->mask;
    }
}

void* ezKVMProbe(ezKVM *kvm, uint64_t position) {
    size_t i = position & kvm->mask;
    Bucket *bucket = BUCKET_AT(kvm, i);
    if (!bucket->dib) {
        return NULL;
    }
    return BUCKET_ITEM(bucket);
}

int ezKVMScan(ezKVM *kvm, int(*callback)(uint64_t, void*, void*), void *userdata) {
    for (size_t i = 0; i < kvm->bucketsCount; i++) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (bucket->dib && !callback(bucket->hash, BUCKET_ITEM(bucket), userdata))
            return 0;
    }
    return 1;
}

int ezKVMIter(ezKVM *kvm, size_t *i, uint64_t *key, void **item) {
    Bucket *bucket;
    do {
        if (*i >= kvm->bucketsCount)
            return 0;
        bucket = BUCKET_AT(kvm, *i);
        (*i)++;
    } while (!bucket->dib);
    *item = BUCKET_ITEM(bucket);
    return 1;
}

void ezKVMClear(ezKVM *kvm, int adjustCapacity) {
    kvm->count = 0;
    if (adjustCapacity)
        kvm->capacity = kvm->bucketsCount;
    else if (kvm->bucketsCount != kvm->capacity) {
        void *new_buckets = EZ_MALLOC(kvm->sizeOfBuckets*kvm->capacity);
        if (new_buckets) {
            EZ_FREE(kvm->buckets);
            kvm->buckets = new_buckets;
        }
        kvm->bucketsCount = kvm->capacity;
    }
    memset(kvm->buckets, 0, kvm->sizeOfBuckets*kvm->bucketsCount);
    kvm->mask = kvm->bucketsCount-1;
    kvm->growAt = kvm->bucketsCount * (kvm->loadFactor / 100.0) ;
    kvm->shrinkAt = kvm->bucketsCount * SHRINK_AT;
}

void ezKVMFree(ezKVM *kvm) {
    EZ_FREE(kvm->buckets);
    EZ_FREE(kvm);
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

