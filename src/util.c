#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>

#include "globals.h"

void read_file(const char* Path, char** Data, size_t* Length)
{
	*Data = NULL;
	*Length = 0;

	FILE* File = fopen(Path, "rb");

	if (!File)
		return;

	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	rewind(File);

	if (Size <= 0)
	{
		fclose(File);

		return;
	}

	char* Buffer = (char*)malloc(Size + 1);

	if (!Buffer)
	{
		fclose(File);

		return;
	}

	size_t Bytes = fread(Buffer, 1, Size, File);

	if (Bytes != (size_t)Size)
	{
		free(Buffer);
		fclose(File);

		return;
	}

	Buffer[Bytes] = '\0';
	fclose(File);

	*Data = Buffer;
	*Length = Size;
}

char* load_shader_code(const char* Path)
{
	char* Code;
	size_t Length;

	read_file(Path, &Code, &Length);

	return Code;
}

bool load_shader(unsigned int Type, unsigned int Count, const char* Path, Shader_t* Shader)
{
	Shader->Type = Type;
	Shader->Count = Count;
	Shader->ID = glCreateShader(Type);

	char* Code = load_shader_code(Path);

	if (!Code)
		return 0;

	glShaderSource(Shader->ID, Shader->Count, (const char**)&Code, NULL);
	glCompileShader(Shader->ID);

	free(Code);

	return 1;
}

void delete_shader(Shader_t* Shader)
{
	glDeleteShader(Shader->ID);
}

void attach_shader(Shader_t* Shader, unsigned int ShaderProgram)
{
	glAttachShader(ShaderProgram, Shader->ID);
	delete_shader(Shader);
}

