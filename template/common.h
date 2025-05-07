#pragma once

#include "maths.h"  

namespace tinybvh
{
	using bvhint2 = int2;
	using bvhint3 = int3;
	using bvhuint2 = uint2;
	using bvhvec2 = float2;
	using bvhvec3 = float3;
	using bvhvec4 = float4;
}

#define NO_DOUBLE_PRECISION_SUPPORT    
#define TINYBVH_USE_CUSTOM_VECTOR_TYPES   
#include <../lib/tiny_bvh.h> 

#include "files.h" 
#include "color.h"
#include "directions.h" 
#include "noise.h" 
#include "camera.h" 
#include "resources.h" 
#include "materials.h"  

namespace Tmpl8 { class Ray; }

int constexpr	SCRWIDTH		= 1280;
int constexpr	SCRHEIGHT		= 720;  
float constexpr ASPECT_RATIO	= static_cast<float>(SCRWIDTH) / static_cast<float>(SCRHEIGHT); 
// #define FULLSCREEN
// #define DOUBLESIZE

#define DEBUG_MODE		1
//#define SINGLE_THREADED 

enum controls : uint8_t
{
	CONTROLS_MOVE_RIGHT		= GLFW_KEY_D,
	CONTROLS_MOVE_FORWARD	= GLFW_KEY_W,
	CONTROLS_MOVE_LEFT		= GLFW_KEY_A,
	CONTROLS_MOVE_BACKWARD	= GLFW_KEY_S,
	CONTROLS_MOVE_UP		= GLFW_KEY_R,
	CONTROLS_MOVE_DOWN		= GLFW_KEY_F,

	CONTROLS_LOOK_RIGHT		= GLFW_KEY_RIGHT, 
	CONTROLS_LOOK_UP		= GLFW_KEY_UP, 
	CONTROLS_LOOK_LEFT		= GLFW_KEY_LEFT, 
	CONTROLS_LOOK_DOWN		= GLFW_KEY_DOWN, 

	CONTROLS_BREAK_PIXEL	= GLFW_KEY_B,
	CONTROLS_PICTURE_MODE	= GLFW_KEY_P
};

[[nodiscard]] float2	calcSphereUv(float3 const& direction);  
[[nodiscard]] float2	randomFloat2(); 
[[nodiscard]] float3	calcIntersectionPoint(Ray const& ray); 
[[nodiscard]] float3	calcIntersectionPoint(tinybvh::Ray const& ray); 
[[nodiscard]] float3	randomUnitOnDisk();
[[nodiscard]] float3	randomUnitOnDisk(blueSeed const seed); 
[[nodiscard]] float3	randomFloat3();
[[nodiscard]] float3	randomFloat3Unit();
[[nodiscard]] float3	randomUnitOnHemisphere(float3 const& normal);
[[nodiscard]] float3	randomUnitOnHemisphere(float3 const& normal, blueSeed const seed); 
[[nodiscard]] float3	diffuseReflection(float3 const& normal);
[[nodiscard]] float3	diffuseReflection(float3 const& normal, blueSeed const seed); 
[[nodiscard]] float3	cosineWeightedDiffuseReflection(float3 const& normal);  
[[nodiscard]] float3	cosineWeightedDiffuseReflection(float3 const& normal, blueSeed const seed);   
[[nodiscard]] float		randomFloatUnit();
[[nodiscard]] float		schlickApprox(float const cosTheta, float const ior);
[[nodiscard]] float3	refract(float3 const& normal, float3 const& in, float cosTheta, float const ior);  
[[nodiscard]] float		distanceToFrustum(FrustumPlane const& frustumPlane, float3 const& point);   
[[nodiscard]] inline bool didHit(tinybvh::Ray const& ray) { return ray.hit.t < BVH_FAR; } 