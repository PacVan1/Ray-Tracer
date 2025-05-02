#pragma once

#include "textures.h" 

struct Intersection; 

float constexpr SPOTLIGHT_INNER_ANGLE	= 0.939692620f;    
float constexpr SPOTLIGHT_OUTER_ANGLE	= 0.887010833f;  
float constexpr SPOTLIGHT_EPSILON		= SPOTLIGHT_INNER_ANGLE - SPOTLIGHT_OUTER_ANGLE;

float constexpr TEXTURED_SPOTLIGHT_VIEWPORT_HEIGHT		= 2.309401040f; 
float constexpr TEXTURED_SPOTLIGHT_VIEWPORT_DISTANCE	= 2.0f;  

enum lightTypes2 : uint8_t
{
	LIGHT_TYPES_POINT_LIGHT,
	LIGHT_TYPES_SPOTLIGHT,
	LIGHT_TYPES_COUNT 
};

enum lightTypes3 : uint8_t  
{
	LIGHT_TYPES_POINT_LIGHT2,
	LIGHT_TYPES_SPOTLIGHT2,
	LIGHT_TYPES_TEXTURED_SPOTLIGHT2,
	LIGHT_TYPES_COUNT2 
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

struct TexturedSpotlight2
{
	Texture<color>	mTexture;
	Frustum			mFrustum;
	float3			mPosition;
	float3			mAhead; 
	float3			mTopLeft; 
	float			mIntensity;
};

struct PointLights4
{
	float3	mPositions[4]	= { 0.0f }; 
	color	mColors[4]		= { WHITE };
	float	mIntensities[4] = { 1.0f };
	uint8_t mPadding[16]	= { 0 }; 
};

struct LightData
{
	union
	{
		PointLight2 mPointLight; 
		Spotlight2	mSpotlight; 
		uint8_t		mDummy[64];
	};

	LightData(int const type); 
};

struct LightData2
{
	union
	{
		PointLight2			mPs[4]; // four point lights
		Spotlight2			mSs[2]; // two spotlights
		TexturedSpotlight2	mTs;	// single textured spotlight
		uint8_t				mDummy[128];
	};

	LightData2(int const type); 
	LightData2(LightData2 const& other);
	~LightData2();  
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
	color	Evaluate(Intersection const& hit) const;
	color	EvaluateStochastic(Intersection const& hit) const;
};

class Lights2
{
public:
	std::vector<LightData2>	mData; 
	std::vector<uint8_t>	mTypes;
	uint8_t					mCount;

public:
	Lights2(); 
	void	Add(int const type);
	color	Evaluate(Intersection const& hit) const;
};

[[nodiscard]] color evaluateLight(LightData const& light, int const type, Intersection const& hit);
[[nodiscard]] color evaluateLight2(LightData2 const& light, int const type, Intersection const& hit);
void				transformTexturedSpotlight(LightData2& light, float3 const position, float3 const target); 