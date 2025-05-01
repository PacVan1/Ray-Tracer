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

inline float  fracf_sign(float v)			{ return copysign(v - truncf(v), v); } 
inline float2 fracf_sign(const float2& v)	{ return make_float2(fracf_sign(v.x), fracf_sign(v.y)); }
inline float3 fracf_sign(const float3& v)	{ return make_float3(fracf_sign(v.x), fracf_sign(v.y), fracf_sign(v.z)); }
inline float4 fracf_sign(const float4& v)	{ return make_float4(fracf_sign(v.x), fracf_sign(v.y), fracf_sign(v.z), fracf_sign(v.w)); } 
 
// SSE ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__forceinline __m128 sqr(__m128 const a)  
{
	// sqr(a) 
	return _mm_mul_ps(a, a); 
}
__forceinline __m128 rsqr(__m128 const a) 
{
	// 1.0f / sqr(a) 
	return _mm_rcp_ps(sqr(a));  
}
__forceinline __m128 dot(__m128 const x1, __m128 const y1, __m128 const z1, __m128 const x2, __m128 const y2, __m128 const z2)
{
	// x1 * x2 + y1 * y2 + z1 * z2
	return _mm_add_ps(_mm_add_ps(_mm_mul_ps(x1, x2), _mm_mul_ps(y1, y2)), _mm_mul_ps(z1, z2));  
}
__forceinline __m128 length(__m128 const x, __m128 const y, __m128 const z)
{
	// sqrtf(dot(v, v))
	return _mm_sqrt_ps(dot(x, y, z, x, y, z));   
}
__forceinline __m128 sqrLength(__m128 const x, __m128 const y, __m128 const z)
{
	// dot(v, v)
	return dot(x, y, z, x, y, z);
}
__forceinline __m128 rLength(__m128 const x, __m128 const y, __m128 const z)   
{
	// rsqrtf(dot(v, v))
	return _mm_rsqrt_ps(dot(x, y, z, x, y, z)); 
}
__forceinline __m128 normalize(__m128 const invLen, __m128 const a)  
{
	// invLen * a
	return _mm_mul_ps(invLen, a);    
}
__forceinline __m128 negate(__m128 const a)
{
	static __m128 const NEG_MASK = _mm_set1_ps(-0.0f); 
	return _mm_xor_ps(a, NEG_MASK);
}