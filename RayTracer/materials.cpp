#include "precomp.h"
#include "materials.h"

#include "renderer.h"  

typedef bool(*materialFunc)(Material const& mat, Intersection const& hit, Ray& out, color& attenuation);

static bool scatterDiffuse(Material const& mat, Intersection const& hit, Ray& out, color& attenuation)
{
	float3 const reflected = cosineWeightedDiffuseReflection(hit.normal);   
	float3 const brdf	= mat.mDiffuse.albedo / PI; 
	float const  pdf	= dot(hit.normal, reflected) / PI;  
	out			= Ray(hit.point + reflected * Renderer::sEps, reflected);
	attenuation = brdf * (dot(hit.normal, reflected) / pdf);   
	return true;
}

static bool scatterMetallic(Material const& mat, Intersection const& hit, Ray& out, color& attenuation)
{
	float3 const reflected = reflect(hit.in, hit.normal);    
	out			= Ray(hit.point + reflected * Renderer::sEps, reflected);
	out.inside	= hit.inside;   
	return true; 
}

static bool scatterDielectric(Material const& mat, Intersection const& hit, Ray& out, color& attenuation)
{
	float ior = 1.0f / mat.mDielectric.ior; 

	if (hit.inside) 
	{
		ior = mat.mDielectric.ior;
		attenuation = expf(-mat.mDielectric.absorption * hit.t);
	}

	float const cosTheta = std::fmin(dot(-hit.in, hit.normal), 1.0f); 
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	if (ior * sinTheta > 1.0f || schlickApprox(cosTheta, ior) > RandomFloat())
 	{
		out = Ray(hit.point + hit.normal * Renderer::sEps, reflect(hit.in, hit.normal));
		out.inside = hit.inside; 
	}
	else
	{
		out = Ray(hit.point + -hit.normal * Renderer::sEps, refract(hit.normal, hit.in, cosTheta, ior));  
		out.inside = !hit.inside;
 	}
	return true;
}

static bool scatterGlossy(Material const& mat, Intersection const& hit, Ray& out, color& attenuation)
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse	= cosineWeightedDiffuseReflection(hit.normal); 
	float3 const specular	= reflect(hit.in, hit.normal); 
	bool const isSpecular	= schlickApprox(std::fmin(dot(-hit.in, hit.normal), 1.0f), 2.0f) > RandomFloat();
	float3 const reflected	= lerp(diffuse, specular, mat.mGlossy.smoothness * static_cast<float>(isSpecular));    
	out			= Ray(hit.point + reflected * Renderer::sEps, reflected); 
	attenuation = isSpecular ? WHITE : mat.mGlossy.albedo; 
	return true;
} 

static materialFunc matDispatchTable[MATERIAL_TYPES_COUNT] =
{
	scatterDiffuse, 
	scatterMetallic, 
	scatterDielectric, 
	scatterGlossy 
};

typedef bool(*blueMaterialFunc)(Material const& mat, Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation);

static bool scatterDiffuseBlue(Material const& mat, Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation)
{
	float3 const reflected	= cosineWeightedDiffuseReflection(hit.normal, seed); 
	float3 const brdf		= mat.mDiffuse.albedo / PI;
	float const  pdf		= dot(hit.normal, reflected) / PI;
	out			= Ray(hit.point + reflected * Renderer::sEps, reflected);
	attenuation = brdf * (dot(hit.normal, reflected) / pdf);
	return true;
}

static bool scatterMetallicBlue(Material const& mat, Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation)
{
	float3 const reflected = reflect(hit.in, hit.normal);
	out			= Ray(hit.point + reflected * Renderer::sEps, reflected);
	out.inside	= hit.inside;
	return true;
}

static bool scatterDielectricBlue(Material const& mat, Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation)
{
	float ior = 1.0f / mat.mDielectric.ior;

	if (hit.inside)
	{
		ior = mat.mDielectric.ior;
		attenuation = expf(-mat.mDielectric.absorption * hit.t);
	}

	float const cosTheta = std::fmin(dot(-hit.in, hit.normal), 1.0f);
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	if (ior * sinTheta > 1.0f || schlickApprox(cosTheta, ior) > BlueNoise::GetInstance().Float(seed)) 
	{
		out = Ray(hit.point + hit.normal * Renderer::sEps, reflect(hit.in, hit.normal));
		out.inside = hit.inside;
	}
	else
	{
		out = Ray(hit.point + -hit.normal * Renderer::sEps, refract(hit.normal, hit.in, cosTheta, ior)); 
		out.inside = !hit.inside;
	}
	return true;
}

static bool scatterGlossyBlue(Material const& mat, Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation)
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse	= cosineWeightedDiffuseReflection(hit.normal, seed); 
	float3 const specular	= reflect(hit.in, hit.normal);
	bool const isSpecular	= schlickApprox(std::fmin(dot(-hit.in, hit.normal), 1.0f), 2.0f) > BlueNoise::GetInstance().Float(seed);
	float3 const reflected	= lerp(diffuse, specular, mat.mGlossy.smoothness * static_cast<float>(isSpecular));
	out			= Ray(hit.point + reflected * Renderer::sEps, reflected);
	attenuation = isSpecular ? WHITE : mat.mGlossy.albedo;
	return true;
}
 
static blueMaterialFunc blueMatDispatchTable[MATERIAL_TYPES_COUNT] = 
{
	scatterDiffuseBlue,
	scatterMetallicBlue,
	scatterDielectricBlue,
	scatterGlossyBlue 
};
 
Diffuse::Diffuse() : 
	albedo(WHITE) 
{}

Dielectric::Dielectric() : 
	absorption(BLACK), 
	ior(INIT_DIELECTRIC_IOR) 
{}

Glossy::Glossy() :
	albedo(WHITE),
	smoothness(INIT_GLOSSY_SMOOTHNESS)   
{}

Material::Material() : 
	mType(INIT_DEFAULT_MATERIAL_TYPE) 
{
	memset(mPadding, 0, 28); 
}

Material::Material(int const type) :
	mType(type), 
	emission(BLACK) 
{
	switch (type)
	{
	case MATERIAL_TYPES_DIFFUSE:	new (&mDiffuse)		Diffuse();		break;
	case MATERIAL_TYPES_DIELECTRIC: new (&mDielectric)	Dielectric();	break; 
	case MATERIAL_TYPES_GLOSSY:		new (&mGlossy)		Glossy();		break; 
	default: memset(mPadding, 0, 28); break; 
	}
}

Material::~Material()
{}

bool Material::Scatter(Intersection const& hit, Ray& out, color& attenuation) const 
{
	return matDispatchTable[mType](*this, hit, out, attenuation); 
}

bool Material::Scatter(Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation) const 
{
	return blueMatDispatchTable[mType](*this, hit, seed, out, attenuation);   
}

color Material::GetAlbedo() const
{
	switch (mType)
	{
	case MATERIAL_TYPES_DIFFUSE:	return mDiffuse.albedo;	break;
	case MATERIAL_TYPES_GLOSSY:		return mGlossy.albedo;	break;
	}
	return WHITE;
}

color Material::GetEmission() const 
{
	return emission; 
}