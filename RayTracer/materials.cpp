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

Dielectric::Dielectric() :
	mAbsorption(0.0f, 0.0f, 0.0f),
	mRefractiveIdx(1.0f)
{}

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
	float ior = 1.0f / mRefractiveIdx;

	if (in.inside) // inside 
	{
		ior		= mRefractiveIdx; 
		color	= expf(-mAbsorption * in.t); 
	}

	float const cosTheta = std::fmin(dot(-in.D, tempN), 1.0f);
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	if (ior * sinTheta > 1.0f || schlickApprox(cosTheta, ior) > RandomFloat())
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

Glossy::Glossy() :
	mAlbedo(WHITE),
	mSpecularProb(0.5f),
	mSmoothness(0.5f)
{}

bool Glossy::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	// src:			https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuseDir		= cosineWeightedDiffuseReflection(normal);
	float3 const specularDir	= reflect(in.D, normal);
	bool const isSpecular		= mSpecularProb > RandomFloat(); 
	float3 const reflectedDir	= lerp(diffuseDir, specularDir, mSmoothness * static_cast<float>(isSpecular));

	out = Ray(intersection + reflectedDir * Renderer::sEps, reflectedDir);
	color = isSpecular ? WHITE : mAlbedo;

	return true;
}

Glossy2::Glossy2() :
	mAlbedo(WHITE),
	mSmoothness(1.0f)
{}

bool Glossy2::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuseDir		= cosineWeightedDiffuseReflection(normal);
	float3 const specularDir	= reflect(in.D, normal);
	bool const isSpecular		= schlickApprox(std::fmin(dot(-in.D, normal), 1.0f), 2.0f) > RandomFloat();
	float3 const reflectedDir	= lerp(diffuseDir, specularDir, mSmoothness * static_cast<float>(isSpecular));

	out		= Ray(intersection + reflectedDir * Renderer::sEps, reflectedDir);
	color	= isSpecular ? WHITE : mAlbedo;

	return true;
}

Lambertian::Lambertian() :
	mAlbedo(WHITE)
{}

bool Lambertian::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	float3 const reflected = cosineWeightedDiffuseReflection(normal);
	out		= Ray(intersection + reflected * Renderer::sEps, reflected);
	color	= mAlbedo;
	return true;
}

Lambertian2::Lambertian2() :
	mAlbedo(WHITE)
{}

bool Lambertian2::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	static float constexpr PDF = 1.0f / TWOPI;

	float3 const reflected	= diffuseReflection(normal);
	float3 const brdf		= mAlbedo / PI;
	out		= Ray(intersection + reflected * Renderer::sEps, reflected); 
	color	= brdf * (dot(normal, reflected) / PDF);
	return true; 
}

Lambertian3::Lambertian3() :
	mAlbedo(WHITE)
{}

bool Lambertian3::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	float3 const reflected	= cosineWeightedDiffuseReflection(normal);
	float3 const brdf		= mAlbedo / PI;
	float const  pdf		= dot(normal, reflected) / PI;  
	out = Ray(intersection + reflected * Renderer::sEps, reflected);
	color = brdf * (dot(normal, reflected) / pdf);
	return true;
}