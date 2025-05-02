#include "precomp.h"
#include "lights_vectorized.h"

#include "renderer.h" 
#include "lights_stochastic.h" 

typedef color(*lightFunction)(NewLight const& light, Intersection const& hit); 

static color evaluatePointLights4(NewLight const& light, Intersection const& hit) 
{
#ifndef SIMD_POINT_LIGHTS
	color result = BLACK;
	for (int i = 0; i < 4; i++)
	{
		float3 dir = hit.point - light.mPs[i].mPosition; 
		float const dist = length(dir);
		dir = normalize(dir);

		//Ray shadow = Ray(hit.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
		//if (hit.mScene->IsOccluded(shadow)) return BLACK; 

		float const cosa = max(0.0f, dot(hit.normal, -dir));
		float const attenuation = 1.0f / (dist * dist);

		result += cosa * attenuation * light.mPs[i].mColor * light.mPs[i].mIntensity;
	}
	return result;
#else 
	// hit.mI
	__m128 const ix4 = _mm_set1_ps(hit.mI.x);   
	__m128 const iy4 = _mm_set1_ps(hit.mI.y);   
	__m128 const iz4 = _mm_set1_ps(hit.mI.z);  
	__m128 const nx4 = _mm_set1_ps(hit.mN.x); 
	__m128 const ny4 = _mm_set1_ps(hit.mN.y); 
	__m128 const nz4 = _mm_set1_ps(hit.mN.z); 

	// float3 dir = hit.mI - light.mPs[i].mPosition;
	__m128 dx4 = _mm_sub_ps(ix4, light.mPs.mX4);   
	__m128 dy4 = _mm_sub_ps(iy4, light.mPs.mY4);   
	__m128 dz4 = _mm_sub_ps(iz4, light.mPs.mZ4);   

	// TODO: compute the sqrt only once:
	// float const dist = length(dir);
	__m128 const dist4 = length(dx4, dy4, dz4); 

	//Ray shadow = Ray(hit.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	//if (hit.mScene->IsOccluded(shadow)) return BLACK; 

	// dir = normalize(dir);
	__m128 const invLen = rLength(dx4, dy4, dz4);  
	dx4 = normalize(invLen, dx4);    
	dy4 = normalize(invLen, dy4);    
	dz4 = normalize(invLen, dz4);   

	// float const cosa = max(0.0f, dot(hit.mN, -dir)); 
	__m128 const cosa4 = _mm_max_ps(_mm_setzero_ps(), negate(dot(nx4, ny4, nz4, dx4, dy4, dz4)));      
	__m128 const atte4 = rsqr(dist4); 

	__m128 const one4 = _mm_set1_ps(1.0f);  
	__m128 const r4 = _mm_mul_ps(cosa4, _mm_mul_ps(atte4, _mm_mul_ps(one4, one4)));   
	__m128 const g4 = _mm_mul_ps(cosa4, _mm_mul_ps(atte4, _mm_mul_ps(one4, one4)));  
	__m128 const b4 = _mm_mul_ps(cosa4, _mm_mul_ps(atte4, _mm_mul_ps(one4, one4))); 

	__m128 const rsum1 = _mm_hadd_ps(r4, r4); __m128 const rsum2 = _mm_hadd_ps(rsum1, rsum1); 
	__m128 const gsum1 = _mm_hadd_ps(g4, g4); __m128 const gsum2 = _mm_hadd_ps(gsum1, gsum1);
	__m128 const bsum1 = _mm_hadd_ps(b4, b4); __m128 const bsum2 = _mm_hadd_ps(bsum1, bsum1);  

	color c;
	_mm_store_ss(&c.r, rsum2);
	_mm_store_ss(&c.g, gsum2); 
	_mm_store_ss(&c.b, bsum2);
	return c; 
#endif
}

