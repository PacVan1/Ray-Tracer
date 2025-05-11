#include "precomp.h"
#include "lights.h"

#include "renderer.h" 
#include "bvh_scene.h" 

PointLight::PointLight() :
	mPosition(0.0f),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 PointLight::Intensity(Intersection const& hit) const 
{
	float3 dir			= hit.point - mPosition; 
	float const dist	= length(dir);
	dir					= normalize(dir);

	Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
	if (hit.scene->IsOccluded(shadow)) return BLACK;

	float const cosa		= max(0.0f, dot(hit.normal, -dir)); 
	float const attenuation = 1.0f / (dist * dist);

	return cosa * attenuation * mColor * mStrength;
}

color PointLight::Intensity(BVHScene const& scene, tinybvh::Ray const& ray) const 
{
	float3 dir = ray.hit.point - mPosition;  
	float const dist = length(dir); 
	dir = normalize(dir); 

	tinybvh::Ray shadow = tinybvh::Ray(ray.hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);  
	if (scene.IsOccluded(shadow)) return BLACK;

	float const cosa = max(0.0f, dot(ray.hit.normal, -dir));
	float const attenuation = 1.0f / (dist * dist);
	return cosa * attenuation * mColor * mStrength;
}

DirectionalLight::DirectionalLight() :
	mDirection(normalize(float3(1.0f))),
	mColor(1.0f),
	mStrength(1.0f)
{}

float3 DirectionalLight::Intensity(Intersection const& hit) const
{
	if (hit.scene->IsOccluded({ hit.point + -mDirection * Renderer::sEps, -mDirection })) return BLACK;

	float const cosa = max(0.0f, dot(hit.normal, -mDirection)); 
	return cosa * mColor * mStrength;
}

color DirectionalLight::Intensity(BVHScene const& scene, tinybvh::Ray const& ray) const 
{
	if (scene.IsOccluded({ ray.hit.point + -mDirection * Renderer::sEps, -mDirection })) return BLACK;
	float const cosa = max(0.0f, dot(ray.hit.normal, -mDirection));
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

float3 Spotlight::Intensity(Intersection const& hit) const 
{
	float3 dir = mPosition - hit.point; 
	float const dist = length(dir);
	dir = normalize(dir);

	if (hit.scene->IsOccluded({ hit.point + -dir * Renderer::sEps, -dir, dist })) return BLACK; 

	float const theta = dot(dir, -mDirection);
	float const intensity = clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);

	float const cosa = max(0.0f, dot(hit.normal, dir));
	float const attenuation = 1.0f / (dist * dist);
	return attenuation * cosa * intensity * mColor * mStrength;
}

color Spotlight::Intensity(BVHScene const& scene, tinybvh::Ray const& ray) const
{
	float3 dir = mPosition - ray.hit.point;  
	float const dist = length(dir); 
	dir = normalize(dir);

	if (scene.IsOccluded({ ray.hit.point + -dir * Renderer::sEps, -dir, dist })) return BLACK;

	float const theta		= dot(dir, -mDirection);
	float const intensity	= clamp((theta - mOuterScalar) / mEpsilon, 0.0f, 1.0f);
	float const cosa		= max(0.0f, dot(ray.hit.normal, dir)); 
	float const attenuation = 1.0f / (dist * dist);
	return attenuation * cosa * intensity * mColor * mStrength;
}

void Spotlight::DirectionFromLookAt()
{
	mDirection = normalize(mLookAt - mPosition);  
}

color TexturedSpotlight::Intensity(Intersection const& hit) const
{
	float3 dir = hit.point - mPosition; 
	float const dist = length(dir);
	dir = normalize(dir);

	Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
	if (hit.scene->IsOccluded(shadow)) return BLACK;  

	float const cosa		= max(0.0f, dot(hit.normal, -dir));
	float const attenuation = 1.0f / (dist * dist);

	float const dLeft	= distanceToFrustum(mFrustum.mPlanes[0], hit.point); 
	float const dRight	= distanceToFrustum(mFrustum.mPlanes[1], hit.point); 
	float const dTop	= distanceToFrustum(mFrustum.mPlanes[2], hit.point); 
	float const dBottom = distanceToFrustum(mFrustum.mPlanes[3], hit.point); 
	float const x		= dLeft / (dLeft + dRight);   
	float const y		= dTop / (dTop + dBottom);   
	return x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f ? 
		mTexture.Sample(float2(x, y)) * cosa * attenuation * mStrength : BLACK;     
}

color TexturedSpotlight::Intensity(BVHScene const& scene, tinybvh::Ray const& ray) const 
{
	float3 dir = ray.hit.point - mPosition;
	float const dist = length(dir);
	dir = normalize(dir);
	
	if (scene.IsOccluded({ ray.hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps })) return BLACK;

	float const cosa = max(0.0f, dot(ray.hit.normal, -dir)); 
	float const attenuation = 1.0f / (dist * dist);

	float const dLeft	= distanceToFrustum(mFrustum.mPlanes[0], ray.hit.point);
	float const dRight	= distanceToFrustum(mFrustum.mPlanes[1], ray.hit.point);
	float const dTop	= distanceToFrustum(mFrustum.mPlanes[2], ray.hit.point); 
	float const dBottom = distanceToFrustum(mFrustum.mPlanes[3], ray.hit.point); 
	float const x = dLeft / (dLeft + dRight);
	float const y = dTop / (dTop + dBottom); 
	return x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f ?
		mTexture.Sample(float2(x, y)) * cosa * attenuation * mStrength : BLACK;
}

TexturedSpotlight::TexturedSpotlight() : 
	//mTexture(loadTextureF("../assets/mario_bros_1.jpeg")),  
	mTexture(loadTextureI("../assets/LDR_RG01_0.png")),    
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
