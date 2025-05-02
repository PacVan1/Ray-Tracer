#pragma once

struct Intersection; 

enum materialTypes : uint8_t
{
	MATERIAL_TYPES_DIFFUSE,		// cosine-weighted diffuse reflections
	MATERIAL_TYPES_METALLIC,	// specular reflections
	MATERIAL_TYPES_DIELECTRIC,	// fresnel reflections and refractions
	MATERIAL_TYPES_GLOSSY,		// cosine-weighted diffuse reflections and specular reflections
	MATERIAL_TYPES_COUNT 
};

int constexpr	INIT_DEFAULT_MATERIAL_TYPE	= MATERIAL_TYPES_DIFFUSE; 
float constexpr INIT_GLOSSY_SMOOTHNESS		= 1.0f;
float constexpr INIT_DIELECTRIC_IOR			= 1.0f; 

struct Diffuse
{
	color	albedo;  
	int8_t	padding[4]	= { 0 };

	Diffuse();  
};

struct Dielectric
{
	color	absorption; 
	float	ior; 

	Dielectric(); 
};

struct Glossy
{
	color	albedo; 
	float	smoothness; 

	Glossy(); 
};

class Material
{
public:
	color emission; 
	union
	{
		Diffuse		mDiffuse;
		Dielectric	mDielectric;
		Glossy		mGlossy;  
		uint8_t		mPadding[28]; // 28 bytes for material data
	};
	int mType;  

public:
						Material(); 
						Material(int const type);  
						~Material();
	bool				Scatter(Intersection const& hit, Ray& out, color& attenuation) const;  
	bool				Scatter(Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation) const;

	[[nodiscard]] color	GetAlbedo() const;  
	[[nodiscard]] color	GetEmission() const;
};   