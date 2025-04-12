#include "precomp.h"
#include "lights.h"

PointLight::PointLight() :
	mPosition(0.0f),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 PointLight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 dir			= mPosition - intersection; 
	float const dist	= length(dir);
	dir					= normalize(dir); 

	float const cosa		= max(0.0f, dot(normal, dir));
	float const attenuation = 1.0f / (dist * dist); 

	return cosa * attenuation * mColor * mStrength; 
}

DirectionalLight::DirectionalLight() :
	mDirection(normalize(float3(1.0f, 0.0f, 0.5f))),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 DirectionalLight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float const cosa = max(0.0f, dot(normal, -mDirection));
	return cosa * mColor * mStrength;
}
