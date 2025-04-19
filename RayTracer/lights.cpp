#include "precomp.h"
#include "lights.h"

#include "Renderer.h" 

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

	if (scene.IsOccluded({ intersection + -dir * Renderer::sEps, -dir })) return BLACK;

	float const cosa		= max(0.0f, dot(normal, dir));
	float const attenuation = 1.0f / (dist * dist); 

	return cosa * attenuation * mColor * mStrength; 
}

float3 PointLight::Intensity2(Scene const& scene, HitInfo const& info) const
{
	float3 dir = mPosition - info.mI;
	float const dist = length(dir);
	dir = normalize(dir);

	if (scene.IsOccluded({ info.mI + -dir * Renderer::sEps, -dir })) return BLACK;

	float const cosa = max(0.0f, dot(info.mN, dir)); 
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
	if (scene.IsOccluded({ intersection + -mDirection * Renderer::sEps, -mDirection })) return BLACK;

	float const cosa = max(0.0f, dot(normal, -mDirection));
	return cosa * mColor * mStrength;
}

float3 DirectionalLight::Intensity2(Scene const& scene, HitInfo const& info) const
{
	if (scene.IsOccluded({ info.mI + -mDirection * Renderer::sEps, -mDirection })) return BLACK;

	float const cosa = max(0.0f, dot(info.mN, -mDirection));
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

	if (scene.IsOccluded({ intersection + -dir * Renderer::sEps, -dir })) return BLACK;

	float const theta		= dot(dir, -mDirection);
	float const intensity	= clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);
  
	float const cosa			= max(0.0f, dot(normal , dir));
	float const attenuation		= 1.0f / (dist * dist); 

	return attenuation * cosa * intensity * mColor * mStrength;
}

float3 SpotLight::Intensity2(Scene const& scene, HitInfo const& info) const
{
	float3 dir = mPosition - info.mI; 
	float const dist = length(dir);
	dir = normalize(dir);

	if (scene.IsOccluded({ info.mI + -dir * Renderer::sEps, -dir })) return BLACK;

	float const theta = dot(dir, -mDirection);
	float const intensity = clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);

	float const cosa = max(0.0f, dot(info.mN, dir));
	float const attenuation = 1.0f / (dist * dist);

	return attenuation * cosa * intensity * mColor * mStrength;
}

void SpotLight::DirectionFromLookAt()
{
	mDirection = normalize(mLookAt - mPosition);  
}
