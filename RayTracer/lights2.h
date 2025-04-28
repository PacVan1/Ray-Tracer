#pragma once

#include "textures.h" 
#include "hitinfo.h" 

float constexpr SPOTLIGHT_INNER_ANGLE	= 0.939692620f;    
float constexpr SPOTLIGHT_OUTER_ANGLE	= 0.887010833f;  
float constexpr SPOTLIGHT_EPSILON		= SPOTLIGHT_INNER_ANGLE - SPOTLIGHT_OUTER_ANGLE;

enum lightTypes2 : uint8_t
{
	LIGHT_TYPES_POINT_LIGHT,
	LIGHT_TYPES_SPOTLIGHT,
	LIGHT_TYPES_TEXTURED_SPOTLIGHT, 
	LIGHT_TYPES_COUNT 
};

struct PointLight2
{
	float3	mPosition		= 0.0f; 
	color	mColor			= WHITE; 
	float	mIntensity		= 1.0f;
	uint8_t mPadding[4]		= { 0 }; 
};

struct Spotlight2
{
	float3	mPosition		= 0.0f;
	color	mColor			= WHITE;  
	float3	mDirection		= float3(0.0f, -1.0f, 0.0f);  
	float	mIntensity		= 1.0f;
	uint8_t mPadding[24]	= { 0 }; 
};

struct TexturedSpotlight
{
	Texture<color>	mTexture;
	Frustum			mFrustum;
	float3			mPosition;
	float3			mAhead; 
	float3			mTopLeft; 
	float			mIntensity;
};

struct LightData
{
	union
	{
		uint8_t		mDummy[64] = { 0 }; 
		PointLight2 mPointLight; 
		Spotlight2	mSpotlight; 
	};

	LightData(int const type); 
};

class Lights 
{
public:
	std::vector<LightData>	mData;
	std::vector<uint8_t>	mTypes;  
	uint8_t					mCount; 
	uint8_t					mSamplesPerPixel = 1;  

public:
			Lights();
	void	Add(int const type);  
	color	Evaluate(HitInfo const& hit) const; 
	color	EvaluateStochastic(HitInfo const& hit) const;
};

[[nodiscard]] color evaluateLight(LightData const& light, int const type, HitInfo const& hit);