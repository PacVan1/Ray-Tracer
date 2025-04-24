#include "precomp.h"
#include "lights.h"

#include "Renderer.h" 

PointLight::PointLight() :
	mPosition(0.0f),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 PointLight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 dir			= mPosition - intersection; 
	float const dist	= length(dir);
	dir					= normalize(dir); 

	if (scene.IsOccluded({ intersection + -dir * Renderer::sEps, -dir, dist })) return BLACK;  

	float const cosa		= max(0.0f, dot(normal, dir));
	float const attenuation = 1.0f / (dist * dist); 

	return cosa * attenuation * mColor * mStrength; 
}

float3 PointLight::Intensity2(Scene const& scene, HitInfo const& info) const
{
	float3 dir			= info.mI - mPosition; 
	float const dist	= length(dir);
	dir					= normalize(dir);

	Ray shadow = Ray(info.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps);    
	if (scene.IsOccluded(shadow)) return BLACK;  

	float const cosa		= max(0.0f, dot(info.mN, -dir)); 
	float const attenuation = 1.0f / (dist * dist);

	return cosa * attenuation * mColor * mStrength;
}

DirectionalLight::DirectionalLight() :
	mDirection(normalize(float3(1.0f))),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 DirectionalLight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	if (scene.IsOccluded({ intersection + -mDirection * Renderer::sEps, -mDirection })) return BLACK;

	float const cosa = max(0.0f, dot(normal, -mDirection));
	return cosa * mColor * mStrength;
}

float3 DirectionalLight::Intensity2(Scene const& scene, HitInfo const& info) const
{
	if (scene.IsOccluded({ info.mI + -mDirection * Renderer::sEps, -mDirection })) return BLACK;

	float const cosa = max(0.0f, dot(info.mN, -mDirection));
	return cosa * mColor * mStrength;
}

Spotlight::Spotlight() : 
	mPosition(0.0f, 1.0f, 0.0f),
	mLookAt(0.0f),
	mDirection(0.0f),
	mColor(1.0f),
	mStrength(1.0f),
	mInnerScalar(cos(20.0f / DEG_OVER_RAD)), 
	mOuterScalar(cos(22.5f / DEG_OVER_RAD)),
	mEpsilon(mInnerScalar - mOuterScalar)
{
	DirectionFromLookAt(); 
}

float3 Spotlight::Intensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 dir			= mPosition - intersection; 
	float const dist	= length(dir);
	dir					= normalize(dir);

	if (scene.IsOccluded({ intersection + -dir * Renderer::sEps, -dir, dist })) return BLACK;

	float const theta		= dot(dir, -mDirection);
	float const intensity	= clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);
  
	float const cosa			= max(0.0f, dot(normal , dir)); 
	float const attenuation		= 1.0f / (dist * dist); 

	return attenuation * cosa * intensity * mColor * mStrength;
}

float3 Spotlight::Intensity2(Scene const& scene, HitInfo const& info) const
{
	float3 dir = mPosition - info.mI; 
	float const dist = length(dir);
	dir = normalize(dir);

	if (scene.IsOccluded({ info.mI + -dir * Renderer::sEps, -dir, dist })) return BLACK; 

	float const theta = dot(dir, -mDirection);
	float const intensity = clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);

	float const cosa = max(0.0f, dot(info.mN, dir));
	float const attenuation = 1.0f / (dist * dist);

	return attenuation * cosa * intensity * mColor * mStrength;
}

void Spotlight::DirectionFromLookAt()
{
	mDirection = normalize(mLookAt - mPosition);  
}

color TexturedSpotlight::Intensity(Scene const& scene, HitInfo const& info) const
{
	float3 dir = info.mI - mPosition;
	float const dist = length(dir);
	dir = normalize(dir);

	Ray shadow = Ray(info.mI - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	if (scene.IsOccluded(shadow)) return BLACK; 

	float const cosa		= max(0.0f, dot(info.mN, -dir));
	float const attenuation = 1.0f / (dist * dist);

	float const dLeft	= distanceToFrustum(mFrustum.mPlanes[0], info.mI); 
	float const dRight	= distanceToFrustum(mFrustum.mPlanes[1], info.mI);
	float const dTop	= distanceToFrustum(mFrustum.mPlanes[2], info.mI);
	float const dBottom = distanceToFrustum(mFrustum.mPlanes[3], info.mI); 
	float const x		= dLeft / (dLeft + dRight);   
	float const y		= dTop / (dTop + dBottom);   
	return x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f ? 
		mTexture.Sample(float2(x, y)) * cosa * attenuation * mStrength : BLACK;     
}

TexturedSpotlight::TexturedSpotlight() : 
	mTexture(loadTexture("../assets/mario_bros_1.jpeg")), 
	mPosition(0.0f, 4.0f, -2.5f), 
	mTarget(0.0f), 
	mFov(60.0f),  
	mFocusDist(2.0f), 
	mAspectRatio(mTexture.mWidth / static_cast<float>(mTexture.mHeight)), 
	mStrength(35.0f)   
{
	Update();  
}

void TexturedSpotlight::Update()
{
	// update basis vectors and viewport:
	mAhead	= normalize(mTarget - mPosition); 
	mRight	= normalize(cross(float3(0.0f, 1.0f, 0.0f), mAhead));
	mUp		= normalize(cross(mAhead, mRight)); 

	float const theta	= mFov / DEG_OVER_RAD; 
	float const height	= tan(theta * 0.5f);

	mViewportHeight = 2.0f * height * mFocusDist;
	mViewportWidth	= mViewportHeight * mAspectRatio; 
	mViewportU = mViewportWidth * mRight;
	mViewportV = mViewportHeight * -mUp;

	mTopLeft	= mPosition + mFocusDist * mAhead - mViewportU * 0.5f - mViewportV * 0.5f;
	mTopRight	= mTopLeft + mViewportU;
	mBottomLeft = mTopLeft + mViewportV;

	// update frustum:
	Frustum frustum = {};
	frustum.mPlanes[0].mNormal = cross(mTopLeft - mBottomLeft, mTopLeft - mPosition);  // left 
	frustum.mPlanes[1].mNormal = cross(mTopRight - mPosition, mTopLeft - mBottomLeft); // right
	frustum.mPlanes[2].mNormal = cross(mTopRight - mTopLeft, mTopLeft - mPosition);    // top
	frustum.mPlanes[3].mNormal = cross(mBottomLeft - mPosition, mTopRight - mTopLeft); // bottom
	for (int i = 0; i < 4; i++)
	{
		frustum.mPlanes[i].mDistance = distanceToFrustum(frustum.mPlanes[i], mPosition);
	}
	mFrustum = frustum;
}
