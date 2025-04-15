#pragma once

#include "color.h" 

namespace Tmpl8 { class Ray; }

// default screen resolution
#define SCRWIDTH		1280
#define SCRHEIGHT		720
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
	CONTROLS_MOVE_EAST		= GLFW_KEY_D,
	CONTROLS_MOVE_NORTH		= GLFW_KEY_W,
	CONTROLS_MOVE_WEST		= GLFW_KEY_A,
	CONTROLS_MOVE_SOUTH		= GLFW_KEY_S,
	CONTROLS_MOVE_UP		= GLFW_KEY_R,
	CONTROLS_MOVE_DOWN		= GLFW_KEY_F,

	CONTROLS_LOOK_EAST		= GLFW_KEY_RIGHT, 
	CONTROLS_LOOK_NORTH		= GLFW_KEY_UP, 
	CONTROLS_LOOK_WEST		= GLFW_KEY_LEFT, 
	CONTROLS_LOOK_SOUTH		= GLFW_KEY_DOWN, 

	CONTROLS_BREAK_PIXEL	= GLFW_KEY_B,
};

[[nodiscard]] float2	calcSphereUv(float3 const& direction);  
[[nodiscard]] float3	calcIntersection(Ray const& ray);
[[nodiscard]] float		schlickApprox(float const cosTheta, float const ior);