#pragma once

#include "scene.h" 
#include "hitinfo.h" 

// perfect reflection
class Metallic
{
public:
	bool Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
};

// fresnel reflection + refraction
class Dielectric
{
public:
	color mAbsorption; 
	float mRefractiveIdx;

public:
			Dielectric();
	bool	Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool	Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool	Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
};

// diffuse reflection + perfect reflection
class Glossy
{
public:
	color mAlbedo;
	float mSpecularProb;
	float mSmoothness; 

public:
					Glossy(); 
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
	inline color	GetAlbedo() const { return mAlbedo; }
};

// cosine weighted diffuse reflection + fresnel reflection 
class Glossy2
{
public:
	color mAlbedo;
	float mSmoothness;

public:
					Glossy2(); 
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
	inline color	GetAlbedo() const { return mAlbedo; }
};

// diffuse reflection
class Lambertian
{
public:
	color mAlbedo; 

public:
					Lambertian();
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
	inline color	GetAlbedo() const { return mAlbedo; }
};

// diffuse reflection + pdf + brdf
class Lambertian2
{
public:
	color mAlbedo;

public:
					Lambertian2();
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
	inline color	GetAlbedo() const { return mAlbedo; }
};

// diffuse reflection + pdf + brdf
class Lambertian3
{
public:
	color mAlbedo;

public:
					Lambertian3();
	bool			Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
	bool			Scatter3(Ray const& in, HitInfo const& info, Ray& out, color& color) const;
	inline color	GetAlbedo() const	{ return mAlbedo; }
	inline color	GetEmission() const { return WHITE * 20.0f; }
};

