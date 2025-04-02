#include "ents.h"

#include <stdio.h>

void ogt_init_entity_system(EntityManager_t* Manager)
{
	*Manager = (EntityManager_t){ 0 };

	Manager->EntIndex = 0;
	Manager->FreeIndexCount = MAX_ENTITIES;

	for (unsigned int i = 0; i < MAX_ENTITIES; ++i)
		Manager->FreeEntIndices[i] = i;
}

void ogt_create_entity(Entity_t* Entity) // TODO: Class names
{
	unsigned int EntityIndex;

	if (EntityManager.FreeIndexCount > 0)
        EntityIndex = EntityManager.FreeEntIndices[--EntityManager.FreeIndexCount];
	else
	{
		if (EntityManager.EntIndex > MAX_ENTITIES)
		{
			printf("Too many entites! %d\n", EntityManager.EntIndex);

			Entity->Valid = 0;

			return;
		}

		EntityIndex = EntityManager.EntIndex++;
	}

	Entity->Valid = 1;
	Entity->Index = EntityIndex;

	Entity->Health = 0;

	Entity->OnCreation = NULL;
	Entity->OnDeletion = NULL;
	Entity->Think = NULL;
	Entity->Render = NULL;

	EntityManager.Entities[EntityIndex] = Entity;
}

void ogt_delete_entity(Entity_t* Entity)
{
	if (Entity->Valid)
	{
		if (Entity->OnDeletion)
			Entity->OnDeletion(Entity);

        unsigned int EntityIndex = Entity->Index;
        Entity->Valid = 0;

        EntityManager.FreeEntIndices[EntityManager.FreeIndexCount++] = EntityIndex;
    }
}

void ogt_think_entities(float DeltaTime)
{
	for (unsigned int i = 0; i < EntityManager.EntIndex; ++i)
	{
		Entity_t* Entity = EntityManager.Entities[i];

		if (Entity->Valid && Entity->Think)
			Entity->Think(Entity, DeltaTime);
	}
}

void ogt_render_entities(float DeltaTime)
{
	for (unsigned int i = 0; i < EntityManager.EntIndex; ++i)
	{
		Entity_t* Entity = EntityManager.Entities[i];

		if (Entity->Valid && Entity->Render)
			Entity->Render(Entity, DeltaTime);
	}
}
