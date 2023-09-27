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

typedef struct {
    float x, y;
} Position;

typedef struct {
    float x, y;
} Velocity;

int main(int argc, char *argv[]) {
    ezWorld *world = ezEcsNewWorld();
    
    ezEntity e1 = ezEcsNewEntity(world);
    ezEntity position = ezNewComponent(world, Position);
    ezEcsAttach(world, e1, position);
    ezEntity e2 = ezEcsNewEntity(world);
    ezEcsAttach(world, e2, position);
    ezEntity e3 = ezEcsNewEntity(world);
    TEST(ezEcsHas(world, e1, position), 1);
    TEST(ezEcsHas(world, e2, position), 1);
    TEST(ezEcsHas(world, e3, position), 0);
    
    Position *p1 = ezEcsGet(world, e1, position);
    Position *p2 = ezEcsGet(world, e2, position);
    Position *p3 = ezEcsGet(world, e3, position);
    TEST(!!p1, 1);
    TEST(!!p2, 1);
    TEST(!!p3, 0);
    
    ezEcsSet(world, e1, position, &(Position) { .x = 1.f, .y = 2.f });
    ezEcsSet(world, e2, position, &(Position) { .x = 3.f, .y = 4.f });
    TEST(p1->x, 1.f);
    TEST(p1->y, 2.f);
    TEST(p2->x, 3.f);
    TEST(p2->y, 4.f);
    
    ezEcsDetach(world, e2, position);
    Position *p1a = ezEcsGet(world, e1, position);
    TEST(ezEcsHas(world, e1, position), 1);
    TEST(p1a->x, 1.f);
    TEST(p1a->y, 2.f);
    TEST(ezEcsHas(world, e2, position), 0);
    TEST(ezEcsGet(world, e2, position), NULL);
    TEST(ezEcsHas(world, e3, position), 0);
    TEST(ezEcsGet(world, e3, position), NULL);
    
    ezEntity testTag = ezNewTag(world);
    ezEcsAttach(world, e1, testTag);
    TEST(ezEcsHas(world, e1, testTag), 1);
    TEST(ezEcsHas(world, e2, testTag), 0);
    
    ezEcsFreeWorld(&world);
    return EXIT_SUCCESS;
}
