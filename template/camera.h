#pragma once

#include "scene.h" 

float3 const	INIT_CAMERA_POSITION		= float3(0.0f, 0.0f, -2.0f);  
float3 const	INIT_CAMERA_TARGET			= float3(0.0f, 0.0f, -1.0f);
float constexpr INIT_CAMERA_SPEED			= 0.0025f;
float constexpr INIT_CAMERA_FOV				= 50.0f;
float constexpr INIT_CAMERA_DEFOCUS_ANGLE	= 0.02f;
float constexpr INIT_CAMERA_FOCUS_DIST		= 10.0f;
float constexpr INIT_CAMERA_MAX_FOCUS_DIST	= 300.0f;

class Camera
{
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

	float	mSpeed;
	float	mFov; 

public:
						Camera();
	bool				Update(float const dt);
	void				Focus(Scene const& scene); 
	[[nodiscard]] Ray	GetPrimaryRay(float2 const pixel) const;
	[[nodiscard]] Ray	GetPrimaryRay2(float2 const pixel) const;
	[[nodiscard]] Ray	GetPrimaryRayFocused(float2 const pixel) const;

private:
	void				UpdateBasisVectors();
	void				UpdateViewport();
	void				UpdateViewport2();
	void				UpdateDefocusDisk();
};
