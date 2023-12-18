#define EZMAP_IMPLEMENTATION
#include "ezmap.h"
#include <stdio.h>
#include <assert.h>

typedef struct {
    const char *name;
    int id;
} ChildStruct;

typedef struct {
    char *name;
    int id;
    ChildStruct *child;
} ParentStruct;

int main(int argc, const char * argv[]) {
    ParentStruct *parentA = malloc(sizeof(ParentStruct));
    parentA->id = 1;
    parentA->name = "Mr Chang";
    
    ParentStruct *parentB = malloc(sizeof(ParentStruct));
    parentB->id = 2;
    parentB->name = "Señor Pablo";
    
    ParentStruct *parentC = malloc(sizeof(ParentStruct));
    parentC->id = 3;
    parentC->name = "Barack Obama";
    
    ChildStruct *childA = malloc(sizeof(ChildStruct));
    childA->id = 11;
    childA->name = "Chang Jr.";
    parentA->child = childA;
    
    ChildStruct *childB = malloc(sizeof(ChildStruct));
    childB->id = 22;
    childB->name = "Pablo Júnior";
    parentB->child = childB;

    ChildStruct *childC = malloc(sizeof(ChildStruct));
    childC->id = 33;
    childC->name = "Michelle Obama Jr.";
    parentC->child = childC;
    
    ezMap *map = ezMapNew(0, sizeof(ParentStruct));
    ezMapSet(map, (uint64_t)1, parentA);
    ezMapSet(map, (uint64_t)2, parentB);
    ezMapSet(map, (uint64_t)3, parentC);
    
    ParentStruct *parentA2 = ezMapGet(map, (uint64_t)1);
    ParentStruct *parentB2 = ezMapGet(map, (uint64_t)2);
    ParentStruct *parentC2 = ezMapGet(map, (uint64_t)3);
    
    assert(parentA->id == parentA2->id);
    assert(!strcmp(parentA->name, parentA->name));
    assert(parentA2->child->id = childA->id);
    assert(!strcmp(parentA2->child->name, childA->name));
    assert(parentB->id == parentB2->id);
    assert(!strcmp(parentB->name, parentB->name));
    assert(parentB2->child->id = childB->id);
    assert(!strcmp(parentB2->child->name, childB->name));
    assert(parentC->id == parentC2->id);
    assert(!strcmp(parentC->name, parentC->name));
    assert(parentC2->child->id = childC->id);
    assert(!strcmp(parentC2->child->name, childC->name));
    
    ezMapDel(map, (uint64_t)3);
    ParentStruct *parentC3 = ezMapGet(map, (uint64_t)3);
    assert(!parentC3);
    
    ezMapClear(map, 1);
    ParentStruct *parentA3 = ezMapGet(map, (uint64_t)1);
    ParentStruct *parentB3 = ezMapGet(map, (uint64_t)2);
    assert(!parentA3);
    assert(!parentB3);
    
    return 0;
}
