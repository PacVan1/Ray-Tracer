#include "precomp.h"
#include "files.h"

#include "../lib/stb_image.h"

FILE* openForRead(char const* path)
{
	FILE* file = fopen(path, "r"); 
	if (!file) FatalError("Couldn't open file: %s", path); 
	return file; 
}

void printLoading(char const* path)
{
	printf("[LOADING]\t%s\n", path);
}

void printSuccess(char const* path)
{
	printf("[SUCCESS]\t%s\n", path);
}

void printFailure(char const* path)
{
	printf("[FAILURE]\t%s\n", path); 
}

void fileNotFound(char const* path) 
{
	FatalError("File not found: %s", path); 
}

Texture<float3> loadTexture(char const* path) 
{
	printLoading(path); 
	if (!FileExists(path)) fileNotFound(path);  
	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;
	float* data = stbi_loadf(path, &width, &height, &channels, 0);
	Texture<float3> texture = Texture<float3>(width, height);
	texture.mWidth = width; texture.mHeight = height;
	memcpy(texture.mData, data, sizeof(float) * width * height * 3); 
	stbi_image_free(data);
	printSuccess(path);  
	return texture;
}
