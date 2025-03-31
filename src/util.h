#ifndef cthing_util
#define cthing_util

#include <stdio.h>
#include <stdlib.h>

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

#endif
