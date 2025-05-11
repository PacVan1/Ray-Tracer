#include "precomp.h"
#include "files.h"

#include <iostream> 
#include "../lib/stb_image.h"

FILE* openForRead(char const* path)
{
	FILE* file = fopen(path, "r"); 
	if (!file) FatalError("Couldn't open file: %s", path); 
	return file; 
}

void printSaving(char const* path)
{
	printf("[SAVING STARTED]\t%s\n", path);
}

void printLoading(char const* path)
{
	printf("[LOADING STARTED]\t%s\n", path);
}

void printSuccess(char const* path)
{
	printf("[LOADING SUCCESS]\t%s\n", path);
}

void printFailure(char const* path)
{
	printf("[LOADING FAILURE]\t%s\n", path); 
}

void fileNotFound(char const* path) 
{
	FatalError("File not found: %s", path); 
}

Texture<float3> loadTextureF(char const* path)  
{
	printLoading(path); 
	if (!FileExists(path)) fileNotFound(path);  
	int width, height, channels;
	float* data = stbi_loadf(path, &width, &height, &channels, 0);
	Texture<float3> texture = Texture<float3>(width, height);
	texture.mWidth = width; texture.mHeight = height;
	memcpy(texture.mData, data, sizeof(float) * width * height * 3); 
	stbi_image_free(data);
	printSuccess(path);  
	return texture;
}

Texture<float3> loadTextureI(char const* path)
{
	printLoading(path);
	if (!FileExists(path)) fileNotFound(path); 
	int width, height, channels; 
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	Texture<float3> texture = Texture<float3>(width, height); 
	texture.mWidth = width; texture.mHeight = height; 
	const int s = width * height;
	if (channels == 1) for (int i = 0; i < s; i++) /* greyscale */
	{
		uint8_t p			= data[i]; 
		uint const c		= p + (p << 8) + (p << 16); 
		uint3 const c3		= uint3((c >> 16) & 255, (c >> 8) & 255, c & 255); 
		texture.mData[i]	= float3(c3) * (1.0f / 255.0f); 
	}
	else for(int i = 0; i < s; i++) 
	{
		uint const c = (data[i * channels + 0] << 16) + (data[i * channels + 1] << 8) + data[i * channels + 2]; 
		uint3 c3 = uint3((c >> 16) & 255, (c >> 8) & 255, c & 255);
		texture.mData[i] = float3(c3) * (1.0f / 255.0f); 
	}
	stbi_image_free(data);
	printSuccess(path); 
	return texture; 
}
