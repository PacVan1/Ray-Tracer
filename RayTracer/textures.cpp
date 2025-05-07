#include "precomp.h"
#include "textures.h"

/// all provided textures must be equal in size
PackedTexture packTexture(AlbedoTexture const& albedo, NormalTexture const& normal, RoughnessTexture roughness, Texture<float> const& alpha)
{
	PackedTexture packed = PackedTexture(albedo.mWidth, albedo.mHeight);
	uint const size = packed.mWidth * packed.mHeight;
	for (int i = 0; i < size; i++)
	{
		packed.mData[i].albedo = albedo.mData[i];
		packed.mData[i].normal = normal.mData[i];
		packed.mData[i].roughness = roughness.mData[i];
		packed.mData[i].alpha = alpha.mData[i];
	}
	return packed;
}

/// all provided textures must be equal in size
PackedTexture packTexture(AlbedoTexture const& albedo, NormalTexture const& normal)
{
	PackedTexture packed = PackedTexture(albedo.mWidth, albedo.mHeight);
	uint const size = packed.mWidth * packed.mHeight;
	for (int i = 0; i < size; i++)
	{
		packed.mData[i].albedo = albedo.mData[i];
		packed.mData[i].normal = normal.mData[i];
	}
	return packed;
}