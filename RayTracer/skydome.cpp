#include "precomp.h"
#include "skydome.h"

#include "renderer.h" 

Skydome::Skydome()
{}

Skydome::Skydome(char const* path) :
	mTexture(loadTextureF(path)) 
{ 
	mTexture.mSampleMode	= TEXTURE_SAMPLE_MODES_LOOPED;
	mTexture.mFilterMode	= TEXTURE_FILTER_MODES_LINEAR;  
	//mTexture.mOwnData		= true;  
}

color Skydome::Intensity(Intersection const& hit) const 
{
	float3 const random = randomUnitOnHemisphere(hit.normal);
	if (hit.scene->IsOccluded({ hit.point + random * Renderer::sEps, random })) return BLACK;
	float const cosa = max(0.0f, dot(hit.normal, random));  
	return cosa * Sample(random);
}

color Skydome::Intensity(Intersection const& hit, blueSeed const seed) const 
{
	float3 const random = randomUnitOnHemisphere(hit.normal, seed);
	if (hit.scene->IsOccluded({ hit.point + random * Renderer::sEps, random })) return BLACK; 
	float const cosa = max(0.0f, dot(hit.normal, random)); 
	return cosa * Sample(random);
}

color Skydome::Sample(float3 const& direction) const
{
	float2 const uv = calcSphereUv(direction);
	return mTexture.Sample(uv);            
}
