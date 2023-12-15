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

static int posCbCounter = 0;
static void posCb(ezView *view) {
    TEST(!!EZ_FIELD(view, Position, 0), 1);
    posCbCounter++;
}

static int moveCbCounter = 0;
static void moveCb(ezView *view) {
    TEST(!!EZ_FIELD(view, Position, 0), 1);
    TEST(!!EZ_FIELD(view, Velocity, 1), 1);
    moveCbCounter++;
}

int main(int argc, const char *argv[]) {
    ezWorld *world = ezEcsNewWorld();
    
    ezEntity e1 = ezEcsNewEntity(world);
    ezEntity position = EZ_COMPONENT(world, Position);
    ezEcsAttach(world, e1, position);
    ezEntity e2 = ezEcsNewEntity(world);
    ezEcsAttach(world, e2, position);
    
    ezEntity velocity = EZ_COMPONENT(world, Velocity);
    ezEcsAttach(world, e2, position);
    EZ_QUERY(world, posCb, position);
    ezEcsAttach(world, e1, velocity);
    EZ_QUERY(world, moveCb, position, velocity);
    TEST(posCbCounter, 2);
    TEST(moveCbCounter, 1);
    
    ezEntity e4 = ezEcsNewEntity(world);
    ezEcsAttach(world, e4, position);
    ezEcsAttach(world, e4, velocity);
    ezEntity testSystemA = EZ_SYSTEM(world, posCb, position);
    ezEntity testSystemB = EZ_SYSTEM(world, moveCb, position, velocity);
    
    ezEcsStep(world);
    TEST(posCbCounter, 5);
    TEST(moveCbCounter, 3);
    
    ezEcsDeleteEntity(world, testSystemA);
    ezEcsStep(world);
    TEST(posCbCounter, moveCbCounter);
    
    ezEntity e5 = ezEcsNewEntity(world);
    ezEntity testPrefab = EZ_PREFAB(world, position, velocity);
    ezEcsAttach(world, e5, testPrefab);
    TEST(ezEcsHas(world, e5, position), 1);
    TEST(ezEcsHas(world, e5, velocity), 1);
    
    TEST(EZ_ENTITY_IS_A(testPrefab, Prefab), 1);
    TEST(EZ_ENTITY_IS_A(testSystemB, System), 1);
    TEST(EZ_ENTITY_IS_A(position, Component), 1);
    TEST(EZ_ENTITY_IS_A(e5, Component), 0);

    ezEcsFreeWorld(&world);
    return EXIT_SUCCESS;
}
