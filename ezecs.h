/* ezecs.h -- Simple entity component system (WIP)
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

#ifndef EZECS_HEADER
#define EZECS_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @header ezecs Simple Entity Component System
 * @copyright George Watson
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Taken from: https://gist.github.com/61131/7a22ac46062ee292c2c8bd6d883d28de
#define N_ARGS(...) _NARG_(__VA_ARGS__, _RSEQ())
#define _NARG_(...) _SEQ(__VA_ARGS__)
#define _SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,N,...) N
#define _RSEQ() 127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

/*!
 * @union ezEntity
 * @abstract An entity represents a general-purpose object
 * @field parts Struct containing entity information
 * @field id 32-bit unique identifier
 * @field version ezEntity generation
 * @field unused 16-bit unused
 * @field flag ezEntity type flag
 */
typedef union {
    struct {
        uint32_t id;
        uint16_t version;
        uint8_t unused;
        uint8_t flag;
    } parts;
    uint64_t id;
} ezEntity;

/*!
 * @defined EZ_ENTITY_ID
 * @abstract Get ezEntity unique 32-bit id
 * @param E ezEntity object
 * @return 32-bit unique identifier
 */
#define EZ_ENTITY_ID(E) \
    ((E).parts.id)
/*!
 * @defined EZ_ENTITY_VERSION
 * @abstract Get ezEntity object generation
 * @param E ezEntity object
 * @return ezEntity generation
 */
#define EZ_ENTITY_VERSION(E) \
    ((E).parts.version)
/*!
 * @defined EZ_ENTITY_IS_NIL
 * @abstract Check if given entity object is nil
 * @param E ezEntity object
 * @return boolean
 */
#define EZ_ENTITY_IS_NIL(E) \
    ((E).parts.id == EZ_ECS_NIL)
/*!
 * @defined EZ_ENTITY_CMP
 * @abstract Compare to entity objects
 * @param A ezEntity object 1
 * @param B ezEntity object 2
 * @return boolean
 */
#define EZ_ENTITY_CMP(A, B) \
    ((A).id == (B).id)

/*!
 * @defined EZ_ECS_NIL
 * @abstract Constatnt 32-bit denoting a nil entity
 */
#define EZ_ECS_NIL 0xFFFFFFFFull
/*!
 * @defined EZ_NIL_ENTITY
 * @abstract Constatnt ezEntity object denoting a nil entity
 */
#define EZ_NIL_ENTITY (ezEntity){.id=EZ_ECS_NIL}
/*!
 * @typedef ezWorld
 * @abstract ECS Manager, holds all ECS data
 */
typedef struct ezWorld ezWorld;

/*!
 * @defined EZ_COMPONENT
 * @abstract Convenience wrapper around ezEcsNewComponent
 * @param WORLD ezWorld instance
 * @param T Component type
 * @return Component entity object
 */
#define EZ_COMPONENT(WORLD, T) ezEcsNewComponent(WORLD, sizeof(T))
/*!
 * @defined EZ_TAG
 * @abstract Convenience wrapper around ezEcsNewComponent (size param is zero)
 * @param WORLD ezWorld instance
 * @return Component entity object
 */
#define EZ_TAG(WORLD) ezEcsNewComponent(WORLD, 0)
/*!
 * @defined EZ_QUERY
 * @abstract Convenience wrapper around ezEcsQuery
 * @param WORLD ezWorld instance
 * @param CB Query callback
 * @param ... List of components for query
 */
#define EZ_QUERY(WORLD, CB, UD, ...) ezEcsQuery(WORLD, CB, UD, (ezEntity[]){__VA_ARGS__}, sizeof((ezEntity[]){__VA_ARGS__}) / sizeof(ezEntity));
/*!
 * @defined EZ_FIELD
 * @abstract Convenience wrapper around ezEcsQueryField (For use in ezEcsQuery callbacks)
 * @param QUERY Query object
 * @param T Component type
 * @param IDX Query component index
 * @return Pointer to component data
 */
#define EZ_FIELD(QUERY, T, IDX) (T *)ezEcsQueryField(QUERY, IDX)
/*!
 * @defined EZ_SYSTEM
 * @abstract Convenience wrapper around ezEcsNewSystem
 * @param WORLD Ecs World object
 * @param CB System callback
 * @param ... List of components
 * @return System entity object
 */
#define EZ_SYSTEM(WORLD, CB, ...) ezEcsNewSystem(WORLD, CB, N_ARGS(__VA_ARGS__), __VA_ARGS__)
/*!
 * @defined EZ_PREFAB
 * @abstract Convenience wrapper around ezEcsNewPrefab
 * @param WORLD ezWorld instance
 * @param ... List of components
 * @return Prefab entity object
 */