#ifdef SIMD_POINT_LIGHTS
static color evaluatePointLights4_2(NewLight const& light, HitInfo const& hit)
{
	// hit.mI
	__m128 const ix4 = _mm_set1_ps(hit.mI.x);
	__m128 const iy4 = _mm_set1_ps(hit.mI.y);
	__m128 const iz4 = _mm_set1_ps(hit.mI.z);
	__m128 const nx4 = _mm_set1_ps(hit.mN.x);
	__m128 const ny4 = _mm_set1_ps(hit.mN.y);
	__m128 const nz4 = _mm_set1_ps(hit.mN.z);

	// float3 dir = hit.mI - light.mPs[i].mPosition;
	__m128 dx4 = _mm_sub_ps(ix4, light.mPs.mX4);
	__m128 dy4 = _mm_sub_ps(iy4, light.mPs.mY4);
	__m128 dz4 = _mm_sub_ps(iz4, light.mPs.mZ4);

	// TODO: compute the sqrt only once:
	// float const dist = length(dir);
	__m128 const dist4 = length(dx4, dy4, dz4);

	//Ray shadow = Ray(hit.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	//if (hit.mScene->IsOccluded(shadow)) return BLACK; 
	__m128 eps4 = _mm_set1_ps(Renderer::sEps); 
	__m128 const shadowOX4 = _mm_sub_ps(ix4, _mm_mul_ps(dx4, eps4));   
	__m128 const shadowOY4 = _mm_sub_ps(iy4, _mm_mul_ps(dy4, eps4));     
	__m128 const shadowOZ4 = _mm_sub_ps(iz4, _mm_mul_ps(dz4, eps4));   
	__m128 const shadowDX4 = negate(dx4);  
	__m128 const shadowDY4 = negate(dy4); 
	__m128 const shadowDZ4 = negate(dz4); 
	__m128 const shadowDist = _mm_sub_ps(dist4, eps4); 

	// dir = normalize(dir);
	__m128 const invLen = rLength(dx4, dy4, dz4);
	dx4 = normalize(invLen, dx4);
	dy4 = normalize(invLen, dy4);
	dz4 = normalize(invLen, dz4);

	// float const cosa = max(0.0f, dot(hit.mN, -dir)); 
	__m128 const cosa4 = _mm_max_ps(_mm_setzero_ps(), negate(dot(nx4, ny4, nz4, dx4, dy4, dz4)));
	__m128 const atte4 = rsqr(dist4);

	__m128 const one4 = _mm_set1_ps(1.0f);
	__m128 const r4 = _mm_mul_ps(cosa4, _mm_mul_ps(atte4, _mm_mul_ps(one4, one4)));
	__m128 const g4 = _mm_mul_ps(cosa4, _mm_mul_ps(atte4, _mm_mul_ps(one4, one4)));
	__m128 const b4 = _mm_mul_ps(cosa4, _mm_mul_ps(atte4, _mm_mul_ps(one4, one4)));

	__m128 const rsum1 = _mm_hadd_ps(r4, r4); __m128 const rsum2 = _mm_hadd_ps(rsum1, rsum1);
	__m128 const gsum1 = _mm_hadd_ps(g4, g4); __m128 const gsum2 = _mm_hadd_ps(gsum1, gsum1);
	__m128 const bsum1 = _mm_hadd_ps(b4, b4); __m128 const bsum2 = _mm_hadd_ps(bsum1, bsum1);

	color c;
	_mm_store_ss(&c.r, rsum2);
	_mm_store_ss(&c.g, gsum2);
	_mm_store_ss(&c.b, bsum2);
	return c;
}
#endif

static color evaluateSpotlights2(NewLight const& light, Intersection const& hit) 
{
	for (int i = 0; i < 2; i++) 
	{
		float3 dir = hit.point - light.mSs[i].mPosition; 
		float const dist = length(dir);
		dir = normalize(dir);

		Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
		if (hit.scene->IsOccluded(shadow)) return BLACK;

		float const theta = dot(dir, light.mSs[i].mDirection);
		float const fallOff = clamp((theta - SPOTLIGHT_OUTER_ANGLE) / SPOTLIGHT_EPSILON, 0.0f, 1.0f);
		float const cosa = max(0.0f, dot(hit.normal, -dir));
		float const attenuation = 1.0f / (dist * dist);

		return attenuation * cosa * fallOff * light.mSs[i].mColor * light.mSs[i].mIntensity;
	}
}

