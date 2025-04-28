#pragma once

#include "tmpl8math.h" 

#define PI				3.14159265358979323846264f
#define INVPI			0.31830988618379067153777f
#define INV2PI			0.15915494309189533576888f
#define TWOPI			6.28318530717958647692528f
#define DEG_OVER_RAD	57.2957795131572957795131f 
#define SQRT_PI_INV		0.56418958355f
#define LARGE_FLOAT		1e34f 
#define GOLDEN_RATIO	0.61803399f

struct FrustumPlane 
{
	float3	mNormal; 
	float	mDistance; 
};

struct Frustum
{
	FrustumPlane mPlanes[4];
};

inline float  fracf_sign(float v) { return copysign(v - truncf(v), v); } 
inline float2 fracf_sign(const float2& v) { return make_float2(fracf_sign(v.x), fracf_sign(v.y)); }
inline float3 fracf_sign(const float3& v) { return make_float3(fracf_sign(v.x), fracf_sign(v.y), fracf_sign(v.z)); }
inline float4 fracf_sign(const float4& v) { return make_float4(fracf_sign(v.x), fracf_sign(v.y), fracf_sign(v.z), fracf_sign(v.w)); } 