#define EZ_PREFAB(WORLD, ...) ezEcsNewPrefab(WORLD, N_ARGS(__VA_ARGS__), __VA_ARGS__)
/*!
 * @defined EZ_ENTITY_IS_CHILD_OF
 * @abstract Convenience wrapper for EcsRelation to find all entities with relation of EcsChildOf
 * @param WORLD ezWorld instance
 * @param PARENT Parent entity
 * @param CB Query callback
 */
#define EZ_ENTITY_IS_CHILD_OF(WORLD, PARENT, CB) (ezEcsRelations((WORLD), (PARENT), ezEcsChildof, (CB)))
/*!
 * @defined EZ_ENTITY_IS_A
 * @abstract Check the type of an entity
 * @param E ezEntity object to check
 * @param TYPE ezEntityFlag to compare
 * @return boolean
 */
#define EZ_ENTITY_IS_A(E, TYPE) ((E).parts.flag == ezEcs##TYPE##Type)

/*!
 * @struct ezQuery
 * @abstract Structure to hold all components for an entity (used for queries)
 * @field componentData Array of component data
 * @field componentIndex Indexes for componentData
 * @field sizeOfComponentData Size of component data and index arrays
 * @field entityId Entity ID components belong to
 */
typedef struct ezQuery {
    void **componentData;
    ezEntity *componentIndex;
    size_t sizeOfComponentData;
    ezEntity entityId;
    void* userdata;
} ezQuery;

/*!
 * @typedef ezSystemCb
 * @abstract Typedef for System callbacks
 * @param query Struct containing component data
 */
typedef void(*ezSystemCb)(ezQuery* query);
/*!
 * @struct Prefab
 * @abstract Prefab component type (Array of component IDs)
 * @field components Array of component IDs
 * @field sizeOfComponents Size of component data
 */
typedef struct ezPrefab {
    ezEntity *components;
    size_t sizeOfComponents;
} ezPrefab;
/*!
 * @struct System
 * @abstract System component type
 * @field callback Address to system callback
 * @field components Array of component IDs
 * @field sizeOfComponents Size of component data
 */
typedef struct ezSystem {
    ezSystemCb callback;
    ezEntity *components;
    size_t sizeOfComponents;
} ezSystem;
/*!
 * @struct Relation
 * @abstract ezEntity pair component type
 * @field object Type of relationship
 * @field relation ezEntity relation
 */
typedef struct ezRelation {
    ezEntity object,
             relation;
} ezRelation;

#define ECS_BOOTSTRAP               \
    X(System, sizeof(ezSystem))     \
    X(Prefab, sizeof(ezPrefab))     \
    X(Relation, sizeof(ezRelation)) \
    X(ChildOf, 0)

#define X(NAME, _) extern ezEntity ezEcs##NAME;
ECS_BOOTSTRAP
#undef X

/*!
 * @enum ezEntityFlag
 * @abstract ezEntity type ID flags
 * @constant ezEcsNormalType Default type
 * @constant EcsComponentType Component type
 * @constant EcsSystemType System type
 * @constant EcsPrefabType Prefab type
 * @constant EcsRelationType Relation type
 */
typedef enum ezEntityFlag {
    ezEcsNormalType    = 0,
    ezEcsComponentType = (1 << 0),
    ezEcsSystemType    = (1 << 1),
    ezEcsPrefabType    = (1 << 2),
    ezEcsRelationType  = (1 << 3)
} ezEntityFlag;

/*!
 * @function ezEcsNewWorld
 * @abstract Create a new ECS instance
 * @return ezWorld instance
 */
ezWorld* ezEcsNewWorld(void);
/*!
 * @function ezEcsFreeWorld
 * @param world ezWorld instance
 * @abstract Destroy ECS instance
 */
void ezEcsFreeWorld(ezWorld **world);

/*!
 * @function ezEcsNewEntity
 * @abstract Register a new ezEntity in the ECS
 * @param world ezWorld instance
 * @return ezEntity object
 */
ezEntity ezEcsNewEntity(ezWorld *world);
/*!
 * @function ezEcsNewComponent
 * @abstract Register a new component in the ECS
 * @param world ezWorld instance
 * @param sizeOfComponent Size of component data
 * @return Component ezEntity object
 */
