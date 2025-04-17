#pragma once

#include "scene.h" 

// perfect reflection
class Metallic
{
public:
	bool Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const; 
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
};

// diffuse reflection + perfect reflection
class Glossy
{
public:
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const; 
};

// diffuse reflection + fresnel reflection 
class Glossy2
{
public:
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
};

