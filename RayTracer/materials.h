#pragma once

#include "textures.h" 

struct Intersection; 
namespace tinybvh { struct Ray; }

enum materialTypes : uint8_t
{
	MATERIAL_TYPES_DIFFUSE,		// cosine-weighted diffuse reflections
	MATERIAL_TYPES_METALLIC,	// specular reflections
	MATERIAL_TYPES_DIELECTRIC,	// fresnel reflections and refractions
	MATERIAL_TYPES_GLOSSY,		// cosine-weighted diffuse reflections and specular reflections
	MATERIAL_TYPES_TEXTURED, 
	MATERIAL_TYPES_COUNT  
};

int constexpr	INIT_DEFAULT_MATERIAL_TYPE	= MATERIAL_TYPES_DIFFUSE; 
float constexpr INIT_GLOSSY_SMOOTHNESS		= 1.0f;
float constexpr INIT_DIELECTRIC_IOR			= 1.0f; 

struct Diffuse
{
	color	emission; 
	color	albedo;  
	int8_t	padding[4]	= { 0 };

	Diffuse();  
};

struct Dielectric
{
	color	emission; 
	color	absorption; 
	float	ior; 

	Dielectric(); 
};

struct Glossy
{
	color	emission; 
	color	albedo; 
	float	smoothness; 

	Glossy(); 
};

struct TexturedMaterial
{
	PackedTexture	texture;	// 24 bytes
	uint8_t			padding[8];

	TexturedMaterial();
};

class alignas(32) Material 
{
public:
	union 
	{
		Diffuse				mDiffuse;
		Dielectric			mDielectric;
		Glossy				mGlossy;  
		TexturedMaterial	mTextured; 

		union { uint8_t mDummy[31]; uint8_t mType; };
	};

public:
						Material(); 
						Material(Material const& other);
						Material(int const type);  
						~Material();
	bool				Scatter(Intersection const& hit, Ray& out, color& attenuation) const;  
	bool				Scatter(Intersection const& hit, blueSeed const seed, Ray& out, color& attenuation) const;
	bool				Scatter(tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation) const;   

	[[nodiscard]] color	GetAlbedo() const;  
	[[nodiscard]] color	GetEmission() const;
};   

struct Textured { PackedTexture texture; };

struct alignas(32) Material2
{ 
	union 
	{
		struct Glossy		{ color albedo = WHITE; float glossiness = 0.5f; }	glossy;  
		struct Dielectric	{ color absorption = WHITE; float ior = 1.0f; }		dielectric;
		//struct Textured		{ PackedTexture texture; }					textured; 
		Textured textured;
	};
	uint8_t type;

	Material2();
	Material2(Material2 const& other);
	Material2(int const type);
	~Material2();
};

bool scatter(Material2 const& mat, tinybvh::Ray const& in, tinybvh::Ray& out, color& attenuation);