ezEntity ezEcsNewComponent(ezWorld *world, size_t sizeOfComponent);
/*!
 * @function ezEcsNewSystem
 * @abstract Register a new system in the ECS
 * @param world ezWorld instance
 * @param fn System callback function
 * @param sizeOfComponents Length of component array
 * @param ... Component IDs
 * @return System ezEntity object
 */
ezEntity ezEcsNewSystem(ezWorld *world, ezSystemCb fn, size_t sizeOfComponents, ...);
/*!
 * @function ezEcsNewPrefab
 * @abstract Register a new prefab in the ECS
 * @param world ezWorld instance
 * @param sizeOfComponents Length of component array
 * @param ... Component IDs
 * @return Prefab ezEntity object
 */
ezEntity ezEcsNewPrefab(ezWorld *world, size_t sizeOfComponents, ...);
/*!
 * @function ezEcsDeleteEntity
 * @abstract Remove an ezEntity from the ECS (Increase the Entity generation in entity index)
 * @param world ezWorld instance
 * @param entity ezEntity to remove
 */
void ezEcsDeleteEntity(ezWorld *world, ezEntity entity);

/*!
 * @function EcsIsValid
 * @abstract Check if ezEntity ID is in range and generation corrosponds with entity generation in the index
 * @param world ezWorld instance
 * @param entity ezEntity to check
 * @return Boolean
 */
int ezEcsIsValid(ezWorld *world, ezEntity entity);
/*!
 * @function ezEcsHas
 * @abstract Check if an ezEntity has component registered to it
 * @param world ezWorld instance
 * @param entity ezEntity to check
 * @param component Component ID to check
 * @return Boolean
 */
int ezEcsHas(ezWorld *world, ezEntity entity, ezEntity component);
/*!
 * @function ezEcsAttach
 * @abstract Attach a component to an entity
 * @param world ezWorld instance
 * @param entity ezEntity to attach to
 * @param component Component ID to attach
 */
void ezEcsAttach(ezWorld *world, ezEntity entity, ezEntity component);
/*!
 * @function ezEcsAssociate
 * @abstract Attach a relation between two entities
 * @param world ezWorld instance
 * @param entity ezEntity to attach relation to
 * @param object Type of relation (Must be a registered component/tag)
 * @param relation ezEntity ID to relate to
 */
void ezEcsAssociate(ezWorld *world, ezEntity entity, ezEntity object, ezEntity relation);
/*!
 * @function ezEcsDetach
 * @abstract Remove a component from an entity
 * @param world ezWorld instance
 * @param entity ezEntity to modify
 * @param component Component to remove
 */
void ezEcsDetach(ezWorld *world, ezEntity entity, ezEntity component);
/*!
 * @function ezEcsDisassociate
 * @abstract Remove a relation from an entity
 * @param world ezWorld instance
 * @param entity ezEntity to modify
 */
void ezEcsDisassociate(ezWorld *world, ezEntity entity);
/*!
 * @function ezEcsHasRelation
 * @abstract Check if an entity has a relation
 * @param world ezWorld instance
 * @param entity ezEntity to check
 * @param object Type of relation to check (Must be a registered component/tag)
 * @return boolean
 */
int ezEcsHasRelation(ezWorld *world, ezEntity entity, ezEntity object);
/*!
 * @function ezEcsRelated
 * @abstract Check if two entities are related (in any way)
 * @param world ezWorld instance
 * @param entity ezEntity with suspected relation
 * @param relation Potentially related entity
 * @return Boolean
 */
int ezEcsRelated(ezWorld *world, ezEntity entity, ezEntity relation);
/*!
 * @function ezEcsGet
 * @abstract Get component data from an entity
 * @param world ezWorld instance
 * @param entity ezEntity object
 * @param component Component ID for desired data
 * @return Pointer to component data
 */
void* ezEcsGet(ezWorld *world, ezEntity entity, ezEntity component);
/*!
 * @function ezEcsSet
 * @abstract Set the component data for an entity
 * @param world ezWorld instance
 * @param entity ezEntity to modify
 * @param component Component ID for desired data
 * @param data Reference to data to set
 */
void ezEcsSet(ezWorld *world, ezEntity entity, ezEntity component, const void *data);
/*!
 * @function ezEcsRelations
 * @abstract Query all entities with specified relation
 * @param world ezWorld instance
 * @param entity Parent entity
 * @param relation Type of relation (Must be a registered component/tag)
 * @param callback Query callback
 * @param userdata Userdata to send to callback
 */
void ezEcsRelations(ezWorld *world, ezEntity entity, ezEntity relation, ezSystemCb callback, void *userdata);

/*!
 * @function ezEcsStep
 * @abstract Progress ECS (run registered systems)
 * @param world ezWorld instance
 */
