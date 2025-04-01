#ifndef cthing_util
#define cthing_util

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

void read_file(const char* path, char** data, size_t* length)
{
	FILE* file = fopen(path, "rb");

	if (!file)
	{
		*data = NULL;
		*length = 0;

		return;
	}

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	if (size <= 0)
	{
		fclose(file);

		*data = NULL;
		*length = 0;

		return;
	}

	char* buffer = (char*)malloc(size + 1);

	if (!buffer)
	{
		fclose(file);

		*data = NULL;
		*length = 0;

		return;
	}

	size_t bytes = fread(buffer, 1, size, file);

	if (bytes != (size_t)size)
	{
		free(buffer);
		fclose(file);

		*data = NULL;
		*length = 0;

		return;
	}

	buffer[bytes] = '\0';
	fclose(file);

	*data = buffer;
	*length = size;
}

char* load_shader_code(const char* path)
{
	char* code;
	size_t length;

	read_file(path, &code, &length);

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

int load_obj(const char* path, float** vertices)
{
	FILE* file = fopen(path, "r");

	if (!file)
	{
		*vertices = NULL;

		return 0;
	}

	size_t vertexCount = 128;
	float* buffer = malloc(vertexCount * 3 * sizeof(float));

	if (!buffer)
	{
		fclose(file);

		*vertices = NULL;

		return 0;
	}

	char line[128]; // pls no big lines

	int vertex = 0;

	while (fgets(line, sizeof(line), file))
	{
		if (line[0] != 'v') continue;

		float x, y, z;

		if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3)
		{
			if (vertex >= vertexCount)
			{
				vertexCount *= 2;

				float* expanded = realloc(buffer, vertexCount * 3 * sizeof(float));

				if (!expanded) // downloadmoreram.com
				{
					free(buffer);
					fclose(file);

					*vertices = NULL;

					return vertex;
				}

				buffer = expanded;
			}

			buffer[vertex * 3] = x;
			buffer[vertex * 3 + 1] = y;
			buffer[vertex * 3 + 2] = z;

			vertex++;
		}
	}

	fclose(file);

	float* trimmed = realloc(buffer, vertexCount * 3 * sizeof(float));

	if (trimmed)
		buffer = trimmed;

	*vertices = buffer;

	return vertex;
}

#endif
