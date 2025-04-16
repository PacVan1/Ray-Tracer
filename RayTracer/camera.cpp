#include "precomp.h"
#include "camera.h"

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
	mDefocusAngle(INIT_CAMERA_DEFOCUS_ANGLE),
	mFocusDist(INIT_CAMERA_FOCUS_DIST),
	mSpeed(INIT_CAMERA_SPEED),
	mFov(INIT_CAMERA_FOV)
{
	UpdateBasisVectors();
	UpdateViewport();
	UpdateDefocusDisk();
}

bool Camera::Update(float const dt)
{
	if (!WindowHasFocus()) return false;
	float const speed	= mSpeed * dt;
	bool		changed	= false;
	if (input.IsKeyDown(CONTROLS_MOVE_LEFT))		mPosition -= speed * 2 * mRight,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_RIGHT))		mPosition += speed * 2 * mRight,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_FORWARD))		mPosition += speed * 2 * mAhead,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_BACKWARD))	mPosition -= speed * 2 * mAhead,	changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_UP))			mPosition += speed * 2 * mUp,		changed = true;
	if (input.IsKeyDown(CONTROLS_MOVE_DOWN))		mPosition -= speed * 2 * mUp,		changed = true;
	float3 target = mPosition + mAhead; 
	if (input.IsKeyDown(CONTROLS_LOOK_UP))			target += speed * mUp,				changed = true;
	if (input.IsKeyDown(CONTROLS_LOOK_DOWN))		target -= speed * mUp,				changed = true;
	if (input.IsKeyDown(CONTROLS_LOOK_LEFT))		target -= speed * mRight,			changed = true;
	if (input.IsKeyDown(CONTROLS_LOOK_RIGHT))		target += speed * mRight,			changed = true;
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

Ray Camera::GetPrimaryRay(float2 const pixel) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { mPosition, normalize(target - mPosition) };
}

Ray Camera::GetPrimaryRayFocused(float2 const pixel) const
{
	float const u = (pixel.x + 0.5f) * (1.0f / SCRWIDTH);
	float const v = (pixel.y + 0.5f) * (1.0f / SCRHEIGHT);
	float3 const random = randomUnitOnDisk();
	float3 const point	= mPosition + random.x * mDefocusDiskU + random.y * mDefocusDiskV;
	float3 const target = mTopLeft + u * mViewportU + v * mViewportV;
	return { point, normalize(target - point) };
}

void Camera::SetPosition(float3 const position)
{
	mPosition = position; 
	UpdateBasisVectors();
	UpdateViewport();
	UpdateDefocusDisk();
}

void Camera::SetTarget(float3 const target)
{
	mTarget = target;
	UpdateBasisVectors();
	UpdateViewport();
	UpdateDefocusDisk();
}

void Camera::SetFocusDist(float const focusDist)
{
	mFocusDist = focusDist;
	UpdateViewport(); 
}

void Camera::SetDefocusAngle(float const defocusAngle)
{
	mDefocusAngle = defocusAngle;
	UpdateDefocusDisk();
}

void Camera::SetFov(float const fov)
{
	mFov = fov;
	UpdateViewport(); 
}

void Camera::UpdateBasisVectors()
{
	mAhead	= normalize(mTarget - mPosition);
	mRight	= normalize(cross(float3(0.0f, 1.0f, 0.0f), mAhead));
	mUp		= normalize(cross(mAhead, mRight));
}

void Camera::UpdateViewport()
{
	float const theta = mFov / DEG_OVER_RAD;
	float const height = tan(theta * 0.5f);

	mViewportHeight = 2.0f * height * mFocusDist;
	mViewportWidth	= mViewportHeight * ASPECT_RATIO;
	mViewportU = mViewportWidth * mRight;
	mViewportV = mViewportHeight * -mUp;

	mTopLeft	= mPosition + mFocusDist * mAhead - mViewportU * 0.5f - mViewportV * 0.5f;
	mTopRight	= mTopLeft + mViewportU;
	mBottomLeft = mTopLeft + mViewportU;
}

void Camera::UpdateDefocusDisk()
{
	mDefocusDiskU = mRight * mDefocusAngle; 
	mDefocusDiskV = mUp * mDefocusAngle; 
}