void ezEcsStep(ezWorld *world);
/*!
 * @function ezEcsQuery
 * @abstract Query all entities with given components
 * @param world ezWorld instance
 * @param callback Query callback
 * @param userdata Userdata to send to callback
 * @param components Array of component IDs
 * @param sizeOfComponents Length of component array
 */
void ezEcsQuery(ezWorld *world, ezSystemCb callback, void *userdata, ezEntity *components, size_t sizeOfComponents);
/*!
 * @function ezEcsQueryField
 * @abstract Retrieve a component from ezQuery object at specified index (For use in ezEcsQuery callbacks)
 * @param query Query object
 * @param index Component index
 * @return Pointer to component data
 */
void* ezEcsQueryField(ezQuery *query, size_t index);

#if defined(__cplusplus)
}
#endif
#endif // EZECS_HEADER

#if defined(EZECS_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
#include <assert.h>
#if defined(DEBUG)
#include <stdio.h>
#define ASSERT(X)                                                                              \
    do {                                                                                       \
        if (!(X)) {                                                                            \
            fprintf(stderr, "ERROR! Assertion hit! %s:%s:%d\n", __FILE__, __func__, __LINE__); \
            assert(0);                                                                         \
        }                                                                                      \
    } while(0)
#define ECS_ASSERT(X, Y, V)                                                                    \
    do {                                                                                       \
        if (!(X)) {                                                                            \
            fprintf(stderr, "ERROR! Assertion hit! %s:%s:%d\n", __FILE__, __func__, __LINE__); \
            Dump##Y(V);                                                                        \
            assert(0);                                                                         \
        }                                                                                      \
    } while(0)
#else
#define ECS_ASSERT(X, _, __) assert(X)
#define ASSERT(X) assert(X)
#endif

#define ECS_COMPOSE_ENTITY(ID, VER, TAG) \
    (ezEntity)                           \
    {                                    \
        .parts = {                       \
            .id = ID,                    \
            .version = VER,              \
            .flag = TAG                  \
        }                                \
    }

#define SAFE_FREE(X)    \
    do                  \
    {                   \
        if ((X))        \
        {               \
            free((X));  \
            (X) = NULL; \
        }               \
    } while (0)

#define ECS_DEFINE_DTOR(T, ...) \
    void Delete##T(T **p)       \
    {                           \
        T *_p = *p;             \
        if (!p || !_p)          \
            return;             \
        __VA_ARGS__             \
        SAFE_FREE(_p);          \
        *p = NULL;              \
    }

typedef struct EcsSparse {
    ezEntity *sparse;
    ezEntity *dense;
    size_t sizeOfSparse;
    size_t sizeOfDense;
} EcsSparse;

typedef struct EcsStorage {
    ezEntity componentId;
    void *data;
    size_t sizeOfData;
    size_t sizeOfComponent;
    EcsSparse *sparse;
} EcsStorage;

#if defined(DEBUG)
static void DumpEntity(ezEntity e) {
    printf("(%llx: %d, %d, %d)\n", e.id, e.parts.id, e.parts.version, e.parts.flag);
}

static void DumpSparse(EcsSparse *sparse) {
    printf("*** DUMP SPARSE ***\n");
    printf("sizeOfSparse: %zu, sizeOfDense: %zu\n", sparse->sizeOfSparse, sparse->sizeOfDense);
    printf("Sparse Contents:\n");
    for (int i = 0; i < sparse->sizeOfSparse; i++)
        DumpEntity(sparse->sparse[i]);
    printf("Dense Contents:\n");
    for (int i = 0; i < sparse->sizeOfDense; i++)
        DumpEntity(sparse->dense[i]);
    printf("*** END SPARSE DUMP ***\n");
}

static void DumpStorage(EcsStorage *storage) {
    printf("*** DUMP STORAGE ***\n");
    printf("componentId: %u, sizeOfData: %zu, sizeOfComponent: %zu\n",
           storage->componentId.parts.id, storage->sizeOfData, storage->sizeOfComponent);
    DumpSparse(storage->sparse);
    printf("*** END STORAGE DUMP ***\n");
}
#endif

static EcsSparse* NewSparse(void) {
    EcsSparse *result = malloc(sizeof(EcsSparse));
    *result = (EcsSparse){0};
    return result;
}

static void DestroyEcsSparse(EcsSparse **sparse) {
    EcsSparse *s = *sparse;
    if (!sparse || !s)
        return;
    SAFE_FREE(s->sparse);
    SAFE_FREE(s->dense);
}

