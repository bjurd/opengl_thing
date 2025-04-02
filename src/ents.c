#include "ents.h"

#include <stdio.h>

void ogt_create_entity(Entity_t* Entity)
{
	if (EntityManager.EntIndex > MAX_ENTITIES)
	{
		printf("Too many entites! %d\n", EntityManager.EntIndex);

		Entity->Valid = 0;

		return;
	}

	Entity->Valid = 1;
	Entity->ID = EntityManager.EntIndex;

	Entity->Health = 0;

	Entity->Think = NULL;
	Entity->Render = NULL;

	EntityManager.Entities[EntityManager.EntIndex] = Entity;
	EntityManager.EntIndex++;
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
