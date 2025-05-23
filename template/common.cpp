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

float2 randomFloat2()
{
	return { randomFloatUnit(), randomFloatUnit() };
}

float3 calcIntersectionPoint(Ray const& ray)
{
	return ray.O + ray.D * ray.t;
}

float3 calcIntersectionPoint(tinybvh::Ray const& ray) 
{
	return ray.O + ray.D * ray.hit.t; 
}

float3 calcRawNormal(float3 const& p1, float3 const& p2, float3 const& p3)
{
	return normalize(cross(p2 - p1, p3 - p1));  
}

float3 randomUnitOnDisk()
{
	while (true)
	{
		float3 const point = { randomFloatUnit(), randomFloatUnit(), 0.0f };
		if (sqrLength(point) < 1.0f) return point;
	}
}

float3 randomUnitOnDisk(blueSeed const seed) 
{
	// try blue noise:
	float2 random = BlueNoise::GetInstance().Float2Unit(seed);  
	while (sqrLength(random) >= 1.0f) 
	{
		// if it does not work, fallback to white noise:
		random = randomFloat2();  
	}
	return { random.x, random.y, 0.0f }; 
}

float3 randomFloat3()
{
	return { randomFloatUnit(), randomFloatUnit() , randomFloatUnit() };
}

float3 randomFloat3Unit()
{
	while (true) {
		float3 const point = randomFloat3();
		float const lensq = sqrLength(point);
		if (1e-160 < lensq && lensq <= 1)
			return point / sqrt(lensq);
	}
}

float3 randomUnitOnHemisphere(float3 const& normal)
{
	float3 const random = normalize(randomFloat3());
	if (dot(random, normal) > 0.0f)
	{
		return random;
	}
	return -random;
}

float3 randomUnitOnHemisphere(float3 const& normal, blueSeed const seed) 
{
	float3 const random = normalize(BlueNoise::GetInstance().Float3Unit(seed));  
	if (dot(random, normal) > 0.0f)
	{
		return random;
	}
	return -random;
}

float3 diffuseReflection(float3 const& normal)
{
	float3 reflected; 
	do
	{
		reflected = randomFloat3();
	} while (sqrLength(reflected) > 1.0f);
	return dot(normal, reflected) < 0.0f ? -reflected : reflected; 
}

float3 diffuseReflection(float3 const& normal, blueSeed const seed)  
{
	float3 reflected;
	do
	{
		reflected = BlueNoise::GetInstance().Float3Unit(seed); 
	} while (sqrLength(reflected) > 1.0f);
	return dot(normal, reflected) < 0.0f ? -reflected : reflected;
}

float3 cosineWeightedDiffuseReflection(float3 const& normal) 
{
	float3 random;
	do
	{
		random = randomFloat3();
	} while (dot(random, random) > 1.0f);
	return normalize(normal + normalize(random));
}

float3 cosineWeightedDiffuseReflection(float3 const& normal, blueSeed const seed) 
{
	// try blue noise:
	float3 random = BlueNoise::GetInstance().Float3UnitLinear(seed); 
	while (dot(random, random) > 1.0f)
	{
		// if it does not work, fallback to white noise:
		random = randomFloat3(); 
	}
	return normalize(normal + normalize(random)); 
}

float randomFloatUnit()
{
	return RandomFloat() * 2.0f - 1.0f; 
}

float schlickApprox(float cosTheta, float const ior)
{
	// src: https://raytracing.github.io/

	float r = (1 - ior) / (1 + ior); r = r * r;
	cosTheta = (1 - cosTheta); 
	cosTheta = cosTheta * cosTheta * cosTheta * cosTheta * cosTheta; 
	return r + (1 - r) * cosTheta;  
}

float3 refract(float3 const& normal, float3 const& in, float const cosTheta, float const ior)
{
	float3 const rPerp = ior * (in + cosTheta * normal); 
	float3 const rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * normal;  
	return rPerp + rPara;
}

float distanceToFrustum(FrustumPlane const& frustumPlane, float3 const& point)  
{ 
	return dot(frustumPlane.mNormal, point) - frustumPlane.mDistance;  
}
