#include "ents.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hashmap/map.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void ogt_init_entity_system(EntityManager_t* Manager)
{
	*Manager = (EntityManager_t){ 0 };

	Manager->EntIndex = 0;
	Manager->FreeIndexCount = 0;
	Manager->EntityClassMap = hashmap_create();

	for (unsigned int i = 0; i < MAX_ENTITIES; ++i)
		Manager->FreeEntIndices[i] = i;
}

EntityClass_t* ogt_register_entity_class(const char* Class, CreationFn OnCreation, DeletionFn OnDeletion, ThinkFn Think, RenderFn Render)
{
	EntityClass_t* Existing = ogt_find_entity_class(Class);

	if (Existing)
	{
		printf("Trying to re-register entity class '%s'\n", Class);

		return NULL;
	}

	EntityClass_t* EntityClass = (EntityClass_t*)malloc(sizeof(EntityClass_t));

	if (!EntityClass)
	{
		printf("Failed to allocate for entity class '%s'\n", Class);

		return NULL;
	}

	EntityClass->Name = Class;

	EntityClass->ModelInfo = NULL;

	EntityClass->OnCreation = *OnCreation;
	EntityClass->OnDeletion = *OnDeletion;
	EntityClass->Think = *Think;
	EntityClass->Render = *Render;

	hashmap_set(EntityManager.EntityClassMap, Class, strlen(Class), (uintptr_t)EntityClass);

	return EntityClass;
}

EntityClass_t* ogt_find_entity_class(const char* Class)
{
	uintptr_t Existing;

	if (hashmap_get(EntityManager.EntityClassMap, Class, strlen(Class), &Existing))
		return (EntityClass_t*)Existing;

	return NULL;
}

Entity_t* ogt_create_entity_ex(EntityClass_t* EntityClass)
{
	unsigned int EntityIndex;

	if (EntityManager.FreeIndexCount > 0)
		EntityIndex = EntityManager.FreeEntIndices[--EntityManager.FreeIndexCount];
	else
	{
		if (EntityManager.EntIndex >= MAX_ENTITIES)
		{
			printf("Too many entities! %d\n", EntityManager.EntIndex);
			return NULL;
		}

		EntityIndex = EntityManager.EntIndex++;
	}

	Entity_t* Entity = (Entity_t*)malloc(sizeof(Entity_t));

	if (!Entity)
	{
		printf("Failed to allocate for entity of class '%s'\n", EntityClass->Name);

		return NULL;
	}

	Entity->Valid = 1;
	Entity->Index = EntityIndex;

	Entity->ClassInfo = EntityClass;

	memset(&Entity->Origin, 0, sizeof(vec3));
	memset(&Entity->Angles, 0, sizeof(vec3));

	Entity->OnCreation = EntityClass->OnCreation;
	Entity->OnDeletion = EntityClass->OnDeletion;
	Entity->Think = EntityClass->Think;
	Entity->Render = EntityClass->Render;

	EntityManager.Entities[EntityIndex] = Entity;

	if (Entity->OnCreation)
		Entity->OnCreation(Entity);

	return Entity;
}

Entity_t* ogt_create_entity(const char* Class)
{
	EntityClass_t* EntityClass = ogt_find_entity_class(Class);

	if (!EntityClass)
	{
		printf("Trying create entity of non-existent class '%s'\n", Class);

		return NULL;
	}

	return ogt_create_entity_ex(EntityClass);
}

void ogt_delete_entity(Entity_t* Entity)
{
	if (Entity->Valid)
	{
		if (Entity->OnDeletion)
			Entity->OnDeletion(Entity);

		unsigned int EntityIndex = Entity->Index;

		Entity->Valid = 0;
		Entity->Index = 0;
		Entity->ClassInfo = NULL;

		EntityManager.Entities[EntityIndex] = NULL;
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

void ogt_render_entity_basic(Entity_t* Entity, float DeltaTime)
{
	if (!Entity->Valid || !Entity->ClassInfo)
	{
		printf("Tried to render invalid entity!\n");
		return;
	}

	EntityModelInfo_t* ModelInfo = Entity->ClassInfo->ModelInfo;

	if (!ModelInfo)
	{
		printf("Tried to render an entity with no model info! '%s' - %d\n", Entity->ClassInfo->Name, Entity->Index);
		return;
	}

	if (!ModelInfo->VAO)
	{
		printf("Tried to render entity with invalid VAO! '%s' - %d\n", Entity->ClassInfo->Name, Entity->Index);
		return;
	}

	if (!ModelInfo->VBO)
	{
		printf("Tried to render entity with invalid VBO! '%s' - %d\n", Entity->ClassInfo->Name, Entity->Index);
		return;
	}

	glBindVertexArray(ModelInfo->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ModelInfo->Materials[0].TextureID);
	glDrawArrays(GL_TRIANGLES, 0, ModelInfo->VertexCount);
}

void ogt_load_entity_class_model(EntityClass_t* EntityClass)
{
	EntityModelInfo_t* ModelInfo = EntityClass->ModelInfo;

	if (!ModelInfo->ModelPath)
	{
		printf("Trying to laod model for class with no model '%s'\n", EntityClass->Name);
		return;
	}

	ModelInfo->Vertices = load_obj(ModelInfo->ModelPath, &ModelInfo->VertexCount, &ModelInfo->MeshCount, &ModelInfo->MaterialCount, &ModelInfo->Materials);

	if (!ModelInfo->Vertices)
	{
		printf("Failed to load model for class '%s'\n", EntityClass->Name);
		return;
	}

	for (size_t i = 0; i < ModelInfo->MaterialCount; ++i)
	{
		Material_t* Material = &ModelInfo->Materials[i];

		if (Material->TexturePath)
			Material->TextureID = create_texture(Material->TexturePath);
	}

	printf(
		"Loaded Model for '%s' - Vertices: %d Size: %d Meshes: %d Materials: %d\n",

		EntityClass->Name,
		ModelInfo->VertexCount,
		ModelInfo->VertexCount * OBJ_CHUNK_SIZE,
		ModelInfo->MeshCount,
		ModelInfo->MaterialCount
	);
}

void ogt_setup_entity_class_model(EntityClass_t* EntityClass, const char* ModelPath)
{
	EntityModelInfo_t* ModelInfo = EntityClass->ModelInfo;

	if (ModelInfo) // Assume this was already done
	{
		printf("Tried to re-setup entity class model '%s'\n", EntityClass->Name);
		return;
	}
	else
	{
		ModelInfo = (EntityModelInfo_t*)malloc(sizeof(EntityModelInfo_t));

		if (!ModelInfo)
		{
			printf("Failed to allocate model info for class '%s'\n", EntityClass->Name);
			return;
		}

		ModelInfo->ModelPath = ModelPath;
		ModelInfo->VAO = 0;
		ModelInfo->VBO = 0;
		EntityClass->ModelInfo = ModelInfo;
	}

	ogt_load_entity_class_model(EntityClass);

	glGenVertexArrays(1, &ModelInfo->VAO);
	glGenBuffers(1, &ModelInfo->VBO);

	glBindVertexArray(ModelInfo->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, ModelInfo->VBO);
	glBufferData(GL_ARRAY_BUFFER, ModelInfo->VertexCount * OBJ_CHUNK_SIZE, ModelInfo->Vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, OBJ_CHUNK_SIZE, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, OBJ_CHUNK_SIZE, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, OBJ_CHUNK_SIZE, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	printf("VAO: %d VBO: %d\n", ModelInfo->VAO, ModelInfo->VBO);
}
