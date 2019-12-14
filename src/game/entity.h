#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//==================================================
// Entities
//==================================================

#define MAX_ENTITY_ID 10000 // TODO(cj): Use MAX_INT32 here

typedef uint32_t EntityId_t;

EntityId_t CreateEntity();

//==================================================
// ComponentArray
//==================================================

typedef void(*InitComponent_t)(void*);
typedef void(*DeinitComponent_t)(void*);

struct ComponentArray
{
	struct ComponentArray* next;
	InitComponent_t init; // TODO(cj): Missing tests.
	DeinitComponent_t deinit;
	void* data;
	int32_t usedCount; // Number of used slots (actually store component data).
	int32_t totalCount; // Total number of slots.
	int32_t componentSize;
};

void CreateComponentArray(struct ComponentArray* componentArray, size_t componentSize, InitComponent_t init, DeinitComponent_t deinit);

// TODO(cj): Missing tests.
void DestroyAllComponentArrays();
void RemoveAllEntityComponents(EntityId_t entityId);

bool HasComponent(const struct ComponentArray* componentArray, EntityId_t entityId);

void* FindComponent(struct ComponentArray* componentArray, EntityId_t entityId);

void* GetComponentData(struct ComponentArray* componentArray, size_t index);

void* AddEntityComponent(struct ComponentArray* componentArray, EntityId_t entityId);
bool RemoveEntityComponent(struct ComponentArray* componentArray, EntityId_t entityId);

//==================================================
// EntityIterator
//==================================================

struct EntityIterator
{
	struct ComponentArray** componentArrays;
	int32_t count;

	EntityId_t last;
	EntityId_t it;

	bool done;
};

void InitEntityIterator(struct EntityIterator* entIt, struct ComponentArray** compArr, int32_t count);
bool NextEntityId(struct EntityIterator* entIt, EntityId_t* entityId);

#ifdef __cplusplus
}
#endif
