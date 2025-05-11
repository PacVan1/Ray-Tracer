#pragma once 

#include "scene.h" 

class BVHScene; 

float3 const	INIT_CAMERA_POSITION		= float3(-3.018f, 1.146f, -3.449f);   
float3 const	INIT_CAMERA_TARGET			= float3(-2.530f, 0.931f, -2.603f); 
float constexpr INIT_CAMERA_SPEED			= 0.0025f;
float constexpr INIT_CAMERA_SENSITIVITY		= 0.0025f;
float constexpr INIT_CAMERA_FOV				= 50.0f;
float constexpr INIT_CAMERA_DEFOCUS_RADIUS	= 0.02f;
float constexpr INIT_CAMERA_FOCUS_DIST		= 10.0f;
float constexpr INIT_CAMERA_MAX_FOCUS_DIST	= 300.0f;
float constexpr INIT_CAMERA_MAX_FOV			= 180.0f;

class Camera
{
public:
	float3	mPosition; 
	float3	mTarget;
	float	mSpeed;
	float	mSensitivity; 
	float	mDefocusRadius; 
	float	mFocusDist;
	float	mFov; 

private:
	Frustum mPrevFrustum; 
	float3	mPrevPosition;
	float3	mTopLeft;
	float3	mTopRight;
	float3	mBottomLeft; 
	float3	mViewportU;
	float3	mViewportV;
	float	mViewportWidth; 
	float	mViewportHeight; 
	float3	mAhead;
	float3	mRight; 
	float3	mUp; 
	float3	mDefocusDiskU;
	float3	mDefocusDiskV;

public:
								Camera();
	bool						Update(float const dt);
	void						BuildFrustum(); 
	void						Focus(Scene const& scene); 
	void						Focus(BVHScene const& scene);
	[[nodiscard]] Ray			GenPrimaryRay(float2 const pixel) const;
	[[nodiscard]] Ray			GenPrimaryRayFocused(float2 const pixel) const;
	[[nodiscard]] Ray			GenPrimaryRayFocused(float2 const pixel, blueSeed const seed) const; 
	[[nodiscard]] tinybvh::Ray	GenPrimaryRayTinyBVH(float2 const pixel) const; 
	[[nodiscard]] tinybvh::Ray	GenPrimaryRayFocusedTinyBVH(float2 const pixel) const;
	[[nodiscard]] tinybvh::Ray	GenPrimaryRayFocusedTinyBVH(float2 const pixel, blueSeed const seed) const; 
	void						UpdateBasisVectors();
	void						UpdateViewport();
	void						UpdateDefocusDisk(); 

	void						SetPosition(float3 const position);
	void						SetTarget(float3 const target);
	void						SetDefocusRadius(float const radius);
	void						SetFocusDist(float const dist);
	void						SetFov(float const fov); 

	inline Frustum const&		GetPrevFrustum() const	{ return mPrevFrustum; }
	inline float3 const&		GetPrevPosition() const { return mPrevPosition; }
};