static int SparseHas(EcsSparse *sparse, ezEntity e) {
    ASSERT(sparse);
    uint32_t id = EZ_ENTITY_ID(e);
    ASSERT(id != EZ_ECS_NIL);
    return (id < sparse->sizeOfSparse) && (EZ_ENTITY_ID(sparse->sparse[id]) != EZ_ECS_NIL);
}

static void SparseEmplace(EcsSparse *sparse, ezEntity e) {
    ASSERT(sparse);
    uint32_t id = EZ_ENTITY_ID(e);
    ASSERT(id != EZ_ECS_NIL);
    if (id >= sparse->sizeOfSparse) {
        const size_t newSize = id + 1;
        sparse->sparse = realloc(sparse->sparse, newSize * sizeof * sparse->sparse);
        for (size_t i = sparse->sizeOfSparse; i < newSize; i++)
            sparse->sparse[i] = EZ_NIL_ENTITY;
        sparse->sizeOfSparse = newSize;
    }
    sparse->sparse[id] = (ezEntity) { .parts = { .id = (uint32_t)sparse->sizeOfDense } };
    sparse->dense = realloc(sparse->dense, (sparse->sizeOfDense + 1) * sizeof * sparse->dense);
    sparse->dense[sparse->sizeOfDense++] = e;
}

static size_t SparseRemove(EcsSparse *sparse, ezEntity e) {
    ASSERT(sparse);
    ECS_ASSERT(SparseHas(sparse, e), Sparse, sparse);

    const uint32_t id = EZ_ENTITY_ID(e);
    uint32_t pos = EZ_ENTITY_ID(sparse->sparse[id]);
    ezEntity other = sparse->dense[sparse->sizeOfDense-1];

    sparse->sparse[EZ_ENTITY_ID(other)] = (ezEntity) { .parts = { .id = pos } };
    sparse->dense[pos] = other;
    sparse->sparse[id] = EZ_NIL_ENTITY;
    sparse->dense = realloc(sparse->dense, --sparse->sizeOfDense * sizeof * sparse->dense);

    return pos;
}

static size_t SparseAt(EcsSparse *sparse, ezEntity e) {
    ASSERT(sparse);
    uint32_t id = EZ_ENTITY_ID(e);
    ASSERT(id != EZ_ECS_NIL);
    return EZ_ENTITY_ID(sparse->sparse[id]);
}

static EcsStorage* NewStorage(ezEntity id, size_t sz) {
    EcsStorage *result = malloc(sizeof(EcsStorage));
    *result = (EcsStorage) {
        .componentId = id,
        .sizeOfComponent = sz,
        .sizeOfData = 0,
        .data = NULL,
        .sparse = NewSparse()
    };
    return result;
}

static void DestroyEcsStorage(EcsStorage **storage) {
    EcsStorage *s = *storage;
    if (!storage || !s)
        return;
    DestroyEcsSparse(&s->sparse);
}

static int StorageHas(EcsStorage *storage, ezEntity e) {
    ASSERT(storage);
    ASSERT(!EZ_ENTITY_IS_NIL(e));
    return SparseHas(storage->sparse, e);
}

static void* StorageEmplace(EcsStorage *storage, ezEntity e) {
    ASSERT(storage);
    storage->data = realloc(storage->data, (storage->sizeOfData + 1) * sizeof(char) * storage->sizeOfComponent);
    storage->sizeOfData++;
    void *result = &((char*)storage->data)[(storage->sizeOfData - 1) * sizeof(char) * storage->sizeOfComponent];
    SparseEmplace(storage->sparse, e);
    return result;
}

static void StorageRemove(EcsStorage *storage, ezEntity e) {
    ASSERT(storage);
    size_t pos = SparseRemove(storage->sparse, e);
    memmove(&((char*)storage->data)[pos * sizeof(char) * storage->sizeOfComponent],
            &((char*)storage->data)[(storage->sizeOfData - 1) * sizeof(char) * storage->sizeOfComponent],
            storage->sizeOfComponent);
    storage->data = realloc(storage->data, --storage->sizeOfData * sizeof(char) * storage->sizeOfComponent);
}

static void* StorageAt(EcsStorage *storage, size_t pos) {
    ASSERT(storage);
    ECS_ASSERT(pos < storage->sizeOfData, Storage, storage);
    return &((char*)storage->data)[pos * sizeof(char) * storage->sizeOfComponent];
}

static void* StorageGet(EcsStorage *storage, ezEntity e) {
    ASSERT(storage);
    ASSERT(!EZ_ENTITY_IS_NIL(e));
    return StorageAt(storage, SparseAt(storage->sparse, e));
}

