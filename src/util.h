#ifndef cthing_util
#define cthing_util

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>

#define OBJ_CHUNK_SIZE (8 * sizeof(float)) // 3 pos, 3 normal, 2 tex = 8

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

static void get_file_data(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len)
{
	(void)ctx;

	printf("Getting file '%s'\n", filename);

	read_file(filename, data, len);
}

float* load_obj(const char* path, size_t* vertexCount)
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t* shapes;
	size_t num_shapes;
	tinyobj_material_t* materials;
	size_t num_materials;

	int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, path, get_file_data, NULL, TINYOBJ_FLAG_TRIANGULATE);

	if (ret != TINYOBJ_SUCCESS)
	{
		printf("Failed to read file '%s'\n", path);

		*vertexCount = 0;

		return NULL;
	}

	// printf("vertices: %zu\n", attrib.num_vertices);
	// printf("normals: %zu\n", attrib.num_normals);
	// printf("texcoords: %zu\n", attrib.num_texcoords);
	// printf("faces: %zu\n", attrib.num_faces);
	// printf("face vert counts: %zu\n", attrib.num_face_num_verts);
	// printf("shapes: %zu\n", num_shapes);

	size_t totalVertices = attrib.num_face_num_verts * 3;

	float* vertices = malloc(totalVertices * OBJ_CHUNK_SIZE);

	if (!vertices)
	{
		printf("Failed to allocate for file '%s'\n", path);

		tinyobj_attrib_free(&attrib);
		tinyobj_shapes_free(shapes, num_shapes);
		tinyobj_materials_free(materials, num_materials);

		*vertexCount = 0;

		return NULL;
	}

	size_t outOffset = 0;

	for (size_t face = 0; face < attrib.num_face_num_verts; face++)
	{
		for (int vert = 0; vert < 3; vert++)
		{
			tinyobj_vertex_index_t idx = attrib.faces[face * 3 + vert];

			// pos
			vertices[outOffset++] = attrib.vertices[3 * idx.v_idx + 0];
			vertices[outOffset++] = attrib.vertices[3 * idx.v_idx + 1];
			vertices[outOffset++] = attrib.vertices[3 * idx.v_idx + 2];

			// normal
			vertices[outOffset++] = attrib.normals[3 * idx.vn_idx + 0];
			vertices[outOffset++] = attrib.normals[3 * idx.vn_idx + 1];
			vertices[outOffset++] = attrib.normals[3 * idx.vn_idx + 2];

			// tex
			vertices[outOffset++] = attrib.texcoords[2 * idx.vt_idx + 0];
			vertices[outOffset++] = attrib.texcoords[2 * idx.vt_idx + 1];
		}
	}

	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, num_shapes);
	tinyobj_materials_free(materials, num_materials);

	*vertexCount = totalVertices;

	return vertices;
}

#endif
