#pragma once

#include "scene.h" 
#include "hitinfo.h" 

class Material
{
public: 
	color mAlbedo; 
	color mEmission;  

public:
					Material(); 
	virtual bool	Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const = 0; 
	virtual bool	Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const = 0;

	inline color	GetAlbedo() const	{ return mAlbedo; } 
	inline color	GetEmission() const	{ return mEmission; } 
};

// perfect reflection
class Metallic : public Material
{
public:
	bool Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override; 
	bool Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;  
};

// fresnel reflection + refraction
class Dielectric : public Material
{
public:
	color mAbsorption; 
	float mIor;  

public:
			Dielectric();
	bool	Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool	Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool	Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override;
	bool	Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;
};

// diffuse reflection + perfect reflection
class Glossy : public Material
{
public:
	float mSpecularProb;
	float mSmoothness; 

public:
					Glossy(); 
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;
};

// cosine weighted diffuse reflection + fresnel reflection 
class Glossy2 : public Material
{
public:
	float mSmoothness;

public:
					Glossy2(); 
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;
};

// diffuse reflection
class Lambertian : public Material
{
public:
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;
	inline color	GetAlbedo() const { return mAlbedo; }
};

// diffuse reflection + pdf + brdf
class Lambertian2 : public Material
{
public:
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;
	inline color	GetAlbedo() const { return mAlbedo; }
};

// diffuse reflection + pdf + brdf
class Lambertian3 : public Material 
{
public:
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation) const override;
	bool			Scatter(Ray const& in, HitInfo const& info, Ray& out, color& attenuation, blueSeed const seed) const override;
	inline color	GetAlbedo() const	{ return mAlbedo; }
	inline color	GetEmission() const { return WHITE * 20.0f; }
};
