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

static int parse_vertex_line(const char* line, float out[3])
{
	return sscanf(line + 2, "%f %f %f", &out[0], &out[1], &out[2]) == 3;
}

static int count_face_triangles(const char* line)
{
	int count = 0;

	for (int i = 2; line[i] != '\0'; i++)
		if (line[i] == ' ')
			count++;

	return (count >= 2) ? (count - 1) : 0;
}

static void process_face_line(const char* line, float* raw, int baseCount, float* out, int* index)
{
	int indices[16];
	int count = 0;

	char lineCopy[256];

	strncpy(lineCopy, line, sizeof(lineCopy));
	lineCopy[sizeof(lineCopy)-1] = '\0';

	char* token;
	char* temp;
	token = strtok_r(lineCopy, " \t\n", &temp);

	while ((token = strtok_r(NULL, " \t\n", &temp)) && count < 16)
	{
		int idx;

		if (sscanf(token, "%d", &idx) == 1)
			indices[count++] = idx;
	}

	// i'm a huge fan
	for (int i = 2; i < count; i++)
	{
		int tri[3] = { indices[0], indices[i - 1], indices[i] };

		for (int j = 0; j < 3; j++)
		{
			int k = tri[j] - 1;

			if (k >= 0 && k < baseCount)
			{
				out[*index * 3] = raw[k * 3];
				out[*index * 3 + 1] = raw[k * 3 + 1];
				out[*index * 3 + 2] = raw[k * 3 + 2];

				(*index)++;
			}
		}
	}
}

int load_obj(const char* path, float** vertices)
{
	FILE* file = fopen(path, "r");

	if (!file)
	{
		*vertices = NULL;

		return 0;
	}

	// get vertices
	int capacity = 128, baseCount = 0;
	float* rawVerts = malloc(capacity * 3 * sizeof(float));

	if (!rawVerts)
	{
		fclose(file);

		*vertices = NULL;

		return 0;
	}

	int vertex = 0;
	int hasFace = 0;
	char line[256];

	while (fgets(line, sizeof(line), file))
	{
		if (strncmp(line, "v ", 2) == 0)
		{
			float temp[3];

			if (parse_vertex_line(line, temp))
			{
				if (baseCount >= capacity)
				{
					capacity *= 2;
					float* newRaw = realloc(rawVerts, capacity * 3 * sizeof(float));

					if (!newRaw)
					{
						free(rawVerts);
						fclose(file);

						*vertices = NULL;

						return 0;
					}

					rawVerts = newRaw;
				}

				rawVerts[baseCount * 3 + 0] = temp[0];
				rawVerts[baseCount * 3 + 1] = temp[1];
				rawVerts[baseCount * 3 + 2] = temp[2];

				baseCount++;
			}
		}
		else if (strncmp(line, "f ", 2) == 0)
		{
			hasFace = 1;

			int triCount = count_face_triangles(line);
			vertex += triCount * 3;
		}
	}

	if (!hasFace)
	{
		fclose(file);

		vertex = baseCount;
		*vertices = rawVerts;

		return vertex;
	}

	float* out = malloc(vertex * 3 * sizeof(float));

	if (!out)
	{
		free(rawVerts);
		fclose(file);

		*vertices = NULL;

		return 0;
	}

	// faces
	rewind(file);

	int outVertex = 0;

	while (fgets(line, sizeof(line), file))
	{
		if (strncmp(line, "f ", 2) == 0)
			process_face_line(line, rawVerts, baseCount, out, &outVertex);
	}

	fclose(file);
	free(rawVerts);

	*vertices = out;

	return outVertex;
}

#endif
