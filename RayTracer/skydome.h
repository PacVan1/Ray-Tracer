#pragma once

#include "textures.h" 
#include "scene.h" 
#include "hitinfo.h" 

class Skydome
{
public:
	HdrTexture* mTexture;

public:
						Skydome(); 
						Skydome(char const* path); 
	[[nodiscard]] color Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] color Intensity2(Scene const& scene, HitInfo const& info) const;
	[[nodiscard]] color Sample(float3 const& direction) const;
};

