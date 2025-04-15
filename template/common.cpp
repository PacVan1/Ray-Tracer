#include "precomp.h"

#include "scene.h" 

float2 calcSphereUv(float3 const& direction)
{
	float const	theta	= std::acos(-direction.y);
	float const	phi		= std::atan2(-direction.z, direction.x) + PI;
	return 
	{
		clamp(phi / TWOPI, 0.0f, 1.0f),
		clamp(theta / PI, 0.0f, 1.0f)
	};
}

float3 calcIntersection(Ray const& ray)
{
	return ray.O + ray.D * ray.t;
}

float schlickApprox(float const cosTheta, float const ior)
{
	float r0 = (1 - ior) / (1 + ior);
	r0 = r0 * r0;
	float const test = (1 - cosTheta);
	return r0 + (1 - r0) * test * test * test * test * test;
}
