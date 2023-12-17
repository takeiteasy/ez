/* ezkvm.h - Simple key value map (WIP)
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

#ifndef EZKVM_HEADER
#define EZKVM_HEADER
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
    void *buckets;
    void *spare;
    void *extra;
} ezKVM;

ezKVM* ezKVMNew(size_t capacity);
int ezKVMSet(ezKVM *kvm, int key, void *value);
void* ezKVMGet(ezKVM *kvm, int key);
void* ezKVMDel(ezKVM *kvm, int key);
void ezKVMIterate(ezKVM *kvm, int(*callback)(int, void*, void*), void *userdata);

#if defined(__cplusplus)
}
#endif
#endif // EZKVM_HEADER

#if defined(EZKVM_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
typedef struct Bucket {
    uint64_t hash:48;
    uint64_t dib:16;
} Bucket;

#define BUCKET_AT(M, I) ((Bucket*)(((char*)((M)->buckets))+((M)->sizeOfBuckets*(I))))
#define BUCKET_ITEM(B) ((char*)(B) + sizeof(Bucket*))

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

int ezKVMSet(ezKVM *kvm, int key, void *value) {
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

void* ezKVMGet(ezKVM *kvm, int key) {
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

void* ezKVMDel(ezKVM *kvm, int key) {
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

void ezKVMIterate(ezKVM *kvm, int(*callback)(int, void*, void*), void *userdata) {
    for (size_t i = 0; i < kvm->bucketsCount; i++) {
        Bucket *bucket = BUCKET_AT(kvm, i);
        if (bucket->dib && !callback((int)bucket->hash, BUCKET_ITEM(bucket), userdata))
            return;
    }
}
#endif
