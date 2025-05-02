#pragma once

#include "textures.h" 

struct Intersection;

class Skydome
{
public:
	Texture<color> mTexture;  

public:
						Skydome(); 
						Skydome(char const* path); 
	[[nodiscard]] color Intensity(Intersection const& hit) const;
	[[nodiscard]] color Intensity(Intersection const& hit, blueSeed const seed) const;
	[[nodiscard]] color Sample(float3 const& direction) const;
};

