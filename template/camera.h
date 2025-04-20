#pragma once

#include "scene.h" 

// position 1: 
//float3 const	INIT_CAMERA_POSITION		= float3(-3.018f, 1.146f, -3.449f);  
//float3 const	INIT_CAMERA_TARGET			= float3(-2.530f, 0.931f, -2.603f);
// position 2: 
float3 const	INIT_CAMERA_POSITION		= float3(-1.793f, 1.109f, 6.860f);  
float3 const	INIT_CAMERA_TARGET			= float3( 0.000f, 0.109f, 2.000f);

float constexpr INIT_CAMERA_SPEED			= 0.0025f;
float constexpr INIT_CAMERA_FOV				= 50.0f;
float constexpr INIT_CAMERA_DEFOCUS_ANGLE	= 0.02f;
float constexpr INIT_CAMERA_FOCUS_DIST		= 10.0f;
float constexpr INIT_CAMERA_MAX_FOCUS_DIST	= 300.0f;
float constexpr INIT_CAMERA_MAX_FOV			= 180.0f;

class Camera
{
public:
	float	mSpeed;

private: 
	float3	mPosition;
	float3	mTarget;

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
	float	mDefocusAngle;
	float	mFocusDist; 

	float	mFov; 

public:
						Camera();
	bool				Update(float const dt);
	void				Focus(Scene const& scene); 
	[[nodiscard]] Ray	GetPrimaryRay(float2 const pixel) const;
	[[nodiscard]] Ray	GetPrimaryRayFocused(float2 const pixel) const;

	void				SetPosition(float3 const position);
	void				SetTarget(float3 const target);
	void				SetFocusDist(float const focusDist);
	void				SetDefocusAngle(float const defocusAngle);
	void				SetFov(float const fov); 

	inline float3		GetPosition() const		{ return mPosition; }
	inline float3		GetTarget() const		{ return mTarget; }
	inline float		GetFocusDist() const	{ return mFocusDist; }
	inline float		GetDefocusAngle() const { return mDefocusAngle; }
	inline float		GetFov() const			{ return mFov; }

private:
	void				UpdateBasisVectors();
	void				UpdateViewport();
	void				UpdateDefocusDisk();
};
