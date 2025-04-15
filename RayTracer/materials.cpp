#include "precomp.h"
#include "materials.h"

#include "renderer.h"  

Ray Metallic::Scatter(Ray const& ray, float3 const& intersection, float3 const& normal) const
{
	float3 const reflectedDir = reflect(ray.D, normal);
	Ray reflected = Ray(intersection + reflectedDir * Renderer::sEps, reflectedDir);
	reflected.inside = ray.inside; 
	return reflected;
}

Ray Dielectric::Scatter(Ray const& ray, float3 const& intersection, float3 const& normal) const
{
	float3 tempN = normal;
	float ior = 1.0f / 1.33f;

	if (ray.inside) // inside 
	{
		ior = 1.33f;
	}

	float cosTheta = std::fmin(dot(-ray.D, tempN), 1.0f);
	float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	//bool cannotRefract = ior * sinTheta > 1.0f; // total internal reflection       
	float fresnel = schlickApprox(cosTheta, ior);

	float3 reflectedDir = reflect(ray.D, tempN);
	Ray reflected = Ray(intersection + reflectedDir * Renderer::sEps, reflectedDir);
	reflected.inside = ray.inside;

	float3 rPerp = ior * (ray.D + cosTheta * tempN);
	float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
	float3 refractedDir = rPerp + rPara;
	Ray refracted = Ray(intersection + refractedDir * Renderer::sEps, refractedDir);
	refracted.inside = ray.inside;

	return Ray();
}