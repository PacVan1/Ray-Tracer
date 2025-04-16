#pragma once

#include "scene.h" 

class Metallic
{
public:
	bool Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const; 
};

class Dielectric
{
public:
	bool Scatter(Ray const& in, Ray& out, float3 const& intersection, float3 const& normal) const;
	bool Scatter2(Ray const& in, Ray& out, color& color, float3 const& intersection, float3 const& normal) const;
};

