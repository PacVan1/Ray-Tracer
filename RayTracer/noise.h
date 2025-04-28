#pragma once

typedef struct
{
	uint16_t	mX;    
	uint16_t	mY;
	uint16_t	mFrame;  
	uint16_t	mBounce = 0; 
} blueSeed;

class BlueNoise 
{
public:
	Texture<float3> mTexture;

public:
	static BlueNoise const& GetInstance();

public:
			BlueNoise(char const* path);  
	float	Float(blueSeed const seed) const;
	float	FloatUnit(blueSeed const seed) const; 
	float2	Float2(blueSeed const seed) const;   
	float2	Float2Unit(blueSeed const seed) const;   
	float3	Float3(blueSeed const seed) const; 
	float3	Float3Unit(blueSeed const seed) const; 
	float3	Float3UnitLinear(blueSeed const seed) const;  
};