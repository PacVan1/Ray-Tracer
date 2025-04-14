#pragma once

#include "scene.h" 

class Metallic
{
public:
	[[nodiscard]] Ray Scatter(Ray const& ray, float3 const& intersection, float3 const& normal) const;
};

class Dielectric
{
public:
	[[nodiscard]] Ray Scatter(Ray const& ray, float3 const& intersection, float3 const& normal) const;
};

[[nodiscard]] float schlickApprox(float const cosTheta, float const ior);

