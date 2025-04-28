#include "precomp.h"
#include "lights2.h"

#include "renderer.h" 

typedef color(*lightFunction)(LightData const& light, HitInfo const& hit); 

color evaluatePointLight(LightData const& light, HitInfo const& hit)
{
	float3 dir			= hit.mI - light.mPointLight.mPosition;   
	float const dist	= length(dir); 
	dir					= normalize(dir); 

	Ray shadow = Ray(hit.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	if (hit.mScene->IsOccluded(shadow)) return BLACK;  

	float const cosa		= max(0.0f, dot(hit.mN, -dir)); 
	float const attenuation = 1.0f / (dist * dist); 

	return cosa * attenuation * light.mPointLight.mColor * light.mPointLight.mIntensity; 
}

color evaluateSpotlight(LightData const& light, HitInfo const& hit) 
{
	float3 dir = hit.mI - light.mSpotlight.mPosition;  
	float const dist = length(dir); 
	dir = normalize(dir); 

	Ray shadow = Ray(hit.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	if (hit.mScene->IsOccluded(shadow)) return BLACK; 

	float const theta		= dot(dir, light.mSpotlight.mDirection); 
	float const fallOff		= clamp((theta - SPOTLIGHT_OUTER_ANGLE) / SPOTLIGHT_EPSILON, 0.0f, 1.0f);
	float const cosa		= max(0.0f, dot(hit.mN, -dir));
	float const attenuation = 1.0f / (dist * dist);

	return attenuation * cosa * fallOff * light.mSpotlight.mColor * light.mSpotlight.mIntensity; 
}

color evaluateTexturedSpotlight(LightData const& light, HitInfo const& hit)
{
	//float3 right = normalize(cross(float3(0.0f, 1.0f, 0.0f), mAhead));
	//mUp = normalize(cross(mAhead, mRight));
	//
	//float const theta = mFov / DEG_OVER_RAD;
	//float const height = tan(theta * 0.5f);
	//
	//mViewportHeight = 2.0f * height * mFocusDist;
	//mViewportWidth = mViewportHeight * mAspectRatio;
	//mViewportU = mViewportWidth * mRight;
	//mViewportV = mViewportHeight * -mUp;
	//
	//mTopLeft = mPosition + mFocusDist * mAhead - mViewportU * 0.5f - mViewportV * 0.5f;
	//mTopRight = mTopLeft + mViewportU;
	//mBottomLeft = mTopLeft + mViewportV;
	//
	//// update frustum:
	//Frustum frustum = {};
	//frustum.mPlanes[0].mNormal = cross(mTopLeft - mBottomLeft, mTopLeft - mPosition);  // left 
	//frustum.mPlanes[1].mNormal = cross(mTopRight - mPosition, mTopLeft - mBottomLeft); // right
	//frustum.mPlanes[2].mNormal = cross(mTopRight - mTopLeft, mTopLeft - mPosition);    // top
	//frustum.mPlanes[3].mNormal = cross(mBottomLeft - mPosition, mTopRight - mTopLeft); // bottom
	//for (int i = 0; i < 4; i++)
	//{
	//	frustum.mPlanes[i].mDistance = distanceToFrustum(frustum.mPlanes[i], mPosition);
	//}
	//mFrustum = frustum;
	//
	//float3 dir = info.mI - mPosition;
	//float const dist = length(dir);  
	//dir = normalize(dir); 
	//
	//Ray shadow = Ray(info.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
	//if (scene.IsOccluded(shadow)) return BLACK;
	//
	//float const cosa = max(0.0f, dot(info.mN, -dir));
	//float const attenuation = 1.0f / (dist * dist);
	//
	//float const dLeft = distanceToFrustum(mFrustum.mPlanes[0], info.mI);
	//float const dRight = distanceToFrustum(mFrustum.mPlanes[1], info.mI);
	//float const dTop = distanceToFrustum(mFrustum.mPlanes[2], info.mI);
	//float const dBottom = distanceToFrustum(mFrustum.mPlanes[3], info.mI);
	//float const x = dLeft / (dLeft + dRight);
	//float const y = dTop / (dTop + dBottom);
	//return x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f ?
	//mTexture.Sample(float2(x, y)) * cosa * attenuation * mStrength : BLACK;
}

lightFunction lightDispatchTable[LIGHT_TYPES_COUNT] =
{
	evaluatePointLight,
	evaluateSpotlight,
	evaluateTexturedSpotlight 
};

color evaluateLight(LightData const& light, int const type, HitInfo const& hit)
{
	return lightDispatchTable[type](light, hit);   
}

LightData::LightData(int const type)  
{
	switch (type)
	{
	case LIGHT_TYPES_POINT_LIGHT:	new (&mPointLight) PointLight2();	break;   
	case LIGHT_TYPES_SPOTLIGHT:		new (&mSpotlight) Spotlight2();		break;  
	default: new (&mPointLight) PointLight2(); break;  
	}
}

Lights::Lights() : 
	mCount(0)
{}

void Lights::Add(int const type)
{
	mData.emplace_back(type);   
	mTypes.emplace_back(type);   
	mCount = mData.size(); 
}

color Lights::Evaluate(HitInfo const& hit) const
{
	color result = BLACK;
	for (int i = 0; i < mCount; i++)
	{
		result += evaluateLight(mData[i], mTypes[i], hit);
	}
	return result;
}

color Lights::EvaluateStochastic(HitInfo const& hit) const 
{
	color result = BLACK; 
	for (int i = 0; i < mSamplesPerPixel; i++)
	{
		uint8_t const randomIdx = RandomUInt() % mCount;  
		result += evaluateLight(mData[randomIdx], mTypes[randomIdx], hit) * static_cast<float>(mCount) / static_cast<float>(mSamplesPerPixel); 
	}
	return result; 
}
