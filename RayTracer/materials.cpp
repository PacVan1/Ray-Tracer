#include "precomp.h"
#include "materials.h"

#include "renderer.h"  

Material::Material() : 
	mAlbedo(WHITE), 
	mEmission(BLACK)
{}

bool Metallic::Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const
{
	float3 const reflected = reflect(in.D, normal);
	out = { intersection + reflected * Renderer::sEps, reflected };
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

bool Metallic::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const
{
	float3 const reflected = reflect(in.D, info.mN);
	out			= Ray(info.mI + reflected * Renderer::sEps, reflected); 
	out.inside	= in.inside;
	return true;
}

Dielectric::Dielectric() :
	mAbsorption(0.0f, 0.0f, 0.0f),
	mIor(1.0f)
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
	float ior = 1.0f / mIor; 

	if (in.inside) // inside 
	{
		ior		= mIor; 
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

bool Dielectric::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const
{
	float ior = 1.0f / mIor; 

	if (in.inside)
	{
		ior			= mIor; 
		attenuation = expf(-mAbsorption * in.t);
	}

	float const cosTheta = std::fmin(dot(-in.D, info.mN), 1.0f);
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
 
	if (ior * sinTheta > 1.0f || schlickApprox(cosTheta, ior) > RandomFloat()) 
	{
		out = Ray(info.mI + info.mN * Renderer::sEps, reflect(in.D, info.mN)); 
		out.inside = in.inside;
	}
	else
	{
		out = Ray(info.mI + -info.mN * Renderer::sEps, refract(info.mN, in.D, cosTheta, ior)); 
		out.inside = !in.inside;
	}
	return true;
}

Glossy::Glossy() : 
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

bool Glossy::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const 
{
	// src:			https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse	= cosineWeightedDiffuseReflection(info.mN);
	float3 const specular	= reflect(in.D, info.mN); 
	bool const isSpecular	= mSpecularProb > RandomFloat();
	float3 const reflected	= lerp(diffuse, specular, mSmoothness * static_cast<float>(isSpecular)); 
	out			= Ray(info.mI + reflected * Renderer::sEps, reflected);  
	attenuation = isSpecular ? WHITE : mAlbedo; 
	return true;
}

Glossy2::Glossy2() :
	mSmoothness(0.75f)
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

bool Glossy2::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const 
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse	= cosineWeightedDiffuseReflection(info.mN); 
	float3 const specular	= reflect(in.D, info.mN);
	bool const isSpecular	= schlickApprox(std::fmin(dot(-in.D, info.mN), 1.0f), 2.0f) > RandomFloat();
	float3 const reflected	= lerp(diffuse, specular, mSmoothness * static_cast<float>(isSpecular)); 
	out			= Ray(info.mI + reflected * Renderer::sEps, reflected); 
	attenuation = isSpecular ? WHITE : mAlbedo;    
	return true;
}

bool Lambertian::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	float3 const reflected = cosineWeightedDiffuseReflection(normal);
	out		= Ray(intersection + reflected * Renderer::sEps, reflected);
	color	= mAlbedo;
	return true;
}

bool Lambertian::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const
{
	float3 const reflected = cosineWeightedDiffuseReflection(info.mN); 
	out			= Ray(info.mI + reflected * Renderer::sEps, reflected);
	attenuation = mAlbedo;   
	return true;
}

bool Lambertian2::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	static float constexpr PDF = 1.0f / TWOPI;

	float3 const reflected	= diffuseReflection(normal);
	float3 const brdf		= mAlbedo / PI;
	out		= Ray(intersection + reflected * Renderer::sEps, reflected); 
	color	= brdf * (dot(normal, reflected) / PDF);
	return true; 
}

bool Lambertian2::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const
{
	static float constexpr PDF = 1.0f / TWOPI;

	float3 const reflected	= diffuseReflection(info.mN); 
	float3 const brdf		= mAlbedo / PI;
	out			= Ray(info.mI + reflected * Renderer::sEps, reflected);
	attenuation = brdf * (dot(info.mN, reflected) / PDF); 
	return true;
}

bool Lambertian3::Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const
{
	float3 const reflected	= cosineWeightedDiffuseReflection(normal);
	float3 const brdf		= mAlbedo / PI;
	float const  pdf		= dot(normal, reflected) / PI;  
	out = Ray(intersection + reflected * Renderer::sEps, reflected);
	color = brdf * (dot(normal, reflected) / pdf);
	return true;
}

bool Lambertian3::Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const 
{
	float3 const reflected	= cosineWeightedDiffuseReflection(info.mN); 
	float3 const brdf		= mAlbedo / PI;
	float const  pdf		= dot(info.mN, reflected) / PI;
	out			= Ray(info.mI + reflected * Renderer::sEps, reflected); 
	attenuation = brdf * (dot(info.mN, reflected) / pdf); 
	return true;
}
