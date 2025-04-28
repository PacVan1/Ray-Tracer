#include "precomp.h"

#include "files.h" 
#include "textures.h" 

BlueNoise const& BlueNoise::GetInstance()  
{
	static BlueNoise instance = BlueNoise("../assets/LDR_RG01_0.png"); // 64x64 blue noise texture
	return instance; 
}

BlueNoise::BlueNoise(char const* path) :
	mTexture(loadTextureI(path))  
{}

float BlueNoise::Float(blueSeed const seed) const 
{
	float const sample	= mTexture.mData[(seed.mX & (mTexture.mWidth - 1)) + (seed.mY & (mTexture.mHeight - 1)) * mTexture.mWidth].cell[seed.mBounce % 3];  
	float const noise	= fracf(sample + seed.mFrame * GOLDEN_RATIO); 
	return noise; 
}

float BlueNoise::FloatUnit(blueSeed const seed) const 
{
	return Float(seed) * 2.0f - 1.0f; 
}

float2 BlueNoise::Float2(blueSeed const seed) const 
{
	float3 const sample = mTexture.mData[(seed.mX & (mTexture.mWidth - 1)) + (seed.mY & (mTexture.mHeight - 1)) * mTexture.mWidth];  
	float2 const noise	= fracf(float2(sample.x, sample.y) + seed.mFrame * GOLDEN_RATIO); 
	return noise;
}

float2 BlueNoise::Float2Unit(blueSeed const seed) const 
{
	return Float2(seed) * 2.0f - 1.0f;   
}

float3 BlueNoise::Float3(blueSeed const seed) const  
{
	float3 const sample = mTexture.mData[(seed.mX & (mTexture.mWidth - 1)) + (seed.mY & (mTexture.mHeight - 1)) * mTexture.mWidth];
	float3 const noise = fracf(sample + seed.mFrame * GOLDEN_RATIO); 
	return noise;     
}

float3 BlueNoise::Float3Unit(blueSeed const seed) const
{ 
	return Float3(seed) * 2.0f - 1.0f; 
}

float3 BlueNoise::Float3UnitLinear(blueSeed const seed) const 
{
	float3 const sample = mTexture.SampleLinearLooped(int2(seed.mX, seed.mY));
	float3 const noise	= fracf(sample + seed.mFrame * GOLDEN_RATIO); 
	float3 const unit	= noise * 2.0f - 1.0f;
	return unit;
}
