#include "precomp.h"
#include "materials.h"

#include "renderer.h"  
#include <../lib/tiny_bvh.h> 

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
	scatterGlossy, 
	nullptr
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
	scatterGlossyBlue, 
	nullptr
};
 
typedef bool(*bvhMaterialFunc)(Material const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation);

static bool bvhScatterDiffuse(Material const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	float3 const reflected = cosineWeightedDiffuseReflection(in.hit.normal);
	float3 const brdf = mat.mDiffuse.albedo / PI;
	float const  pdf = dot(in.hit.normal, reflected) / PI; 
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);
	attenuation = brdf * (dot(in.hit.normal, reflected) / pdf); 
	return true;
}

static bool bvhScatterMetallic(Material const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	float3 const reflected = reflect(in.D, in.hit.normal); 
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);
	return true;
}

static bool bvhScatterDielectric(Material const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	float ior = 1.0f / mat.mDielectric.ior;
	float3 normal = in.hit.normal;

	if (dot(in.hit.normal, in.D) > 0.0f)
	{
		ior = mat.mDielectric.ior;
		attenuation = expf(-mat.mDielectric.absorption * in.hit.t);
		normal = -normal;
	}

	float const cosTheta = std::fmin(dot(-in.D, normal), 1.0f); 
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	if (ior * sinTheta > 1.0f || schlickApprox(cosTheta, ior) > RandomFloat())
	{
		out = tinybvh::Ray(in.hit.point + normal * Renderer::sEps, reflect(in.D, normal));
	}
	else
	{
		out = tinybvh::Ray(in.hit.point + normal * Renderer::sEps, refract(normal, in.D, cosTheta, ior));
	}
	return true;
}

static bool bvhScatterGlossy(Material const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse = cosineWeightedDiffuseReflection(in.hit.normal);
	float3 const specular = reflect(in.D, in.hit.normal); 
	bool const isSpecular = schlickApprox(std::fmin(dot(-in.D, in.hit.normal), 1.0f), 2.0f) > RandomFloat(); 
	float3 const reflected = lerp(diffuse, specular, mat.mGlossy.smoothness * static_cast<float>(isSpecular)); 
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);  
	attenuation = isSpecular ? WHITE : mat.mGlossy.albedo;
	return true;
}

static bool bvhScatterTextured(Material const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse = cosineWeightedDiffuseReflection(in.hit.normal);
	float3 const specular = reflect(in.D, in.hit.normal);
	bool const isSpecular = schlickApprox(std::fmin(dot(-in.D, in.hit.normal), 1.0f), 2.0f) > RandomFloat();
	float3 const reflected = lerp(diffuse, specular, mat.mGlossy.smoothness * static_cast<float>(isSpecular));
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);
	attenuation = isSpecular ? WHITE : mat.mGlossy.albedo;
	return true;
}

static bvhMaterialFunc bvhMatDispatchTable[MATERIAL_TYPES_COUNT] =
{
	bvhScatterDiffuse, 
	bvhScatterMetallic, 
	bvhScatterDielectric, 
	bvhScatterGlossy, 
	bvhScatterTextured 
};

typedef bool(*bvhMaterial2Func)(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation);  

static bool bvhScatterDiffuse2(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	//float3 const reflected = cosineWeightedDiffuseReflection(in.hit.normal);
	//float3 const brdf = mat.mDiffuse.albedo / PI;
	//float const  pdf = dot(in.hit.normal, reflected) / PI;
	//out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);
	//attenuation = brdf * (dot(in.hit.normal, reflected) / pdf);
	return false; // true
}

static bool bvhScatterMetallic2(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	float3 const reflected = reflect(in.D, in.hit.normal);
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);
	return true;
}

static bool bvhScatterDielectric2(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	float ior = 1.0f / mat.dielectric.ior;
	float3 normal = in.hit.normal;

	if (dot(normal, in.D) > 0.0f)
	{ 
		ior = mat.dielectric.ior;
		attenuation = expf(-mat.dielectric.absorption * in.hit.t); 
		normal = -normal;
	}

	float const cosTheta = std::fmin(dot(-in.D, normal), 1.0f);
	float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	if (ior * sinTheta > 1.0f || schlickApprox(cosTheta, ior) > RandomFloat())
	{
		out = tinybvh::Ray(in.hit.point + normal * Renderer::sEps, reflect(in.D, normal));
	}
	else
	{
		out = tinybvh::Ray(in.hit.point - normal * Renderer::sEps, refract(normal, in.D, cosTheta, ior));
	}
	return true;
}

static bool bvhScatterGlossy2(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation)
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse = cosineWeightedDiffuseReflection(in.hit.normal);
	float3 const specular = reflect(in.D, in.hit.normal);
	bool const isSpecular = schlickApprox(std::fmin(dot(-in.D, in.hit.normal), 1.0f), 2.0f) > RandomFloat();
	float3 const reflected = lerp(diffuse, specular, mat.glossy.glossiness * static_cast<float>(isSpecular));
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected); 
	attenuation = isSpecular ? WHITE : mat.glossy.albedo; 
	return true;
}

