#include "models.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>

#include "globals.h"
#include "util.h"

static void get_file_data(void* _, const char* Path, const int IsMaterial, const char* OBJPath, char** Data, size_t* Length)
{
	read_file(Path, Data, Length);
}

void load_obj(ModelInfo_t* ModelInfo)
{
	ModelInfo->Vertices = NULL;
	ModelInfo->VertexCount = 0;
	ModelInfo->MeshCount = 0;
	ModelInfo->MaterialCount = 0;
	ModelInfo->Materials = NULL;
	ModelInfo->Submeshes = NULL;
	ModelInfo->SubmeshCount = 0;

	tinyobj_attrib_t Attributes;
	tinyobj_shape_t* Shapes;
	size_t ShapeCount;
	tinyobj_material_t* Materials;
	size_t MaterialCount;

	if (tinyobj_parse_obj(&Attributes, &Shapes, &ShapeCount, &Materials, &MaterialCount, ModelInfo->ModelPath, get_file_data, NULL, TINYOBJ_FLAG_TRIANGULATE) != TINYOBJ_SUCCESS)
	{
		printf("Failed to read file '%s'\n", ModelInfo->ModelPath);
		return;
	}

	size_t TotalVertices = Attributes.num_face_num_verts * 3;
	float* Vertices = malloc(TotalVertices * OBJ_CHUNK_SIZE);

	if (!Vertices)
	{
		printf("Failed to allocate for file '%s'\n", ModelInfo->ModelPath);

		tinyobj_attrib_free(&Attributes);
		tinyobj_shapes_free(Shapes, ShapeCount);
		tinyobj_materials_free(Materials, MaterialCount);

		return;
	}

	size_t Offset = 0;

	for (size_t Face = 0; Face < Attributes.num_face_num_verts; Face++)
	{
		int MaterialID = Attributes.material_ids[Face];
		vec3 MaterialColor = { 1.f, 1.f, 1.f };

		if (MaterialID >= 0 && MaterialID < (int)MaterialCount)
			glm_vec3_copy(Materials[MaterialID].diffuse, MaterialColor);

		for (int Vert = 0; Vert < 3; ++Vert)
		{
			tinyobj_vertex_index_t Index = Attributes.faces[(Face * 3) + Vert];

			// pos
			Vertices[Offset++] = Attributes.vertices[(3 * Index.v_idx) + 0];
			Vertices[Offset++] = Attributes.vertices[(3 * Index.v_idx) + 1];
			Vertices[Offset++] = Attributes.vertices[(3 * Index.v_idx) + 2];

			// normal
			Vertices[Offset++] = Attributes.normals[(3 * Index.vn_idx) + 0];
			Vertices[Offset++] = Attributes.normals[(3 * Index.vn_idx) + 1];
			Vertices[Offset++] = Attributes.normals[(3 * Index.vn_idx) + 2];

			// tex
			Vertices[Offset++] = Attributes.texcoords[(2 * Index.vt_idx) + 0];
			Vertices[Offset++] = Attributes.texcoords[(2 * Index.vt_idx) + 1];

			// material color
			Vertices[Offset++] = MaterialColor[0];
			Vertices[Offset++] = MaterialColor[1];
			Vertices[Offset++] = MaterialColor[2];
		}
	}

	Material_t* OutMaterials = NULL;

	if (MaterialCount > 0)
	{
		OutMaterials = malloc(MaterialCount * sizeof(Material_t));

		if (!OutMaterials)
		{
			printf("Failed to allocate materials for file '%s'\n", ModelInfo->ModelPath);

			free(Vertices);

			tinyobj_attrib_free(&Attributes);
			tinyobj_shapes_free(Shapes, ShapeCount);
			tinyobj_materials_free(Materials, MaterialCount);

			return;
		}

		memset(OutMaterials, 0, MaterialCount * sizeof(Material_t));

		for (size_t i = 0; i < MaterialCount; ++i)
		{
			tinyobj_material_t* In = &Materials[i];
			Material_t* Out = &OutMaterials[i];

			if (In->diffuse_texname)
				Out->TexturePath = strdup(In->diffuse_texname);

			glm_vec3_copy(In->ambient, Out->AmbientColor);
			glm_vec3_copy(In->diffuse, Out->DiffuseColor);
			glm_vec3_copy(In->specular, Out->SpecularColor);

			Out->SpecularExponent = In->shininess;
			Out->Dissolve = In->dissolve;
			Out->Illumination = In->illum;
		}
	}

	size_t* FaceCounts = calloc(MaterialCount, sizeof(size_t));

	for (size_t Face = 0; Face < Attributes.num_face_num_verts; ++Face)
	{
		int MaterialID = Attributes.material_ids[Face];

		if (MaterialID >= 0 && MaterialID < (int)MaterialCount)
			FaceCounts[MaterialID]++;
	}

	size_t SubmeshCount = 0;

	for (size_t i = 0; i < MaterialCount; ++i)
		if (FaceCounts[i] > 0)
			SubmeshCount++;

	Mesh_t* Submeshes = NULL;

	if (SubmeshCount > 0)
	{
		Submeshes = malloc(SubmeshCount * sizeof(Mesh_t));

		if (!Submeshes)
		{
			printf("Failed to allocate submeshes for file '%s'\n", ModelInfo->ModelPath);

			free(Vertices);
			free(OutMaterials);
			free(FaceCounts);

			tinyobj_attrib_free(&Attributes);
			tinyobj_shapes_free(Shapes, ShapeCount);
			tinyobj_materials_free(Materials, MaterialCount);

			return;
		}

		size_t Offset = 0;

		for (size_t i = 0; i < MaterialCount; ++i)
		{
			if (FaceCounts[i] == 0)
				continue;

			Submeshes[ModelInfo->SubmeshCount].Index = Offset * 3;
			Submeshes[ModelInfo->SubmeshCount].VertexCount = FaceCounts[i] * 3;
			Submeshes[ModelInfo->SubmeshCount].Material = &OutMaterials[i];

			Offset += FaceCounts[i];
			ModelInfo->SubmeshCount++;
		}
	}

	free(FaceCounts);

	tinyobj_attrib_free(&Attributes);
	tinyobj_shapes_free(Shapes, ShapeCount);
	tinyobj_materials_free(Materials, MaterialCount);

	ModelInfo->Vertices = Vertices;
	ModelInfo->VertexCount = TotalVertices;
	ModelInfo->MeshCount = 1;
	ModelInfo->MaterialCount = MaterialCount;
	ModelInfo->Materials = OutMaterials;
	ModelInfo->Submeshes = Submeshes;
}

ModelInfo_t* ogt_get_model_info(const char* Path)
{
	uintptr_t Existing;

	if (hashmap_get(GlobalVars->EntityManager->EntityModelMap, Path, strlen(Path), &Existing))
		return (ModelInfo_t*)Existing;

		ModelInfo_t* ModelInfo = (ModelInfo_t*)malloc(sizeof(ModelInfo_t));

	if (!ModelInfo)
	{
		printf("Failed to allocate model info for '%s'\n", Path);

		return NULL;
	}

	ModelInfo->ModelPath = Path;
	ModelInfo->VAO = 0;
	ModelInfo->VBO = 0;

	load_obj(ModelInfo);

	if (ModelInfo->VertexCount <= 0)
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
		"Loaded Model for '%s' - Vertices: %d Size: %d Meshes: %d Materials: %d Submeshes: %d\n",

		Path,
		ModelInfo->VertexCount,
		ModelInfo->VertexCount * OBJ_CHUNK_SIZE,
		ModelInfo->MeshCount,
		ModelInfo->MaterialCount,
		ModelInfo->SubmeshCount
	);

	return ModelInfo;
}
