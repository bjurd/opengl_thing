#ifndef cthing_util
#define cthing_util

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

char* load_shader_code(const char* path)
{
	FILE* file = fopen(path, "rb");

	if (!file)
		return NULL;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	if (size <= 0)
	{
		fclose(file);
		return NULL;
	}

	char* code = (char*)malloc(size + 1);

	if (!code)
	{
		fclose(file);
		return NULL;
	}

	size_t bytes = fread(code, 1, size, file);

	if (bytes != (size_t)size)
	{
		free(code);
		fclose(file);
		return NULL;
	}

	code[bytes] = '\0';
	fclose(file);

	return code;
}

typedef struct
{
	unsigned int id;
	unsigned int count;
	unsigned int type;
} shader_t;

bool load_shader(unsigned int type, unsigned int count, const char* path, shader_t* shader)
{
	shader->type = type;
	shader->count = count;
	shader->id = glCreateShader(type);

	char* code = load_shader_code(path);

	if (!code)
		return 0;

	glShaderSource(shader->id, shader->count, (const char**)&code, NULL);
	glCompileShader(shader->id);

	free(code);

	return 1;
}

void delete_shader(shader_t* shader)
{
	glDeleteShader(shader->id);
}

void attach_shader(shader_t* shader, unsigned int program)
{
	glAttachShader(program, shader->id);
	delete_shader(shader);
}

#endif
