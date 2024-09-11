/* ezstack.h - -- https://github.com/takeiteasy/ez
 
 ezstack -- Simple double-linked list
 
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

#ifndef EZSTACK_HEADER
#define EZSTACK_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <assert.h>

#if !defined(EZ_MALLOC)
#define EZ_MALLOC malloc
#endif

typedef struct ezStackEntry {
    int id;
    void *data;
    struct ezStackEntry *next, *prev;
} ezStackEntry;

typedef struct ezStack {
    ezStackEntry *front, *back;
} ezStack;

void ezStackPush(ezStack *stack, int id, void *data);
void ezStackAppend(ezStack *stack, int id, void *data);
ezStackEntry* ezStackShift(ezStack *stack);
ezStackEntry* ezStackDrop(ezStack *stack);

#if defined(__cplusplus)
}
#endif
#endif // EZSTACK_HEADER

#if defined(EZSTACK_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
static ezStackEntry* NewStackEntry(int id, void *data, ezStackEntry *next, ezStackEntry *prev) {
    assert(data);
    ezStackEntry *entry = EZ_MALLOC(sizeof(ezStackEntry));
    entry->id   = id;
    entry->data = data;
    entry->next = next;
    entry->prev = prev;
    return entry;
}

void ezStackPush(ezStack *stack, int id, void *data) {
    ezStackEntry *entry = NewStackEntry(id, data, stack->front, NULL);
    if (stack->front)
        stack->front->prev = entry;
    stack->front = entry;
    if (!stack->back)
        stack->back = stack->front;
}

void ezStackAppend(ezStack *stack, int id, void *data) {
    ezStackEntry *entry = NewStackEntry(id, data, NULL, NULL);
    if (!stack->back)
        stack->front = stack->back = entry;
    else {
        entry->prev = stack->back;
        stack->back->next = entry;
        stack->back = entry;
    }
}

ezStackEntry* ezStackShift(ezStack *stack) {
    if (!stack->front)
        return NULL;
    ezStackEntry *tmp = stack->front;
    stack->front = stack->front->next;
    if (stack->front)
        stack->front->prev = NULL;
    else
        stack->back = NULL;
    return tmp;
}

ezStackEntry* ezStackDrop(ezStack *stack) {
    if (!stack->back)
        return NULL;
    ezStackEntry *tmp = stack->back;
    stack->back = stack->back->prev;
    if (stack->back)
        stack->back->next = NULL;
    else
        stack->front = NULL;
    return tmp;
}
#endif
