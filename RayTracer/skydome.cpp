#include "precomp.h"
#include "skydome.h"

#include "renderer.h" 

Skydome::Skydome() :
	mTexture(nullptr)
{}

Skydome::Skydome(char const* path) :
	mTexture(new HdrTexture(path)) 
{}

color Skydome::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 const random = randomUnitOnHemisphere(normal); 
	if (scene.IsOccluded({ intersection + random * Renderer::sEps, random })) return BLACK;
	float const cosa = max(0.0f, dot(normal, random));
	return cosa * Sample(random);
}

color Skydome::Sample(float3 const& direction) const
{
    return mTexture->Sample(calcSphereUv(direction));
}
