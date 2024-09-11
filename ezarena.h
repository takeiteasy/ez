/* ezarena.h -- https://github.com/takeiteasy/ez

 ezarena -- Cross-platform arena memory storage pool
 
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

#ifndef EZARENA_HEADER
#define EZARENA_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
#define ARENA_PLATFORM_WINDOWS
#endif

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#if defined(ARENA_PLATFORM_WINDOWS)
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

typedef struct ezArenaRegion {
    void* memory;
    size_t sizeOfMemory;
    struct ezArenaRegion *next, *prev;
} ezArenaRegion;

typedef struct ezArena {
    ezArenaRegion *head, *tail;
} ezArena;

void ezArenaReset(ezArena *arena, void(*callback)(void*));
void* ezArenaAlloc(ezArena *arena, size_t sizeOfMemory);
void ezArenaFree(ezArena *arena, void *memory);

#if defined(__cplusplus)
}
#endif
#endif // EZARENA_HEADER

#if defined(EZARENA_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
typedef ezArenaRegion Region;

static void* MemAlloc(size_t size) {
#if defined(ARENA_PLATFORM_WINDOWS)
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
#else
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
}

static int MemFree(void *location, size_t sizeOfMemory) {
#if defined(ARENA_PLATFORM_WINDOWS)
    return VirtualFree(location, sizeOfMemory, MEM_RELEASE);
#else
    return munmap(location, sizeOfMemory) != -1;
#endif
}

void ezArenaReset(ezArena *arena, void(*callback)(void*)) {
    ezArenaRegion *current = arena->head;
    while(current) {
        ezArenaRegion *next = current->next;
        if (callback)
            callback(current->memory);
        assert(MemFree(current->memory, current->sizeOfMemory));
        assert(MemFree(current, sizeof(ezArenaRegion)));
        current = next;
    }
    arena->head = NULL;
    arena->tail = NULL;
}

void* ezArenaAlloc(ezArena *arena, size_t sizeOfMemory) {
    Region *region = (Region*)MemAlloc(sizeof(Region));
    region->memory = MemAlloc(sizeOfMemory);
    region->sizeOfMemory = sizeOfMemory;

    if (!arena->tail) {
        region->prev = NULL;
        region->next = NULL;
        arena->head = arena->tail = region;
    } else {
        region->prev = arena->tail;
        arena->tail->next = region;
        arena->tail = region;
    }

    return region->memory;
}

void ezArenaFree(ezArena *arena, void *memory) {
    Region *cursor = arena->head;
    while (cursor) {
        if (cursor->memory == memory) {
            if (cursor->prev && cursor->prev->next)
                cursor->prev->next = cursor->next;
            if (cursor->next && cursor->next->prev)
                cursor->next->prev = cursor->prev;
            assert(MemFree(cursor->memory, cursor->sizeOfMemory));
            assert(MemFree(cursor, sizeof(Region)));
            return;
        }
        cursor = cursor->next;
    }
    assert(0); // Not found
}
#endif
