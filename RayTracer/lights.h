#pragma once

#include "scene.h" 
#include "hitinfo.h" 

enum lightTypes : uint8_t
{
	LIGHT_TYPES_POINT, 
	LIGHT_TYPES_DIRECTIONAL, 
	LIGHT_TYPES_SPOT 
};

class PointLight
{
public:
	float3	mPosition;
	float3	mColor;
	float	mStrength;

public:
							PointLight();
	[[nodiscard]] float3	Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const; 
	[[nodiscard]] float3	Intensity2(Scene const& scene, HitInfo const& info) const;
};

class DirectionalLight
{
public:
	float3	mDirection;
	float3	mColor;
	float	mStrength;

public:
							DirectionalLight();
	[[nodiscard]] float3	Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] float3	Intensity2(Scene const& scene, HitInfo const& info) const;
};

class Spotlight 
{
public:
	float3	mPosition;
	float3	mLookAt; 
	float3	mDirection;
	float3	mColor;
	float	mStrength;

private: 
	float	mInnerScalar;
	float	mOuterScalar;
	float	mEpsilon;

public:
							Spotlight(); 
	[[nodiscard]] float3	Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] float3	Intensity2(Scene const& scene, HitInfo const& info) const;
	void					DirectionFromLookAt();
};

class TexturedSpotlight 
{
public:
	Texture<color>	mTexture;  
	Frustum			mFrustum; 
	float3			mPosition; 
	float3			mTarget;   
	float			mStrength; 

	float3			mAhead;
	float3			mRight;
	float3			mUp; 
	float			mFocusDist;
	float			mFov; 

	float3			mTopLeft;
	float3			mTopRight;
	float3			mBottomLeft;
	float3			mViewportU;
	float3			mViewportV;
	float			mViewportWidth;
	float			mViewportHeight; 
	float			mAspectRatio; 

public:
						TexturedSpotlight(); 
	[[nodiscard]] color	Intensity(Scene const& scene, HitInfo const& info) const; 
	void				Update();
};
