#ifndef ogt_util
#define ogt_util

#include <stdlib.h>
#include <cglm/types.h>

void read_file(const char* Path, char** Data, size_t* Length);

typedef struct
{
	unsigned int ID;
	unsigned int Count;
	unsigned int Type;
} Shader_t;

char* load_shader_code(const char* Path);
bool load_shader(unsigned int Type, unsigned int Count, const char* Path, Shader_t* Shader);
void delete_shader(Shader_t* Shader);
void attach_shader(Shader_t* Shader, unsigned int ShaderProgram);

unsigned int create_texture(const char* Path);

void vec3_to_angles(const vec3 Forward, float* Yaw, float* Pitch);
void angles_to_vec3(float Yaw, float Pitch, vec3 Forward, vec3 Right, vec3 Up);
void vec3_directionals(vec3 Forward, vec3 Right, vec3 Up);

#endif
