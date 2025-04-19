#pragma once

#include "color.h"

namespace Tmpl8 { class Ray; }

int constexpr	SCRWIDTH		= 1280;
int constexpr	SCRHEIGHT		= 720;
float constexpr ASPECT_RATIO	= static_cast<float>(SCRWIDTH) / static_cast<float>(SCRHEIGHT); 
// #define FULLSCREEN
// #define DOUBLESIZE

// constants
#define PI				3.14159265358979323846264f
#define INVPI			0.31830988618379067153777f
#define INV2PI			0.15915494309189533576888f
#define TWOPI			6.28318530717958647692528f
#define DEG_OVER_RAD	57.2957795131572957795131f 
#define SQRT_PI_INV		0.56418958355f
#define LARGE_FLOAT		1e34f

#define DEBUG_MODE		1

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
};

[[nodiscard]] float2	calcSphereUv(float3 const& direction);  
[[nodiscard]] float3	calcIntersection(Ray const& ray);
[[nodiscard]] float3	randomUnitOnDisk();
[[nodiscard]] float3	randomFloat3();
[[nodiscard]] float3	randomFloat3Unit();
[[nodiscard]] float3	randomUnitOnHemisphere(float3 const& normal);
[[nodiscard]] float3	diffuseReflection(float3 const& normal);
[[nodiscard]] float3	cosineWeightedDiffuseReflection(float3 const& normal);  
[[nodiscard]] float		randomFloatUnit();
[[nodiscard]] float		schlickApprox(float const cosTheta, float const ior);