struct ezWorld {
    EcsStorage **storages;
    size_t sizeOfStorages;
    ezEntity *entities;
    size_t sizeOfEntities;
    uint32_t *recyclable;
    size_t sizeOfRecyclable;
    uint32_t nextAvailableId;
};

#define X(NAME, _) ezEntity ezEcs##NAME = (ezEntity) { .id = -1 };
ECS_BOOTSTRAP
#undef X

ezWorld* ezEcsNewWorld(void) {
    ezWorld *result = malloc(sizeof(ezWorld));
    *result = (ezWorld){0};
    result->nextAvailableId = EZ_ECS_NIL;

#define X(NAME, SZ) ezEcs##NAME = ezEcsNewComponent(result, SZ);
    ECS_BOOTSTRAP
#undef X

    return result;
}

#define DEL_TYPES \
    X(System, 0)  \
    X(Prefab, 1)

#define X(TYPE, N)                                                           \
    case N: {                                                                \
        for (int j = 0; j < storage->sparse->sizeOfDense; j++) {             \
            ez##TYPE *type = StorageGet(storage, storage->sparse->dense[j]); \
            free(type->components);                                          \
        }                                                                    \
        break;                                                               \
    }
void ezEcsFreeWorld(ezWorld **world) {
    ezWorld *w = *world;
    if (!world || !w)
        return;
    if (w->storages)
        for (int i = 0; i < w->sizeOfStorages; i++) {
            EcsStorage *storage = w->storages[i];
            switch (i) {
                DEL_TYPES
            }
            DestroyEcsStorage(&storage);
        }
    SAFE_FREE(w->storages);
    SAFE_FREE(w->entities);
    SAFE_FREE(w->recyclable);
}
#undef X

int ezEcsIsValid(ezWorld *world, ezEntity e) {
    ASSERT(world);
    uint32_t id = EZ_ENTITY_ID(e);
    return id < world->sizeOfEntities && EZ_ENTITY_CMP(world->entities[id], e);
}

static ezEntity EcsNewEntityType(ezWorld *world, uint8_t type) {
    ASSERT(world);
    if (world->sizeOfRecyclable) {
        uint32_t idx = world->recyclable[world->sizeOfRecyclable-1];
        ezEntity e = world->entities[idx];
        ezEntity new = ECS_COMPOSE_ENTITY(EZ_ENTITY_ID(e), EZ_ENTITY_VERSION(e), type);
        world->entities[idx] = new;
        world->recyclable = realloc(world->recyclable, --world->sizeOfRecyclable * sizeof(uint32_t));
        return new;
    } else {
        world->entities = realloc(world->entities, ++world->sizeOfEntities * sizeof(ezEntity));
        ezEntity e = ECS_COMPOSE_ENTITY((uint32_t)world->sizeOfEntities-1, 0, type);
        world->entities[world->sizeOfEntities-1] = e;
        return e;
    }
}

ezEntity ezEcsNewEntity(ezWorld *world) {
    return EcsNewEntityType(world, ezEcsNormalType);
}

static EcsStorage* EcsFind(ezWorld *world, ezEntity e) {
    for (int i = 0; i < world->sizeOfStorages; i++)
        if (EZ_ENTITY_ID(world->storages[i]->componentId) == EZ_ENTITY_ID(e))
            return world->storages[i];
    return NULL;
}

static EcsStorage* EcsAssure(ezWorld *world, ezEntity componentId, size_t sizeOfComponent) {
    EcsStorage *found = EcsFind(world, componentId);
    if (found)
        return found;
    EcsStorage *new = NewStorage(componentId, sizeOfComponent);
    world->storages = realloc(world->storages, (world->sizeOfStorages + 1) * sizeof * world->storages);
    world->storages[world->sizeOfStorages++] = new;
    return new;
}

int ezEcsHas(ezWorld *world, ezEntity entity, ezEntity component) {
    ASSERT(world);
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, component), Entity, component);
    return StorageHas(EcsFind(world, component), entity);
}

ezEntity ezEcsNewComponent(ezWorld *world, size_t sizeOfComponent) {
    ezEntity e = EcsNewEntityType(world, ezEcsComponentType);
    return EcsAssure(world, e, sizeOfComponent) ? e : EZ_NIL_ENTITY;
}

