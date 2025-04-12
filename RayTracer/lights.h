#pragma once

class PointLight
{
public:
	float3	mPosition;
	float3	mColor;
	float	mStrength;

public:
							PointLight();
	[[nodiscard]] float3	Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const; 
};