static color evaluateTexturedSpotlight(NewLight const& light, Intersection const& hit)
{
	float3 dir			= hit.point - light.mTs.mPosition; 
	float const dist	= length(dir);
	dir					= normalize(dir);

	Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
	if (hit.scene->IsOccluded(shadow)) return BLACK;

	float const cosa = max(0.0f, dot(hit.normal, -dir));
	float const attenuation = 1.0f / (dist * dist);

	float const dLeft	= distanceToFrustum(light.mTs.mFrustum.mPlanes[0], hit.point);
	float const dRight	= distanceToFrustum(light.mTs.mFrustum.mPlanes[1], hit.point);
	float const dTop	= distanceToFrustum(light.mTs.mFrustum.mPlanes[2], hit.point);
	float const dBottom = distanceToFrustum(light.mTs.mFrustum.mPlanes[3], hit.point);
	float const x = dLeft / (dLeft + dRight);
	float const y = dTop / (dTop + dBottom);
	return x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f ?
		cosa * attenuation * light.mTs.mTexture.Sample(float2(x, y)) * light.mTs.mIntensity : BLACK;
}

lightFunction lightDispatchTable[NEW_LIGHT_TYPES_COUNT] =
{
	evaluatePointLights4,
	evaluateSpotlights2,
	evaluateTexturedSpotlight
};

color evaluateLight(NewLight const& light, int const type, Intersection const& hit) 
{
	return lightDispatchTable[type](light, hit);  
}

void transformTexturedSpotlight(NewLight& light, float3 const position, float3 const target)
{ 
	light.mTs.mPosition		= position;
	light.mTs.mDirection	= normalize(target - position);
	float3 right			= normalize(cross(float3(0.0f, 1.0f, 0.0f), light.mTs.mDirection));
	float3 up				= normalize(cross(light.mTs.mDirection, right));

	float const viewportWidth = TEXTURED_SPOTLIGHT_VIEWPORT_HEIGHT * light.mTs.mTexture.mAspectRatio;
	float3 const viewportU = viewportWidth * right;
	float3 const viewportV = TEXTURED_SPOTLIGHT_VIEWPORT_HEIGHT * -up;

	light.mTs.mTopLeft = light.mTs.mPosition + TEXTURED_SPOTLIGHT_VIEWPORT_DISTANCE * light.mTs.mDirection - viewportU * 0.5f - viewportV * 0.5f;
	float3 const topRight = light.mTs.mTopLeft + viewportU;
	float3 const bottomLeft = light.mTs.mTopLeft + viewportV;

	Frustum frustum = {};
	frustum.mPlanes[0].mNormal = cross(light.mTs.mTopLeft - bottomLeft, light.mTs.mTopLeft - light.mTs.mPosition);  // left 
	frustum.mPlanes[1].mNormal = cross(topRight - light.mTs.mPosition, light.mTs.mTopLeft - bottomLeft);			// right  
	frustum.mPlanes[2].mNormal = cross(topRight - light.mTs.mTopLeft, light.mTs.mTopLeft - light.mTs.mPosition);    // top   
	frustum.mPlanes[3].mNormal = cross(bottomLeft - light.mTs.mPosition, topRight - light.mTs.mTopLeft);			// bottom   
	for (int i = 0; i < 4; i++)
	{
		frustum.mPlanes[i].mDistance = distanceToFrustum(frustum.mPlanes[i], light.mTs.mPosition);
	}
	light.mTs.mFrustum = frustum;
}

NewLight::NewLight(int const type) 
{
	switch (type)
	{ 
#ifndef SIMD_POINT_LIGHTS 
	case NEW_LIGHT_TYPES_POINT_LIGHT:			std::fill(mPs, mPs + 4, NewPointLight());	break; 
#else
	case NEW_LIGHT_TYPES_POINT_LIGHT:			mPs = NewPointLightSoA(); break;     
#endif
	case NEW_LIGHT_TYPES_SPOTLIGHT:				std::fill(mSs, mSs + 2, NewSpotlight());	break; 
	case NEW_LIGHT_TYPES_TEXTURED_SPOTLIGHT:	mTs = NewTexturedSpotlight();				break; 
	default: memset(mDummy, 0, 128); break; 
	}
}

NewLight::NewLight(NewLight const& other)
{
	memcpy(mDummy, other.mDummy, 128); 
}

NewLight::~NewLight() 
{}

void NewLights::Add(int const type)
{
	mData.emplace_back(type); 
	mTypes.emplace_back(type);
	mCount = mData.size();  
}

color NewLights::Evaluate(Intersection const& hit) const
{
	color result = BLACK; 
	for (int i = 0; i < mCount; i++)
	{
		result += evaluateLight(mData[i], mTypes[i], hit); 
	}
	return result; 
}