ezEntity ezEcsNewSystem(ezWorld *world, ezSystemCb fn, size_t sizeOfComponents, ...) {
    ezEntity e = EcsNewEntityType(world, ezEcsSystemType);
    ezEcsAttach(world, e, ezEcsSystem);
    ezSystem *c = ezEcsGet(world, e, ezEcsSystem);
    c->callback = fn;
    c->sizeOfComponents = sizeOfComponents;
    c->components = malloc(sizeof(ezEntity) * sizeOfComponents);

    va_list args;
    va_start(args, sizeOfComponents);
    for (int i = 0; i < sizeOfComponents; i++)
        c->components[i] = va_arg(args, ezEntity);
    va_end(args);
    return e;
}

ezEntity ezEcsNewPrefab(ezWorld *world, size_t sizeOfComponents, ...) {
    ezEntity e = EcsNewEntityType(world, ezEcsPrefabType);
    ezEcsAttach(world, e, ezEcsPrefab);
    ezPrefab *c = ezEcsGet(world, e, ezEcsPrefab);
    c->sizeOfComponents = sizeOfComponents;
    c->components = malloc(sizeof(ezEntity) * sizeOfComponents);

    va_list args;
    va_start(args, sizeOfComponents);
    for (int i = 0; i < sizeOfComponents; i++)
        c->components[i] = va_arg(args, ezEntity);
    va_end(args);
    return e;
}

void ezEcsDeleteEntity(ezWorld *world, ezEntity e) {
    ASSERT(world);
    ECS_ASSERT(ezEcsIsValid(world, e), Entity, e);
    switch (e.parts.flag) {
#define X(TYPE, _)                                         \
        case ezEcs##TYPE##Type: {                          \
            ez##TYPE *s = ezEcsGet(world, e, ezEcs##TYPE); \
            if (s && s->components)                        \
                free(s->components);                       \
            break;                                         \
        }
        DEL_TYPES
#undef X
    }
    for (size_t i = world->sizeOfStorages; i; --i)
        if (world->storages[i - 1] && SparseHas(world->storages[i - 1]->sparse, e))
            StorageRemove(world->storages[i - 1], e);
    uint32_t id = EZ_ENTITY_ID(e);
    world->entities[id] = ECS_COMPOSE_ENTITY(id, EZ_ENTITY_VERSION(e) + 1, 0);
    world->recyclable = realloc(world->recyclable, ++world->sizeOfRecyclable * sizeof(uint32_t));
    world->recyclable[world->sizeOfRecyclable-1] = id;
}

void ezEcsAttach(ezWorld *world, ezEntity entity, ezEntity component) {
    switch (component.parts.flag) {
        case ezEcsRelationType: // Use EcsRelation()
        case ezEcsSystemType: // NOTE: potentially could be used for some sort of event system
            ASSERT(0);
        case ezEcsPrefabType: {
            ezPrefab *c = ezEcsGet(world, component, ezEcsPrefab);
            for (int i = 0; i < c->sizeOfComponents; i++) {
                if (EZ_ENTITY_IS_NIL(c->components[i]))
                    break;
                ezEcsAttach(world, entity, c->components[i]);
            }
            break;
        }
        case ezEcsComponentType:
        default: {
            ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
            ECS_ASSERT(ezEcsIsValid(world, component), Entity, component);
            EcsStorage *storage = EcsFind(world, component);
            ASSERT(storage);
            StorageEmplace(storage, entity);
            break;
        }
    }
}

void ezEcsAssociate(ezWorld *world, ezEntity entity, ezEntity object, ezEntity relation) {
    ASSERT(world);
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, object), Entity, object);
    ECS_ASSERT(EZ_ENTITY_IS_A(object, Component), Entity, object);
    ECS_ASSERT(ezEcsIsValid(world, relation), Entity, relation);
    ECS_ASSERT(EZ_ENTITY_IS_A(relation, Normal), Entity, relation);
    ezEcsAttach(world, entity, ezEcsRelation);
    ezRelation *pair = ezEcsGet(world, entity, ezEcsRelation);
    pair->object = object;
    pair->relation = relation;
}

void ezEcsDetach(ezWorld *world, ezEntity entity, ezEntity component) {
    ASSERT(world);
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, component), Entity, component);
    EcsStorage *storage = EcsFind(world, component);
    ASSERT(storage);
    ECS_ASSERT(StorageHas(storage, entity), Storage, storage);
    StorageRemove(storage, entity);
}

void ezEcsDisassociate(ezWorld *world, ezEntity entity) {
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsHas(world, entity, ezEcsRelation), Entity, entity);
    ezEcsDetach(world, entity, ezEcsRelation);
}

