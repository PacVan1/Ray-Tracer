#pragma once

#include "scene.h" 
#include "hitinfo.h" 

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

class SpotLight
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
							SpotLight();
	[[nodiscard]] float3	Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] float3	Intensity2(Scene const& scene, HitInfo const& info) const;
	void					DirectionFromLookAt();
};
