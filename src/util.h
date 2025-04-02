#ifndef ogt_util
#define ogt_util

#include <stdlib.h>

#define OBJ_CHUNK_SIZE (8 * sizeof(float)) // 3 pos, 3 normal, 2 tex = 8

void read_file(const char* path, char** data, size_t* length);

char* load_shader_code(const char* path);

typedef struct
{
	unsigned int id;
	unsigned int count;
	unsigned int type;
} shader_t;

bool load_shader(unsigned int type, unsigned int count, const char* path, shader_t* shader);
void delete_shader(shader_t* shader);
void attach_shader(shader_t* shader, unsigned int program);

static void get_file_data(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len);
float* load_obj(const char* path, size_t* vertexCount);

#endif