int ezEcsHasRelation(ezWorld *world, ezEntity entity, ezEntity object) {
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, object), Entity, object);
    EcsStorage *storage = EcsFind(world, ezEcsRelation);
    if (!storage)
        return 0;
    ezRelation *relation = StorageGet(storage, entity);
    if (!relation)
        return 0;
    return EZ_ENTITY_CMP(relation->object, object);
}

int ezEcsRelated(ezWorld *world, ezEntity entity, ezEntity relation) {
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, relation), Entity, relation);
    EcsStorage *storage = EcsFind(world, ezEcsRelation);
    if (!storage)
        return 0;
    ezRelation *_relation = StorageGet(storage, entity);
    if (!_relation)
        return 0;
    return EZ_ENTITY_CMP(_relation->relation, relation);
}

void* ezEcsGet(ezWorld *world, ezEntity entity, ezEntity component) {
    ASSERT(world);
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, component), Entity, component);
    EcsStorage *storage = EcsFind(world, component);
    ASSERT(storage);
    return StorageHas(storage, entity) ? StorageGet(storage, entity) : NULL;
}

void ezEcsSet(ezWorld *world, ezEntity entity, ezEntity component, const void *data) {
    ASSERT(world);
    ECS_ASSERT(ezEcsIsValid(world, entity), Entity, entity);
    ECS_ASSERT(ezEcsIsValid(world, component), Entity, component);
    EcsStorage *storage = EcsFind(world, component);
    ASSERT(storage);

    void *componentData = StorageHas(storage, entity) ?
                                    StorageGet(storage, entity) :
                                    StorageEmplace(storage, entity);
    ASSERT(componentData);
    memcpy(componentData, data, storage->sizeOfComponent);
}

static void QueryFree(ezQuery *query) {
    SAFE_FREE(query->componentIndex);
    SAFE_FREE(query->componentData);
}

void ezEcsRelations(ezWorld *world, ezEntity entity, ezEntity relation, ezSystemCb cb, void *userdata) {
    EcsStorage *pairs = EcsFind(world, ezEcsRelation);
    for (size_t i = 0; i < world->sizeOfEntities; i++) {
        ezEntity e = world->entities[i];
        if (StorageHas(pairs, e)) {
            ezRelation *pair = StorageGet(pairs, e);
            if (EZ_ENTITY_CMP(pair->object, relation) && EZ_ENTITY_CMP(pair->relation, entity)) {
                ezQuery query = { .entityId = e };
                query.componentData = malloc(sizeof(void*));
                query.componentIndex = malloc(sizeof(ezEntity));
                query.sizeOfComponentData = 1;
                query.componentIndex[0] = relation;
                query.componentData[0] = (void*)pair;
                query.userdata = userdata;
                cb(&query);
                QueryFree(&query);
            }
        }
    }
}

void ezEcsStep(ezWorld *world) {
    EcsStorage *storage = world->storages[EZ_ENTITY_ID(ezEcsSystem)];
    for (int i = 0; i < storage->sparse->sizeOfDense; i++) {
        ezSystem *system = StorageGet(storage, storage->sparse->dense[i]);
        ezEcsQuery(world, system->callback, NULL, system->components, system->sizeOfComponents);
    }
}

void ezEcsQuery(ezWorld *world, ezSystemCb cb, void *userdata, ezEntity *components, size_t sizeOfComponents) {
    for (size_t e = 0; e < world->sizeOfEntities; e++) {
        int hasComponents = 1;
        ezQuery query = {
            .componentData = NULL,
            .componentIndex = NULL,
            .sizeOfComponentData = 0,
            .entityId = world->entities[e]
        };
        for (size_t i = 0; i < sizeOfComponents; i++) {
            EcsStorage *storage = EcsFind(world, components[i]);

            if (StorageHas(storage, world->entities[e])) {
                query.sizeOfComponentData++;
                query.componentData = realloc(query.componentData, query.sizeOfComponentData * sizeof(void*));
                query.componentIndex = realloc(query.componentIndex, query.sizeOfComponentData * sizeof(ezEntity));
                query.componentIndex[query.sizeOfComponentData-1] = components[i];
                query.componentData[query.sizeOfComponentData-1] = StorageGet(storage, world->entities[e]);
                query.userdata = userdata;
            } else {
                hasComponents = 0;
                break;
            }
        }

        if (hasComponents)
            cb(&query);
        QueryFree(&query);
    }
}

void* ezEcsQueryField(ezQuery *query, size_t index) {
    return index >= query->sizeOfComponentData || EZ_ENTITY_IS_NIL(query->componentIndex[index]) ? NULL : query->componentData[index];
}
#endif
