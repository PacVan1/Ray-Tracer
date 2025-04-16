#include "precomp.h"
#include "materials.h"

#include "renderer.h"  

bool Metallic::Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const
{
	float3 const reflectedDir = reflect(in.D, normal);
	out = { intersection + reflectedDir * Renderer::sEps, reflectedDir };
	out.inside = in.inside; 
	return true;
}

bool Metallic::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	float3 const reflectedDir = reflect(in.D, normal);
	out = { intersection + reflectedDir * Renderer::sEps, reflectedDir };
	out.inside = in.inside;
	return true;
}

bool Dielectric::Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const
{
	float3 tempN = normal;
	float ior = 1.0f / 1.33f;

	if (in.inside) // inside 
	{
		ior = 1.33f;
	}

	float const cosTheta	= std::fmin(dot(-in.D, tempN), 1.0f);
	float const sinTheta	= std::sqrt(1.0f - cosTheta * cosTheta);
	float const fresnel		= schlickApprox(cosTheta, ior);

	if (ior * sinTheta > 1.0f || fresnel > RandomFloat())
	{
		out = { intersection + normal * Renderer::sEps, reflect(in.D, tempN) };
		out.inside = in.inside;
		return true; 
	}

	float3 rPerp = ior * (in.D + cosTheta * tempN);
	float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
	out = { intersection + (-normal) * Renderer::sEps, rPerp + rPara };
	out.inside = !in.inside;
	return true; 
}

bool Dielectric::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	float3 tempN = normal;
	float ior = 1.0f / 1.33f;

	if (in.inside) // inside 
	{
		ior = 1.33f;
		color = expf(-float3(0.9f, 1.8f, 2.3f) * in.t); 
	}

	float const cosTheta = std::fmin(dot(-in.D, tempN), 1.0f);
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
	float const fresnel = schlickApprox(cosTheta, ior);

	if (ior * sinTheta > 1.0f || fresnel > RandomFloat())
	{
		out = { intersection + normal * Renderer::sEps, reflect(in.D, tempN) };
		out.inside = in.inside;
		return true;
	}

	float3 rPerp = ior * (in.D + cosTheta * tempN);
	float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
	out = { intersection + (-normal) * Renderer::sEps, rPerp + rPara };
	out.inside = !in.inside;
	return true;
}
