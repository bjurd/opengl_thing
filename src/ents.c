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
	Manager->EntityModelMap = hashmap_create();

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
	Entity->ModelInfo = NULL;

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

void ogt_render_entities(float DeltaTime, unsigned int ShaderProgram)
{
	for (unsigned int i = 0; i < EntityManager.EntIndex; ++i)
	{
		Entity_t* Entity = EntityManager.Entities[i];

		if (Entity->Valid && Entity->Render)
			Entity->Render(Entity, DeltaTime, ShaderProgram);
	}
}

void ogt_render_entity_basic(Entity_t* Entity, float DeltaTime, unsigned int ShaderProgram)
{
	if (!Entity->Valid)
	{
		printf("Tried to render invalid entity!\n");
		return;
	}

	EntityModelInfo_t* ModelInfo = Entity->ModelInfo;

	if (!ModelInfo)
	{
		printf("Tried to render an entity with no model info! %d ('%s')\n", Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	if (!ModelInfo->VAO)
	{
		printf("Tried to render entity with invalid VAO! %d ('%s')\n", Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	if (!ModelInfo->VBO)
	{
		printf("Tried to render entity with invalid VBO! %d ('%s')\n", Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	unsigned int useTextureLoc = glGetUniformLocation(ShaderProgram, "useTexture");

	glBindVertexArray(ModelInfo->VAO);

	if (useTextureLoc)
	{
		if (ModelInfo->MaterialCount > 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ModelInfo->Materials[0].TextureID);

			glUniform1i(useTextureLoc, 1);
		}
		else
			glUniform1i(useTextureLoc, 0);
	}

	glDrawArrays(GL_TRIANGLES, 0, ModelInfo->VertexCount);
}

EntityModelInfo_t* ogt_get_model_info(const char* Path)
{
	uintptr_t Existing;

	if (hashmap_get(EntityManager.EntityModelMap, Path, strlen(Path), &Existing))
		return (EntityModelInfo_t*)Existing;

	EntityModelInfo_t* ModelInfo = (EntityModelInfo_t*)malloc(sizeof(EntityModelInfo_t));

	if (!ModelInfo)
	{
		printf("Failed to allocate model info for '%s'\n", Path);

		return NULL;
	}

	ModelInfo->ModelPath = Path;
	ModelInfo->VAO = 0;
	ModelInfo->VBO = 0;

	ModelInfo->Vertices = load_obj(ModelInfo->ModelPath, &ModelInfo->VertexCount, &ModelInfo->MeshCount, &ModelInfo->MaterialCount, &ModelInfo->Materials);

	if (!ModelInfo->Vertices)
	{
		printf("Failed to load model for '%s'\n", Path);

		return NULL;
	}

	for (size_t i = 0; i < ModelInfo->MaterialCount; ++i)
	{
		Material_t* Material = &ModelInfo->Materials[i];

		if (Material->TexturePath)
			Material->TextureID = create_texture(Material->TexturePath);
	}

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

	hashmap_set(EntityManager.EntityModelMap, Path, strlen(Path), (uintptr_t)ModelInfo);

	printf(
		"Loaded Model for '%s' - Vertices: %d Size: %d Meshes: %d Materials: %d\n",

		Path,
		ModelInfo->VertexCount,
		ModelInfo->VertexCount * OBJ_CHUNK_SIZE,
		ModelInfo->MeshCount,
		ModelInfo->MaterialCount
	);

	return ModelInfo;
}

void ogt_set_entity_model(Entity_t* Entity, const char* Path)
{
	if (!Entity->Valid)
	{
		printf("Tried to set model '%s' on invalid entity!\n", Path);
		return;
	}

	EntityModelInfo_t* ModelInfo = ogt_get_model_info(Path);

	if (!ModelInfo)
	{
		printf("Failed to set model '%s' for %d ('%s')\n", Path, Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	Entity->ModelInfo = ModelInfo;
}
