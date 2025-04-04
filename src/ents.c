#include "ents.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hashmap/map.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/types.h>

#include "globals.h"

void ogt_init_entity_system()
{
	GlobalVars->EntityManager = (EntityManager_t*)malloc(sizeof(EntityManager_t));

	if (!GlobalVars->EntityManager)
	{
		printf("Failed to allocate for entity manager!\n");
		return;
	}

	GlobalVars->EntityManager->EntIndex = 0;
	GlobalVars->EntityManager->FreeIndexCount = 0;
	GlobalVars->EntityManager->EntityClassMap = hashmap_create();
	GlobalVars->EntityManager->EntityModelMap = hashmap_create();

	for (unsigned int i = 0; i < MAX_ENTITIES; ++i)
		GlobalVars->EntityManager->FreeEntIndices[i] = i;
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

	hashmap_set(GlobalVars->EntityManager->EntityClassMap, Class, strlen(Class), (uintptr_t)EntityClass);

	return EntityClass;
}

EntityClass_t* ogt_find_entity_class(const char* Class)
{
	uintptr_t Existing;

	if (hashmap_get(GlobalVars->EntityManager->EntityClassMap, Class, strlen(Class), &Existing))
		return (EntityClass_t*)Existing;

	return NULL;
}

Entity_t* ogt_create_entity_ex(EntityClass_t* EntityClass)
{
	unsigned int EntityIndex;

	if (GlobalVars->EntityManager->FreeIndexCount > 0)
		EntityIndex = GlobalVars->EntityManager->FreeEntIndices[--GlobalVars->EntityManager->FreeIndexCount];
	else
	{
		if (GlobalVars->EntityManager->EntIndex >= MAX_ENTITIES)
		{
			printf("Too many entities! %d\n", GlobalVars->EntityManager->EntIndex);
			return NULL;
		}

		EntityIndex = GlobalVars->EntityManager->EntIndex++;
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
	glm_vec3_one(Entity->Color);

	Entity->OnCreation = EntityClass->OnCreation;
	Entity->OnDeletion = EntityClass->OnDeletion;
	Entity->Think = EntityClass->Think;
	Entity->Render = EntityClass->Render;

	GlobalVars->EntityManager->Entities[EntityIndex] = Entity;

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

		GlobalVars->EntityManager->Entities[EntityIndex] = NULL;
		GlobalVars->EntityManager->FreeEntIndices[GlobalVars->EntityManager->FreeIndexCount++] = EntityIndex;
	}
}

void ogt_think_entities(float DeltaTime)
{
	for (unsigned int i = 0; i < GlobalVars->EntityManager->EntIndex; ++i)
	{
		Entity_t* Entity = GlobalVars->EntityManager->Entities[i];

		if (Entity->Valid && Entity->Think)
			Entity->Think(Entity, DeltaTime);
	}
}

void ogt_render_entities(float DeltaTime)
{
	for (unsigned int i = 0; i < GlobalVars->EntityManager->EntIndex; ++i)
	{
		Entity_t* Entity = GlobalVars->EntityManager->Entities[i];

		if (Entity->Valid && Entity->Render)
			Entity->Render(Entity, DeltaTime);
	}
}

void ogt_render_entity_basic(Entity_t* Entity, float DeltaTime)
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

	unsigned int ShaderProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &ShaderProgram);

	if (ShaderProgram)
	{
		unsigned int ObjectColorLoc = glGetUniformLocation(ShaderProgram, "objectColor");
		unsigned int UseTextureLoc = glGetUniformLocation(ShaderProgram, "useTexture");

		if (ObjectColorLoc)
		{
			glUniform3fv(ObjectColorLoc, 1, (float*)Entity->Color);
		}

		if (UseTextureLoc)
		{
			if (ModelInfo->MaterialCount > 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ModelInfo->Materials[0].TextureID);

				glUniform1i(UseTextureLoc, 1);
			}
			else
				glUniform1i(UseTextureLoc, 0);
		}
	}

	unsigned int ModelLoc = glGetUniformLocation(ShaderProgram, "model");

	if (ModelLoc)
	{
		mat4 Transform;
		glm_mat4_identity(Transform);
		glm_translate(Transform, Entity->Origin);
		glm_rotate(Transform, glm_rad(Entity->Angles[0]), VEC3_RIGHT);
		glm_rotate(Transform, glm_rad(Entity->Angles[1]), VEC3_UP);
		glm_rotate(Transform, glm_rad(Entity->Angles[2]), VEC3_FORWARD);

		glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, (float*)Transform);
	}

	glBindVertexArray(ModelInfo->VAO);
	glDrawArrays(GL_TRIANGLES, 0, ModelInfo->VertexCount);
}

EntityModelInfo_t* ogt_get_model_info(const char* Path)
{
	uintptr_t Existing;

	if (hashmap_get(GlobalVars->EntityManager->EntityModelMap, Path, strlen(Path), &Existing))
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

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, OBJ_CHUNK_SIZE, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	hashmap_set(GlobalVars->EntityManager->EntityModelMap, Path, strlen(Path), (uintptr_t)ModelInfo);

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