unsigned int create_texture(const char* Path)
{
	unsigned int Texture;
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_set_flip_vertically_on_load(1);

	int Width, Height, Channels;
	unsigned char* Data = stbi_load(Path, &Width, &Height, &Channels, 0);

	if (Data)
	{
		unsigned int Format;

		switch (Channels)
		{
			case 1:
				Format = GL_RED;
				break;

			default:
			case 3:
				Format = GL_RGB;
				break;

			case 4:
				Format = GL_RGBA;
				break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		printf("Failed to load texture at '%s'\n", Path);

	stbi_image_free(Data);

	return Texture;
}

static void get_file_data(void* _, const char* Path, const int IsMaterial, const char* OBJPath, char** Data, size_t* Length)
{
	read_file(Path, Data, Length);
}

float* load_obj(const char* Path, size_t* VertexCount, size_t* MeshCount, size_t* MaterialCountOut, Material_t** MaterialsOut)
{
	*VertexCount = 0;
	*MeshCount = 0;
	*MaterialCountOut = 0;
	*MaterialsOut = NULL;

	tinyobj_attrib_t Attributes;
	tinyobj_shape_t* Shapes;
	size_t ShapeCount;
	tinyobj_material_t* Materials;
	size_t MaterialCount;

	if (tinyobj_parse_obj(&Attributes, &Shapes, &ShapeCount, &Materials, &MaterialCount, Path, get_file_data, NULL, TINYOBJ_FLAG_TRIANGULATE) != TINYOBJ_SUCCESS)
	{
		printf("Failed to read file '%s'\n", Path);

		return NULL;
	}

	size_t TotalVertices = Attributes.num_face_num_verts * 3;
	float* Vertices = malloc(TotalVertices * OBJ_CHUNK_SIZE);

	if (!Vertices)
	{
		printf("Failed to allocate for file '%s'\n", Path);

		tinyobj_attrib_free(&Attributes);
		tinyobj_shapes_free(Shapes, ShapeCount);
		tinyobj_materials_free(Materials, MaterialCount);

		return NULL;
	}

	size_t OutOffset = 0;

	for (size_t Face = 0; Face < Attributes.num_face_num_verts; Face++)
	{
		int MaterialID = Attributes.material_ids[Face];
		vec3 MaterialColor = { 1.f, 1.f, 1.f };

		if (MaterialID >= 0 && MaterialID < (int)MaterialCount)
			glm_vec3_copy(Materials[MaterialID].diffuse, MaterialColor);

		for (int Vert = 0; Vert < 3; ++Vert)
		{
			tinyobj_vertex_index_t Index = Attributes.faces[(Face * 3) + Vert];

			// pos
			Vertices[OutOffset++] = Attributes.vertices[(3 * Index.v_idx) + 0];
			Vertices[OutOffset++] = Attributes.vertices[(3 * Index.v_idx) + 1];
			Vertices[OutOffset++] = Attributes.vertices[(3 * Index.v_idx) + 2];

			// normal
			float nx = Attributes.normals[(3 * Index.vn_idx) + 0];
			float ny = Attributes.normals[(3 * Index.vn_idx) + 1];
			float nz = Attributes.normals[(3 * Index.vn_idx) + 2];
			Vertices[OutOffset++] = nx;
			Vertices[OutOffset++] = ny;
			Vertices[OutOffset++] = nz;

			// tex
			Vertices[OutOffset++] = Attributes.texcoords[(2 * Index.vt_idx) + 0];
			Vertices[OutOffset++] = Attributes.texcoords[(2 * Index.vt_idx) + 1];

			// material color
			Vertices[OutOffset++] = MaterialColor[0];
			Vertices[OutOffset++] = MaterialColor[1];
			Vertices[OutOffset++] = MaterialColor[2];
		}
	}

	Material_t* OutMaterials = NULL;

	if (MaterialCount > 0)
	{
		OutMaterials = malloc(MaterialCount * sizeof(Material_t));
		memset(OutMaterials, 0, MaterialCount * sizeof(Material_t));

		if (OutMaterials == NULL)
		{
			printf("Failed to allocate materials for file '%s'\n", Path);

			free(Vertices);

			tinyobj_attrib_free(&Attributes);
			tinyobj_shapes_free(Shapes, ShapeCount);
			tinyobj_materials_free(Materials, MaterialCount);

			return NULL;
		}

		for (size_t i = 0; i < MaterialCount; ++i)
		{
			if (Materials[i].diffuse_texname)
				OutMaterials[i].TexturePath = strdup(Materials[i].diffuse_texname);
			else
				OutMaterials[i].TexturePath = NULL;

			OutMaterials[i].Color[0] = Materials[i].diffuse[0];
			OutMaterials[i].Color[1] = Materials[i].diffuse[1];
			OutMaterials[i].Color[2] = Materials[i].diffuse[2];
		}
	}

	tinyobj_attrib_free(&Attributes);
	tinyobj_shapes_free(Shapes, ShapeCount);
	tinyobj_materials_free(Materials, MaterialCount);

	*VertexCount = TotalVertices;
	*MeshCount = 1;
	*MaterialCountOut = MaterialCount;
	*MaterialsOut = OutMaterials;

	return Vertices;
}

// https://github.com/lua9520/source-engine-2018-hl2_src/blob/3bf9df6b2785fa6d951086978a3e66f49427166a/mathlib/mathlib_base.cpp#L535
void vec3_to_angles(const vec3 Forward, float* Yaw, float* Pitch)
{
	if (Forward[0] == 0.f && Forward[2] == 0.f)
	{
		*Yaw = 0.f;
		*Pitch = (Forward[1] > 0.f) ? 90.f : -90.f;
	}
	else
	{
		*Yaw = glm_deg(atan2f(Forward[0], -Forward[2]));
		if (*Yaw < 0.f) *Yaw += 360.f;

		*Pitch = glm_deg(asinf(Forward[1]));
	}
}

void angles_to_vec3(float Yaw, float Pitch, vec3 Forward, vec3 Right, vec3 Up)
{
	Yaw = glm_rad(Yaw);
	Pitch = glm_rad(Pitch);

	Forward[0] = sinf(Yaw) * cosf(Pitch);
	Forward[1] = sinf(Pitch);
	Forward[2] = -cosf(Yaw) * cosf(Pitch);

	glm_vec3_cross(Forward, VEC3_UP, Right);
	glm_vec3_normalize(Right);

	glm_vec3_cross(Right, Forward, Up);

	glm_vec3_normalize(Forward);
	glm_vec3_normalize(Right);
	glm_vec3_normalize(Up);
}

void vec3_directionals(vec3 Forward, vec3 Right, vec3 Up)
{
	float Yaw, Pitch;
	vec3_to_angles(Forward, &Yaw, &Pitch);
	angles_to_vec3(Yaw, Pitch, Forward, Right, Up);
}
