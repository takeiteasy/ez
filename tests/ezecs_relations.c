#define EZECS_IMPLEMENTATION
#include "ezecs.h"
#include <stdio.h>

static int TEST_COUNTER = 0;
#define TEST(OBJECT, RESULT)                                                                              \
    do                                                                                                    \
    {                                                                                                     \
        int res = (OBJECT) == (RESULT);                                                                   \
        printf("[TEST%02d:%s] %s == %s\n", ++TEST_COUNTER, res ? "SUCCESS" : "FAILED", #OBJECT, #RESULT); \
        if (!res)                                                                                         \
            exit(EXIT_FAILURE);                                                                           \
    } while (0)

static int childCbCounter = 0;
static void childCb(ezView *view) {
    TEST(!!EZ_FIELD(view, ezRelation, 0), 1);
    childCbCounter++;
}

int main(int argc, char *argv[]) {
    ezWorld *world = ezEcsNewWorld();
    
    ezEntity e1 = ezEcsNewEntity(world);
    ezEntity parent = ezEcsNewEntity(world);
    ezEntity child = ezEcsNewEntity(world);
    ezEcsAssociate(world, child, ezEcsChildof, parent);
    ezEcsAssociate(world, e1, ezEcsChildof, parent);
    EZ_ENTITY_IS_CHILD_OF(world, parent, childCb);
    TEST(childCbCounter, 2);
    TEST(ezEcsHasRelation(world, child, ezEcsChildof), 1);
    TEST(ezEcsRelated(world, child, parent), 1);
    ezEcsDisassociate(world, e1);
    EZ_ENTITY_IS_CHILD_OF(world, parent, childCb);
    TEST(childCbCounter, 3);
    
    ezEcsFreeWorld(&world);
    return EXIT_SUCCESS;
}
