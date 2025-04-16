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

float3 randomUnitOnDisk()
{
	while (true)
	{
		float3 const point = { randomFloatUnit(), randomFloatUnit(), 0.0f };
		if (sqrLength(point) < 1.0f) return point;
	}
}

float3 randomFloat3Unit()
{
	return { randomFloatUnit(), randomFloatUnit() , randomFloatUnit() };
}

float randomFloatUnit()
{
	return RandomFloat() * 2.0f - 1.0f; 
}

float schlickApprox(float const cosTheta, float const ior)
{
	float r = (1 - ior) / (1 + ior);
	r = r * r;
	float const cosTheta2 = (1 - cosTheta);
	return r + (1 - r) * cosTheta2 * cosTheta2 * cosTheta2 * cosTheta2 * cosTheta2;
}
