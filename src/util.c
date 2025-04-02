#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>

void read_file(const char* path, char** data, size_t* length)
{
	*data = NULL;
	*length = 0;

	FILE* file = fopen(path, "rb");

	if (!file)
		return;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	if (size <= 0)
	{
		fclose(file);

		return;
	}

	char* buffer = (char*)malloc(size + 1);

	if (!buffer)
	{
		fclose(file);

		return;
	}

	size_t bytes = fread(buffer, 1, size, file);

	if (bytes != (size_t)size)
	{
		free(buffer);
		fclose(file);

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

bool load_shader(unsigned int type, unsigned int count, const char* path, Shader_t* shader)
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

void delete_shader(Shader_t* shader)
{
	glDeleteShader(shader->id);
}

void attach_shader(Shader_t* shader, unsigned int program)
{
	glAttachShader(program, shader->id);
	delete_shader(shader);
}

unsigned int create_texture(const char* path)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_set_flip_vertically_on_load(1);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data)
	{
		unsigned int format;

		switch (nrChannels)
		{
			case 1:
				format = GL_RED;
				break;

			default:
			case 3:
				format = GL_RGB;
				break;

			case 4:
				format = GL_RGBA;
				break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		printf("Failed to load texture at '%s'\n", path);

	stbi_image_free(data);

	return texture;
}

static void get_file_data(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len)
{
	(void)ctx;

	printf("Getting file '%s'\n", filename);

	read_file(filename, data, len);
}

float* load_obj(const char* Path, size_t* VertexCount, size_t* MeshCount, size_t* MaterialCount, Material_t** Materials)
{
	*VertexCount = 0;
	*MeshCount = 0;
	*MaterialCount = 0;
	*Materials = NULL;

	tinyobj_attrib_t attrib;
	tinyobj_shape_t* shapes;
	size_t num_shapes;
	tinyobj_material_t* materials;
	size_t num_materials;

	int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, Path, get_file_data, NULL, TINYOBJ_FLAG_TRIANGULATE);

	if (ret != TINYOBJ_SUCCESS)
	{
		printf("Failed to read file '%s'\n", Path);

		return NULL;
	}

	size_t totalVertices = attrib.num_face_num_verts * 3;

	float* vertices = malloc(totalVertices * OBJ_CHUNK_SIZE);

	if (!vertices)
	{
		printf("Failed to allocate for file '%s'\n", Path);

		tinyobj_attrib_free(&attrib);
		tinyobj_shapes_free(shapes, num_shapes);
		tinyobj_materials_free(materials, num_materials);

		return NULL;
	}

	size_t outOffset = 0;

	for (size_t face = 0; face < attrib.num_face_num_verts; face++)
	{
		for (int vert = 0; vert < 3; ++vert)
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

	Material_t* OutMaterials = NULL;

	if (num_materials > 0)
	{
		OutMaterials = malloc(num_materials * sizeof(Material_t));

		if (OutMaterials == NULL)
		{
			printf("Failed to allocate materials for file '%s'\n", Path);

			free(vertices);

			tinyobj_attrib_free(&attrib);
			tinyobj_shapes_free(shapes, num_shapes);
			tinyobj_materials_free(materials, num_materials);

			return NULL;
		}

		for (size_t i = 0; i < num_materials; ++i)
		{
			if (materials[i].diffuse_texname)
				OutMaterials[i].TexturePath = strdup(materials[i].diffuse_texname);
			else
				OutMaterials[i].TexturePath = NULL;

			OutMaterials[i].Color[0] = materials[i].diffuse[0];
			OutMaterials[i].Color[1] = materials[i].diffuse[1];
			OutMaterials[i].Color[2] = materials[i].diffuse[2];
		}
	}

	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, num_shapes);
	tinyobj_materials_free(materials, num_materials);

	*VertexCount = totalVertices;
	*MeshCount = 1;
	*MaterialCount = num_materials;
	*Materials = OutMaterials;

	return vertices;
}
