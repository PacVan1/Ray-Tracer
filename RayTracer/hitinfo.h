#pragma once

class Material;   

struct HitInfo
{
	float3			mI;
	float3			mN;
	Material const*	mMat;    
	Scene const*	mScene; 
};

