#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

void normalize_angle(float* Angle)
{
	*Angle = fmodf(*Angle + 180.f, 360.f) - 180.f;
}

void normalize_angles(vec3 Angles)
{
	normalize_angle(&Angles[0]);
	normalize_angle(&Angles[1]);
	normalize_angle(&Angles[2]);
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

	normalize_angle(Yaw);
	normalize_angle(Pitch);
}

void angles_to_vec3(float Yaw, float Pitch, vec3 Forward, vec3 Right, vec3 Up)
{
	normalize_angle(&Yaw);
	normalize_angle(&Pitch);

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

void quat_to_euler_deg(const versor Q, vec3 Angles) // Found this online turns a quaternion into euler angles via math I don't understand :^)
{
	float x = Q[0], y = Q[1], z = Q[2], w = Q[3];

	float sinr_cosp = 2.f * (w * x + y * z);
	float cosr_cosp = 1.f - 2.f * (x * x + y * y);
	float roll = atan2f(sinr_cosp, cosr_cosp);

	float sinp = 2.f * (w * y - z * x);
	float pitch;

	if (fabsf(sinp) >= 1.f)
		pitch = copysignf(GLM_PI_2f, sinp);
	else
		pitch = asinf(sinp);

	float siny_cosp = 2.f * (w * z + x * y);
	float cosy_cosp = 1.f - 2.f * (y * y + z * z);
	float yaw = atan2f(siny_cosp, cosy_cosp);

	Angles[0] = glm_deg(pitch);
	Angles[1] = glm_deg(yaw);
	Angles[2] = glm_deg(roll);

	normalize_angles(Angles);
}
