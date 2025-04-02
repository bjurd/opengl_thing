#include "ents.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hashmap/map.h>

void ogt_init_entity_system(EntityManager_t* Manager)
{
	*Manager = (EntityManager_t){ 0 };

	Manager->EntIndex = 0;
	Manager->FreeIndexCount = MAX_ENTITIES;
	Manager->EntityClassMap = hashmap_create();

	for (unsigned int i = 0; i < MAX_ENTITIES; ++i)
		Manager->FreeEntIndices[i] = i;
}

void ogt_register_entity_class(const char* Class, CreationFn OnCreation, DeletionFn OnDeletion, ThinkFn Think, RenderFn Render)
{
	EntityClass_t* Existing = ogt_find_entity_class(Class);

	if (Existing)
	{
		printf("Trying to re-register entity class '%s'\n", Class);
		return;
	}

	EntityClass_t* EntityClass = (EntityClass_t*)malloc(sizeof(EntityClass_t));

	if (!EntityClass)
	{
		printf("Failed to allocate for entity class '%s'\n", Class);
		return;
	}

	EntityClass->Name = Class;
	EntityClass->OnCreation = *OnCreation;
	EntityClass->OnDeletion = *OnDeletion;
	EntityClass->Think = *Think;
	EntityClass->Render = *Render;

	hashmap_set(EntityManager.EntityClassMap, Class, strlen(Class), (uintptr_t)EntityClass);
}

EntityClass_t* ogt_find_entity_class(const char* Class)
{
	uintptr_t Existing;

	if (hashmap_get(EntityManager.EntityClassMap, Class, strlen(Class), &Existing))
		return (EntityClass_t*)Existing;

	return NULL;
}

Entity_t* ogt_create_entity(const char* Class)
{
	EntityClass_t* EntityClass = ogt_find_entity_class(Class);

	if (!EntityClass)
	{
		printf("Trying create entity of non-existent class '%s'\n", Class);

		return NULL;
	}

	unsigned int EntityIndex;

	if (EntityManager.FreeIndexCount > 0)
		EntityIndex = EntityManager.FreeEntIndices[--EntityManager.FreeIndexCount];
	else
	{
		if (EntityManager.EntIndex > MAX_ENTITIES)
		{
			printf("Too many entites! %d\n", EntityManager.EntIndex);

			return NULL;
		}

		EntityIndex = EntityManager.EntIndex++;
	}

	Entity_t* Entity = (Entity_t*)malloc(sizeof(Entity_t));

	if (!Entity)
	{
		printf("Failed to allocate for entity of class '%s'\n", Class);

		return NULL;
	}

	Entity->Valid = 1;
	Entity->ClassName = Class;
	Entity->Index = EntityIndex;

	Entity->OnCreation = EntityClass->OnCreation;
	Entity->OnDeletion = EntityClass->OnDeletion;
	Entity->Think = EntityClass->Think;
	Entity->Render = EntityClass->Render;

	EntityManager.Entities[EntityIndex] = Entity;

	if (Entity->OnCreation)
		Entity->OnCreation(Entity);

	return Entity;
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
