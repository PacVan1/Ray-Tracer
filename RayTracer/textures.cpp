#include "precomp.h"
#include "textures.h"

#include "../lib/stb_image.h"

HdrTexture::HdrTexture() :
	mPixels(nullptr),
	mWidth(0),
	mHeight(0)
{}

HdrTexture::HdrTexture(char const* path)
{
	int width, height, channels;

	FILE* f = fopen(path, "rb");
	if (!f) FatalError("File not found: %s", path);
	fclose(f);

	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(path, &width, &height, &channels, 0);
	if (!data) return;
	mWidth = width; mHeight = height;
	int const size = width * height;
	mPixels = static_cast<float3*>(MALLOC64(size * sizeof(float3))); 
	for (int i = 0; i < size; i++)
	{
		mPixels[i].x = data[i * channels + 0];
		mPixels[i].y = data[i * channels + 1];
		mPixels[i].z = data[i * channels + 2];
	}
	stbi_image_free(data);
}

HdrTexture::~HdrTexture()
{
	delete[] mPixels; 
}

color HdrTexture::Sample(float2 const uv) const
{
	unsigned int const ix = static_cast<unsigned int>(uv.x * static_cast<float>(mWidth));
	unsigned int const iy = static_cast<unsigned int>(uv.y * static_cast<float>(mHeight)); 
	unsigned int const px = (ix % (mWidth - 1));	// loops when coordinates are outside of texture
	unsigned int const py = (iy % (mHeight - 1));
	return mPixels[px + py * mWidth];
}
