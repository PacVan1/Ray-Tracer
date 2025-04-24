#include "precomp.h"
#include "skydome.h"

#include "renderer.h" 

Skydome::Skydome()
{}

Skydome::Skydome(char const* path) :
	mTexture(loadTexture(path)) 
{ 
	mTexture.mSampleMode	= TEXTURE_SAMPLE_MODES_LOOPED;
	mTexture.mFilterMode	= TEXTURE_FILTER_MODES_LINEAR;  
	//mTexture.mOwnData		= true;  
}

color Skydome::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 const random = randomUnitOnHemisphere(normal); 
	if (scene.IsOccluded({ intersection + random * Renderer::sEps, random })) return BLACK;
	float const cosa = max(0.0f, dot(normal, random));
	return cosa * Sample(random);
}

color Skydome::Intensity2(Scene const& scene, HitInfo const& info) const 
{
	float3 const random = randomUnitOnHemisphere(info.mN); 
	if (scene.IsOccluded({ info.mI + random * Renderer::sEps, random })) return BLACK; 
	float const cosa = max(0.0f, dot(info.mN, random)); 
	return cosa * Sample(random);
}

color Skydome::Sample(float3 const& direction) const
{
	float2 const uv = calcSphereUv(direction);
	return mTexture.Sample(uv);            
}
