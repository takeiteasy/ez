/* ezstack.h - Simple double linked-list implementation (WIP)
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

#ifndef EZSTACK_HEADER
#define EZSTACK_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

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
    ezStackEntry *entry = malloc(sizeof(ezStackEntry));
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
