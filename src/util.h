#ifndef ogt_util
#define ogt_util

#include <stdlib.h>
#include <cglm/types.h>

#define OBJ_CHUNK_SIZE (8 * sizeof(float)) // 3 pos, 3 normal, 2 tex = 8

void read_file(const char* Path, char** Data, size_t* Length);

typedef struct
{
	unsigned int ID;
	unsigned int Count;
	unsigned int Type;
} Shader_t;

typedef struct
{
	unsigned int TextureID;
	char* TexturePath;
	vec3 Color;
} Material_t;

typedef struct
{
	float* Vertices;
	size_t VertexCount;
	int MaterialIndex;
} Mesh_t;

char* load_shader_code(const char* Path);
bool load_shader(unsigned int Type, unsigned int Count, const char* Path, Shader_t* Shader);
void delete_shader(Shader_t* Shader);
void attach_shader(Shader_t* Shader, unsigned int ShaderProgram);

unsigned int create_texture(const char* Path);
static void get_file_data(void* _, const char* Path, const int IsMaterial, const char* OBJPath, char** Data, size_t* Length);
float* load_obj(const char* Path, size_t* VertexCount, size_t* MeshCount, size_t* MaterialCountOut, Material_t** MaterialsOut);

#endif
