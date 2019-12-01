#include "entity.h"

#include <stdlib.h>
#include <string.h>

//==================================================
// Entities
//==================================================

static EntityId_t s_nextEntityId;

EntityId_t CreateEntity()
{
	return ++s_nextEntityId;
}

//==================================================
// ComponentArray
//==================================================

static struct ComponentArray* s_componentArraysHead;

void CreateComponentArray(struct ComponentArray* componentArray, size_t componentSize, InitComponent_t init, DeinitComponent_t deinit)
{
	componentArray->init = init;
	componentArray->deinit = deinit;
	componentArray->data = NULL;
	componentArray->usedCount = 0;
	componentArray->totalCount = 0;
	componentArray->componentSize = (int32_t)componentSize;

	componentArray->next = s_componentArraysHead;
	s_componentArraysHead = componentArray;
}

static uint8_t* GetComponentDataArray(struct ComponentArray* componentArray)
{
	return (uint8_t*)((EntityId_t*)componentArray->data + componentArray->totalCount);
}

void DestroyAllComponentArrays()
{
	struct ComponentArray* componentArray = s_componentArraysHead;
	while (componentArray)
	{
		if (componentArray->deinit)
		{
			uint8_t* data = GetComponentDataArray(componentArray);
			for (int32_t i = 0; i < componentArray->usedCount; ++i)
			{
				componentArray->deinit(data);
				data += componentArray->componentSize;
			}
		}

		free(componentArray->data);
		componentArray = componentArray->next;
	}
	s_componentArraysHead = NULL;
}

void RemoveAllEntityComponents(EntityId_t entityId)
{
	struct ComponentArray* componentArray = s_componentArraysHead;
	while (componentArray)
	{
		RemoveEntityComponent(componentArray, entityId);
		componentArray = componentArray->next;
	}
}

static EntityId_t* GetEntityIds(struct ComponentArray* componentArray)
{
	return (EntityId_t*)componentArray->data;
}

static const EntityId_t* GetEntityIdsConst(const struct ComponentArray* componentArray)
{
	return (const EntityId_t*)componentArray->data;
}

void* GetComponentData(struct ComponentArray* componentArray, size_t index)
{
	uint8_t* componentDatas = (uint8_t*)((EntityId_t*)componentArray->data + componentArray->totalCount);
	return componentDatas + index * componentArray->componentSize;
}

bool HasComponent(const struct ComponentArray* componentArray, EntityId_t entityId)
{
	const EntityId_t* entityIds = GetEntityIdsConst(componentArray);
	for (int32_t i = 0; i < componentArray->usedCount; ++i)
	{
		if (entityIds[i] == entityId)
		{
			return true;
		}
	}
	return false;
}

void* FindComponent(struct ComponentArray* componentArray, EntityId_t entityId)
{
	EntityId_t* entityIds = GetEntityIds(componentArray);
	for (int32_t i = 0; i < componentArray->usedCount; ++i)
	{
		if (entityIds[i] == entityId)
		{
			return GetComponentData(componentArray, i);
		}
	}
	return NULL;
}

void* AddEntityComponent(struct ComponentArray* componentArray, EntityId_t entityId)
{
	// TODO(cj): Make sure that we do not insert the same entityId twice.

	if (componentArray->usedCount == componentArray->totalCount)
	{
		// If the component array does not have any free slots, we need to re-allocate.
		// TODO(cj): Test realloc instead of using malloc right away.

		const size_t slotSize = sizeof(EntityId_t) + componentArray->componentSize;

		// TODO(cj): Maybe use geometric grow factor here.
		// We use a small constant for now to provoke constant reallocation for testing purposes.
		const int32_t newTotalCount = componentArray->totalCount + 1;

		void* newData = malloc(newTotalCount * slotSize);
		// TODO(cj): Handle out-of-memory situation.

		EntityId_t* newEntityIds = (EntityId_t*)newData;
		void* newComponentDatas = (EntityId_t*)newData + newTotalCount;

		memcpy(newEntityIds, GetEntityIds(componentArray), componentArray->usedCount * sizeof(EntityId_t));
		memcpy(newComponentDatas, GetComponentData(componentArray, 0), componentArray->usedCount * componentArray->componentSize);

		free(componentArray->data);

		componentArray->data = newData;
		componentArray->totalCount = newTotalCount;
	}


	EntityId_t* const entityIds = GetEntityIds(componentArray);

	// TODO(cj): We could do binary search here.
	int32_t insertIndex = 0;
	while (entityIds[insertIndex] < entityId && insertIndex < componentArray->usedCount)
	{
		insertIndex++;
	}

	// Shift indices one over.
	for (int32_t i = componentArray->usedCount; i > insertIndex; --i)
	{
		entityIds[i] = entityIds[i - 1];
	}

	// Shift components one over.
	for (int32_t i = componentArray->usedCount; i > insertIndex; --i)
	{
		void* target = GetComponentData(componentArray, i);
		void* source = GetComponentData(componentArray, i - 1);
		memcpy(target, source, componentArray->componentSize);
	}

	// Now we can insert the new component.
	entityIds[insertIndex] = entityId;

	componentArray->usedCount++;

	return GetComponentData(componentArray, insertIndex);
}

bool RemoveEntityComponent(struct ComponentArray* componentArray, EntityId_t entityId)
{
	EntityId_t* const entityIds = GetEntityIds(componentArray);
	uint8_t* const dataArray = GetComponentDataArray(componentArray);
	for (int32_t i = 0; i < componentArray->usedCount; ++i)
	{
		if (entityIds[i] == entityId)
		{
			if (componentArray->deinit)
			{
				componentArray->deinit(dataArray + componentArray->componentSize * i);
			}

			// TODO(cj): Is it possible to do a memcpy (move) here instead?
			for (; i < componentArray->usedCount - 1; ++i)
			{
				entityIds[i] = entityIds[i + 1];
				memcpy(dataArray + componentArray->componentSize * i, dataArray + componentArray->componentSize * (i + 1), componentArray->componentSize);
			}

			componentArray->usedCount--;

			return true;
		}
	}
	return false;
}

//==================================================
// EntityIterator
//==================================================

void InitEntityIterator(struct EntityIterator* entIt, struct ComponentArray** compArr, int32_t count)
{
	entIt->componentArrays = compArr;
	entIt->count = count;

	entIt->it = 0;
	entIt->last = MAX_ENTITY_ID;

	for (int32_t i = 0; i < count; ++i)
	{
		if (!compArr[i]->usedCount)
		{
			// One of the arrays is empty, so there can be no entity that has all the required components.
			entIt->done = true;
			return;
		}

		const EntityId_t* entityIds = GetEntityIdsConst(compArr[i]);
		EntityId_t minId = entityIds[0];
		EntityId_t maxId = entityIds[compArr[i]->usedCount - 1];

		if (entIt->it < minId)
		{
			entIt->it = minId;
		}

		if (entIt->last > maxId)
		{
			entIt->last = maxId;
		}
	}

	entIt->done = entIt->last < entIt->it; // TODO(cj): Write test for this.
}

bool NextEntityId(struct EntityIterator* entIt, EntityId_t* outEntityId)
{
	while (!entIt->done)
	{
		EntityId_t entityId = entIt->it;
		entIt->it++;

		bool found = true;
		for (int32_t i = 0; i < entIt->count; ++i)
		{
			if (!HasComponent(entIt->componentArrays[i], entityId))
			{
				found = false;
				break;
			}
		}

		entIt->done = entityId == entIt->last;

		if (found)
		{
			*outEntityId = entityId;
			return true;
		}
	}

	return false;
}
