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
#include "util.h"

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

	Entity->Body = dBodyCreate(GlobalVars->PhysicsManager->World);
	dBodySetPosition(Entity->Body, Entity->Origin[0], Entity->Origin[1], Entity->Origin[2]);
	dBodySetLinearVel(Entity->Body, 0, 0, 0);
	dBodySetAngularVel(Entity->Body, 0, 0, 0);

	dMass Mass;
	dMassSetBox(&Mass, 1.0, 1.0, 1.0, 1.0);
	dBodySetMass(Entity->Body, &Mass);

	Entity->Geometry = dCreateBox(GlobalVars->PhysicsManager->Space, 1.0, 1.0, 1.0);
	dGeomSetBody(Entity->Geometry, Entity->Body);

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

		if (Entity->Valid && Entity->Body)
		{
			const dReal* Pos = dBodyGetPosition(Entity->Body);
			glm_vec3_copy((vec3){ Pos[0], Pos[1], Pos[2] }, Entity->Origin);

			const dReal* AngularVelocity = dBodyGetAngularVel(Entity->Body);
			dBodySetAngularVel(Entity->Body, AngularVelocity[0] * 0.98f, AngularVelocity[1] * 0.98f, AngularVelocity[2] * 0.98f); // SLOW THE FUCK DOWN

			const dReal* Rot = dBodyGetQuaternion(Entity->Body);
			versor Q = { Rot[1], Rot[2], Rot[3], Rot[0] };

			vec3 Euler;
			quat_to_euler_deg(Q, Euler);
			glm_vec3_scale(Euler, (float)(180.0 / M_PI), Entity->Angles);

			normalize_angles(Entity->Angles);
		}
	}

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

	ModelInfo_t* ModelInfo = Entity->ModelInfo;

	if (!ModelInfo)
	{
		printf("Tried to render an entity with no model info! %d ('%s')\n", Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	if (!ModelInfo->VAO || !ModelInfo->VBO)
	{
		printf("Tried to render entity with invalid VAO/VBO! %d ('%s')\n", Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	unsigned int ShaderProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&ShaderProgram);

	glUniform3fv(glGetUniformLocation(ShaderProgram, "objectColor"), 1, (float*)Entity->Color);

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

	if (ModelInfo->SubmeshCount > 0)
	{
		for (size_t i = 0; i < ModelInfo->SubmeshCount; ++i)
		{
			Mesh_t* Submesh = &ModelInfo->Submeshes[i];
			Material_t* Material = Submesh->Material;

			if (Material && Material->TextureID)
			{
				glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialAmbient"), 1, Material->AmbientColor);
				glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialDiffuse"), 1, Material->DiffuseColor);
				glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialSpecular"), 1, Material->SpecularColor);
				glUniform1f(glGetUniformLocation(ShaderProgram, "MaterialShininess"), Material->SpecularExponent);
				glUniform1f(glGetUniformLocation(ShaderProgram, "uMaterialAlpha"), Material->Dissolve);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Material->TextureID);
				glUniform1i(glGetUniformLocation(ShaderProgram, "useTexture"), 1);
			}
			else
			{
				glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialAmbient"), 1, (float*)VEC3_ONE);
				glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialDiffuse"), 1, (float*)VEC3_ONE);
				glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialSpecular"), 1, (float*)VEC3_ONE);
				glUniform1f(glGetUniformLocation(ShaderProgram, "MaterialShininess"), 1.f);
				glUniform1f(glGetUniformLocation(ShaderProgram, "uMaterialAlpha"), 1.f);

				glUniform1i(glGetUniformLocation(ShaderProgram, "useTexture"), 0);
			}

			glDrawArrays(GL_TRIANGLES, (GLint)Submesh->Index, (GLsizei)Submesh->VertexCount);
		}
	}
	else
	{
		glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialAmbient"), 1, Entity->Color);
		glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialDiffuse"), 1, Entity->Color);
		glUniform3fv(glGetUniformLocation(ShaderProgram, "MaterialSpecular"), 1, (float*)VEC3_ONE);
		glUniform1f(glGetUniformLocation(ShaderProgram, "MaterialShininess"), 1.f);
		glUniform1f(glGetUniformLocation(ShaderProgram, "uMaterialAlpha"), 1.f);
		glUniform1i(glGetUniformLocation(ShaderProgram, "useTexture"), 0);

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)ModelInfo->VertexCount);
	}
}

void ogt_set_entity_model(Entity_t* Entity, const char* Path)
{
	if (!Entity->Valid)
	{
		printf("Tried to set model '%s' on invalid entity!\n", Path);
		return;
	}

	ModelInfo_t* ModelInfo = ogt_get_model_info(Path);

	if (!ModelInfo)
	{
		printf("Failed to set model '%s' for %d ('%s')\n", Path, Entity->Index, Entity->ClassInfo->Name);
		return;
	}

	Entity->ModelInfo = ModelInfo;
}
