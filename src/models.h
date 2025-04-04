#ifndef ogt_models
#define ogt_models

#include <cglm/types.h>
#include <stddef.h>

#define OBJ_CHUNK_SIZE (11 * sizeof(float)) // 3 pos, 3 normal, 2 tex, 3 material color

typedef struct
{
	unsigned int TextureID;
	char* TexturePath;
	vec3 AmbientColor;
	vec3 DiffuseColor;
	vec3 SpecularColor;
	float SpecularExponent;
	float Dissolve;
	int Illumination;
} Material_t;

typedef struct
{
	size_t Index;
	size_t VertexCount;
	Material_t* Material;
} Mesh_t;

typedef struct
{
	unsigned int VAO;
	unsigned int VBO;

	const char* ModelPath;
	size_t VertexCount;
	size_t MeshCount;
	size_t MaterialCount;
	Material_t* Materials;
	float* Vertices;

	Mesh_t* Submeshes;
	size_t SubmeshCount;
} ModelInfo_t;

void load_obj(ModelInfo_t* ModelInfo);
ModelInfo_t* ogt_get_model_info(const char* Path);

#endif
