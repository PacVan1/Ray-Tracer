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
	mDirection(normalize(float3(1.0f))),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 DirectionalLight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float const cosa = max(0.0f, dot(normal, -mDirection));
	return cosa * mColor * mStrength;
}

SpotLight::SpotLight() :
	mPosition(0.0f, 1.0f, 0.0f),
	mLookAt(0.0f),
	mDirection(0.0f),
	mColor(1.0f),
	mStrength(1.0f),
	mInnerScalar(cos(20.0f / DEG_OVER_RAD)), 
	mOuterScalar(cos(22.5f / DEG_OVER_RAD)),
	mEpsilon(mInnerScalar - mOuterScalar)
{
	DirectionFromLookAt();
}

float3 SpotLight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 dir			= mPosition - intersection; 
	float const dist	= length(dir);
	dir					= normalize(dir);

	float const theta		= dot(dir, -mDirection);
	float const intensity	= clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);
  
	float const cosa			= max(0.0f, dot(normal , dir));
	float const attenuation		= 1.0f / (dist * dist); 

	return attenuation * cosa * intensity * mColor * mStrength;
}

void SpotLight::DirectionFromLookAt()
{
	mDirection = normalize(mLookAt - mPosition);  
}
