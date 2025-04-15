#pragma once

#include "textures.h" 
#include "scene.h" 

class Skydome
{
public: 
	[[nodiscard]] color Intensity(Ray const& ray, float3 const& intersection, float3 const& normal) const; 
};