static bool bvhScatterTextured2(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation) 
{
	// inspiration:	https://www.youtube.com/watch?v=Qz0KTGYJtUk
	// timestamp:	27:14

	float3 const diffuse = cosineWeightedDiffuseReflection(in.hit.normal);
	float3 const specular = reflect(in.D, in.hit.normal);
	bool const isSpecular = schlickApprox(std::fmin(dot(-in.D, in.hit.normal), 1.0f), 2.0f) > RandomFloat();
	float3 const reflected = lerp(diffuse, specular, 0.8f * static_cast<float>(isSpecular));  
	out = tinybvh::Ray(in.hit.point + reflected * Renderer::sEps, reflected);
	attenuation = isSpecular ? WHITE : in.hit.albedo; 
	return true;
}

static bvhMaterial2Func bvhMat2DispatchTable[MATERIAL_TYPES_COUNT] = 
{
	bvhScatterDiffuse2,
	bvhScatterMetallic2,
	bvhScatterDielectric2,
	bvhScatterGlossy2,
	bvhScatterTextured2
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

TexturedMaterial::TexturedMaterial() : 
	texture(PackedTexture())
{}

Material::Material(Material const& other)
{
	switch (other.mType) 
	{
	case MATERIAL_TYPES_DIFFUSE:	new (&mDiffuse)		Diffuse(other.mDiffuse);			break;
	case MATERIAL_TYPES_METALLIC:	memcpy(this, &other, sizeof(Material));					break; 
	case MATERIAL_TYPES_DIELECTRIC: new (&mDielectric)	Dielectric(other.mDielectric);		break;
	case MATERIAL_TYPES_GLOSSY:		new (&mGlossy)		Glossy(other.mGlossy);				break; 
	case MATERIAL_TYPES_TEXTURED:	new (&mTextured)	TexturedMaterial(other.mTextured);	break;
	default: memset(this, 0, sizeof(Material)); break; 
	}
	mType = other.mType;
}

Material::Material()
{
	switch (INIT_DEFAULT_MATERIAL_TYPE) 
	{
	case MATERIAL_TYPES_DIFFUSE:	new (&mDiffuse)		Diffuse();			break;
	case MATERIAL_TYPES_DIELECTRIC: new (&mDielectric)	Dielectric();		break;
	case MATERIAL_TYPES_GLOSSY:		new (&mGlossy)		Glossy();			break;
	case MATERIAL_TYPES_TEXTURED:	new (&mTextured)	TexturedMaterial();	break; 
	default: memset(this, 0, sizeof(Material)); break;
	}
	mType = INIT_DEFAULT_MATERIAL_TYPE; 
}

Material::Material(int const type)
{
	switch (type)
	{
	case MATERIAL_TYPES_DIFFUSE:	new (&mDiffuse)		Diffuse();			break;
	case MATERIAL_TYPES_DIELECTRIC: new (&mDielectric)	Dielectric();		break; 
	case MATERIAL_TYPES_GLOSSY:		new (&mGlossy)		Glossy();			break; 
	case MATERIAL_TYPES_TEXTURED:	new (&mTextured)	TexturedMaterial();	break; 
	default: break; 
	}
	mType = type;
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

bool Material::Scatter(tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation) const   
{
	return bvhMatDispatchTable[mType](*this, in, out, attenuation);  
}

bool scatter(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation) 
{
	return bvhMat2DispatchTable[mat.type](mat, in, out, attenuation);  
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
	switch (mType)
	{
	case MATERIAL_TYPES_DIFFUSE:	return mDiffuse.emission;		break; 
	case MATERIAL_TYPES_DIELECTRIC:	return mDielectric.emission;	break; 
	case MATERIAL_TYPES_GLOSSY:		return mGlossy.emission;		break;  
	}
	return WHITE;
}

Material2::Material2()
{
	switch (INIT_DEFAULT_MATERIAL_TYPE)
	{
	case MATERIAL_TYPES_GLOSSY:		new (&glossy)		Glossy();		break;
	case MATERIAL_TYPES_DIELECTRIC: new (&dielectric)	Dielectric();	break;
	case MATERIAL_TYPES_TEXTURED:	new (&textured)		Textured();		break; 
	default: break;
	}

	type = INIT_DEFAULT_MATERIAL_TYPE;
}
Material2::Material2(Material2 const& other)
{
	switch (other.type)
	{
	case MATERIAL_TYPES_GLOSSY:		new (&glossy)		Glossy();						break;
	case MATERIAL_TYPES_DIELECTRIC: new (&dielectric)	Dielectric();					break;
	case MATERIAL_TYPES_TEXTURED:	new (&textured)		Textured(other.textured);		break;
	default: break;
	}

	this->type = other.type;
}
Material2::Material2(int const type)
{
	switch (type)
	{
	case MATERIAL_TYPES_GLOSSY:		new (&glossy)		Glossy();		break;
	case MATERIAL_TYPES_DIELECTRIC: new (&dielectric)	Dielectric();	break;
	case MATERIAL_TYPES_TEXTURED:	new (&textured)		Textured();		break;
	default: break;
	}

	this->type = type;
}
Material2::~Material2() 
{
	switch (type)
	{
	case MATERIAL_TYPES_GLOSSY:		glossy.~Glossy();			break;
	case MATERIAL_TYPES_DIELECTRIC: dielectric.~Dielectric();	break;
	case MATERIAL_TYPES_TEXTURED:	textured.~Textured();		break; 
	default: break;
	}
}