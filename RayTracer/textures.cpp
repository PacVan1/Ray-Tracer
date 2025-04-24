#include "precomp.h"
#include "textures.h"

#include "../lib/stb_image.h"

Texture<float3> loadTexture(char const* path)
{
	FILE* f = fopen(path, "rb");
	if (!f) FatalError("File not found: %s", path);
	fclose(f);
	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;  
	float* data = stbi_loadf(path, &width, &height, &channels, 0);
	Texture<float3> texture = Texture<float3>(width, height);   
	texture.mWidth = width; texture.mHeight = height;  
	memcpy(texture.mData, data, sizeof(float) * 3 * width * height);      
	stbi_image_free(data);
	return texture; 
}

