#include "precomp.h"
#include "camera.h"

#include "bvh_scene.h"

Camera::Camera() :
	mPosition(INIT_CAMERA_POSITION),
	mTarget(INIT_CAMERA_TARGET), 
	mTopLeft(0.0f),
	mTopRight(0.0f),
	mBottomLeft(0.0f),
	mViewportU(0.0f),
	mViewportV(0.0f),
	mViewportWidth(0.0f),
	mViewportHeight(0.0f),
	mAhead(0.0f),
	mRight(0.0f),
	mUp(0.0f),
	mDefocusDiskU(0.0f),
	mDefocusDiskV(0.0f),
	mDefocusRadius(INIT_CAMERA_DEFOCUS_RADIUS),
	mFocusDist(INIT_CAMERA_FOCUS_DIST),
	mSpeed(INIT_CAMERA_SPEED),
	mSensitivity(INIT_CAMERA_SENSITIVITY), 
	mFov(INIT_CAMERA_FOV) 
{
	UpdateBasisVectors();
	UpdateViewport();
	UpdateDefocusDisk();
	BuildFrustum();
}

bool Camera::Update(float const dt)
{
	if (!WindowHasFocus()) return false;
	float const speed	= mSpeed * dt;
	float const sens	= mSensitivity * dt; 
	bool		changed	= false;
	if (input.IsKeyDown(CONTROLS_MOVE_LEFT))		mPosition -= speed * 2 * mRight,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_RIGHT))		mPosition += speed * 2 * mRight,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_FORWARD))		mPosition += speed * 2 * mAhead,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_BACKWARD))	mPosition -= speed * 2 * mAhead,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_UP))			mPosition += speed * 2 * mUp,		changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_DOWN))		mPosition -= speed * 2 * mUp,		changed = true;
	float3 target = mPosition + mAhead; 
	if (input.IsKeyDown(CONTROLS_LOOK_UP))			target += sens * mUp,				changed = true; 
	if (input.IsKeyDown(CONTROLS_LOOK_DOWN))		target -= sens * mUp,				changed = true; 
	if (input.IsKeyDown(CONTROLS_LOOK_LEFT))		target -= sens * mRight,			changed = true; 
	if (input.IsKeyDown(CONTROLS_LOOK_RIGHT))		target += sens * mRight,			changed = true; 
	if (!changed) return false;
	mTarget = target; 
	UpdateBasisVectors();
	UpdateViewport();
	UpdateDefocusDisk();
	return true;
}

void Camera::Focus(Scene const& scene)
{
	Ray ray = { mPosition, mAhead };
	scene.FindNearest(ray);
	mFocusDist = min(ray.t, INIT_CAMERA_MAX_FOCUS_DIST);
	UpdateViewport(); 
}

void Camera::Focus(BVHScene const& scene)
{
	tinybvh::Ray ray = { mPosition, mAhead };
	scene.mTlas.Intersect(ray); 
	mFocusDist = min(ray.hit.t, INIT_CAMERA_MAX_FOCUS_DIST);
	UpdateViewport();
}

Ray Camera::GenPrimaryRay(float2 const pixel) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { mPosition, normalize(target - mPosition) };
}

Ray Camera::GenPrimaryRayFocused(float2 const pixel) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const random = randomUnitOnDisk();
	float3 const point	= mPosition + random.x * mDefocusDiskU + random.y * mDefocusDiskV;
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { point, normalize(target - point) };
}

Ray	Camera::GenPrimaryRayFocused(float2 const pixel, blueSeed const seed) const 
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const random = randomUnitOnDisk(seed);
	float3 const point = mPosition + random.x * mDefocusDiskU + random.y * mDefocusDiskV;
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { point, normalize(target - point) };
}

tinybvh::Ray Camera::GenPrimaryRayTinyBVH(float2 const pixel) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { mPosition, normalize(target - mPosition) };
}

tinybvh::Ray Camera::GenPrimaryRayFocusedTinyBVH(float2 const pixel) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const random = randomUnitOnDisk();
	float3 const point = mPosition + random.x * mDefocusDiskU + random.y * mDefocusDiskV;
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { point, normalize(target - point) };
}

tinybvh::Ray Camera::GenPrimaryRayFocusedTinyBVH(float2 const pixel, blueSeed const seed) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const random = randomUnitOnDisk(seed);
	float3 const point = mPosition + random.x * mDefocusDiskU + random.y * mDefocusDiskV;
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { point, normalize(target - point) }; 
}

void Camera::UpdateBasisVectors()
{
	mAhead	= normalize(mTarget - mPosition);
	mRight	= normalize(cross(float3(0.0f, 1.0f, 0.0f), mAhead));
	mUp		= normalize(cross(mAhead, mRight));
}

void Camera::UpdateViewport()
{
	float const theta	= mFov / DEG_OVER_RAD;
	float const height	= tan(theta * 0.5f);

	mViewportHeight = 2.0f * height * mFocusDist;
	mViewportWidth	= mViewportHeight * ASPECT_RATIO;
	mViewportU = mViewportWidth * mRight;
	mViewportV = mViewportHeight * -mUp;

	mTopLeft	= mPosition + mFocusDist * mAhead - mViewportU * 0.5f - mViewportV * 0.5f; 
	mTopRight	= mTopLeft + mViewportU;
	mBottomLeft = mTopLeft + mViewportV; 
}

void Camera::UpdateDefocusDisk()
{
	mDefocusDiskU = mRight * mDefocusRadius; 
	mDefocusDiskV = mUp * mDefocusRadius; 
}

void Camera::BuildFrustum()
{
	Frustum frustum = {}; 
	frustum.mPlanes[0].mNormal = cross(mTopLeft - mBottomLeft, mTopLeft - mPosition);  // left
	frustum.mPlanes[1].mNormal = cross(mTopRight - mPosition, mTopLeft - mBottomLeft); // right
	frustum.mPlanes[2].mNormal = cross(mTopRight - mTopLeft, mTopLeft - mPosition);    // top
	frustum.mPlanes[3].mNormal = cross(mBottomLeft - mPosition, mTopRight - mTopLeft); // bottom
	for (int i = 0; i < 4; i++)
	{
		frustum.mPlanes[i].mDistance = distanceToFrustum(frustum.mPlanes[i], mPosition);
	}
	mPrevFrustum	= frustum;   
	mPrevPosition	= mPosition;  
}

void Camera::SetPosition(float3 const position)
{
	mPosition = position;
	UpdateBasisVectors(); 
	UpdateDefocusDisk(); 
	UpdateViewport(); 
}

void Camera::SetTarget(float3 const target)
{
	mTarget = target; 
	UpdateBasisVectors(); 
	UpdateDefocusDisk();
	UpdateViewport();
}

void Camera::SetDefocusRadius(float const radius)
{
	mDefocusRadius = radius;
	UpdateDefocusDisk(); 
}

void Camera::SetFocusDist(float const dist)
{
	mFocusDist = dist; 
	UpdateViewport();
}

void Camera::SetFov(float const fov)
{
	mFov = fov;
	UpdateViewport(); 
}
