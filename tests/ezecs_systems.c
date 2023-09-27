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
    TEST(!!ezViewField(view, Position, 0), 1);
    posCbCounter++;
}

static int moveCbCounter = 0;
static void moveCb(ezView *view) {
    TEST(!!ezViewField(view, Position, 0), 1);
    TEST(!!ezViewField(view, Velocity, 1), 1);
    moveCbCounter++;
}

int main(int argc, const char *argv[]) {
    ezWorld *world = ezEcsNewWorld();
    
    ezEntity e1 = ezEcsNewEntity(world);
    ezEntity position = ezNewComponent(world, Position);
    ezEcsAttach(world, e1, position);
    ezEntity e2 = ezEcsNewEntity(world);
    ezEcsAttach(world, e2, position);
    
    ezEntity velocity = ezNewComponent(world, Velocity);
    ezEcsAttach(world, e2, position);
    ezQuery(world, posCb, position);
    ezEcsAttach(world, e1, velocity);
    ezQuery(world, moveCb, position, velocity);
    TEST(posCbCounter, 2);
    TEST(moveCbCounter, 1);
    
    ezEntity e4 = ezEcsNewEntity(world);
    ezEcsAttach(world, e4, position);
    ezEcsAttach(world, e4, velocity);
    ezEntity testSystemA = ezNewSystem(world, posCb, position);
    ezEntity testSystemB = ezNewSystem(world, moveCb, position, velocity);
    
    ezEcsStep(world);
    TEST(posCbCounter, 5);
    TEST(moveCbCounter, 3);
    
    ezEcsDeleteEntity(world, testSystemA);
    ezEcsStep(world);
    TEST(posCbCounter, moveCbCounter);
    
    ezEntity e5 = ezEcsNewEntity(world);
    ezEntity testPrefab = ezNewPrefab(world, position, velocity);
    ezEcsAttach(world, e5, testPrefab);
    TEST(ezEcsHas(world, e5, position), 1);
    TEST(ezEcsHas(world, e5, velocity), 1);
    
    TEST(ezEntityIsA(testPrefab, Prefab), 1);
    TEST(ezEntityIsA(testSystemB, System), 1);
    TEST(ezEntityIsA(position, Component), 1);
    TEST(ezEntityIsA(e5, Component), 0);

    ezEcsFreeWorld(&world);
    return EXIT_SUCCESS;
}
