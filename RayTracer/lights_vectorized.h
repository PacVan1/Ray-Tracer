#pragma once

#include "textures.h" 

struct Intersection; 

//#define SIMD_POINT_LIGHTS 

//float constexpr SPOTLIGHT_INNER_ANGLE					= 0.939692620f;
//float constexpr SPOTLIGHT_OUTER_ANGLE					= 0.887010833f;
//float constexpr SPOTLIGHT_EPSILON						= SPOTLIGHT_INNER_ANGLE - SPOTLIGHT_OUTER_ANGLE;
//float constexpr TEXTURED_SPOTLIGHT_VIEWPORT_HEIGHT		= 2.309401040f;
//float constexpr TEXTURED_SPOTLIGHT_VIEWPORT_DISTANCE	= 2.0f;

enum newLightTypes : uint8_t
{
	NEW_LIGHT_TYPES_POINT_LIGHT,
	NEW_LIGHT_TYPES_SPOTLIGHT,
	NEW_LIGHT_TYPES_TEXTURED_SPOTLIGHT, 
	NEW_LIGHT_TYPES_COUNT 
};

// 32 bytes point light container
struct NewPointLight
{
	float3	mPosition	= 0.0f; 
	color	mColor		= WHITE; 
	float	mIntensity	= 1.0f; 
	uint8_t mPadding[4] = { 0 }; 
};

// 128 bytes point light container (SoA) 
struct NewPointLightSoA
{
	union { float mX[4]; __m128 mX4; }; // x position
	union { float mY[4]; __m128 mY4; }; // y position
	union { float mZ[4]; __m128 mZ4; }; // z position
	union { float mR[4]; __m128 mR4; }; // r color
	union { float mG[4]; __m128 mG4; }; // g color
	union { float mB[4]; __m128 mB4; }; // b color
	union { float mI[4]; __m128 mI4; }; // intensity
	uint8_t mPadding[16] = { 0 }; 
};

// 64 bytes spotlight container
struct NewSpotlight
{
	float3	mPosition		= 0.0f;
	color	mColor			= WHITE;
	float3	mDirection		= DOWN; 
	float	mIntensity		= 1.0f;
	uint8_t mPadding[24]	= { 0 };
};

// 128 bytes textured spotlight container
struct NewTexturedSpotlight
{
	Texture<color>	mTexture;
	Frustum			mFrustum;
	float3			mPosition	= 0.0f;
	float3			mDirection	= DOWN;
	float3			mTopLeft	= 0.0f;
	float			mIntensity	= 1.0f;
};

// 128 bytes light container 
// (4 point lights OR 2 spotlights OR 1 textured spotlight)
struct NewLight
{
	union 
	{
#ifndef SIMD_POINT_LIGHTS
		NewPointLight			mPs[4];
#else
		NewPointLightSoA		mPs; 
#endif
		NewSpotlight			mSs[2];
		NewTexturedSpotlight	mTs;
		uint8_t					mDummy[128];
	};

	NewLight(int const type); 
	NewLight(NewLight const& other);
	~NewLight();
};

class NewLights
{
public:
	std::vector<NewLight>	mData; 
	std::vector<uint8_t>	mTypes; 
	uint8_t					mCount;

public:
	void				Add(int const type); 
	[[nodiscard]] color Evaluate(Intersection const& hit) const;
};

[[nodiscard]] color	evaluateLight(NewLight const& light, int const type, Intersection const& hit);
void				transformTexturedSpotlight(NewLight& light, float3 const position, float3 const target); 