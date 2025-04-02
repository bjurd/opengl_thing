#ifndef ogt_util
#define ogt_util

#include <stdlib.h>
#include <cglm/types.h>

#define OBJ_CHUNK_SIZE (8 * sizeof(float)) // 3 pos, 3 normal, 2 tex = 8

void read_file(const char* path, char** data, size_t* length);

typedef struct
{
	unsigned int id;
	unsigned int count;
	unsigned int type;
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

char* load_shader_code(const char* path);
bool load_shader(unsigned int type, unsigned int count, const char* path, Shader_t* shader);
void delete_shader(Shader_t* shader);
void attach_shader(Shader_t* shader, unsigned int program);

unsigned int create_texture(const char* path);
static void get_file_data(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len);
float* load_obj(const char* Path, size_t* VertexCount, size_t* MeshCount, size_t* MaterialCount, Material_t** Materials);

#endif
