#pragma once

struct Intersection; 

enum lightTypes : uint8_t
{
	LIGHT_TYPES_POINT, 
	LIGHT_TYPES_DIRECTIONAL, 
	LIGHT_TYPES_SPOT 
};

class PointLight
{
public:
	float3	mPosition;
	float3	mColor;
	float	mStrength;

public:
							PointLight();
	[[nodiscard]] float3	Intensity(Intersection const& hit) const;
};

class DirectionalLight
{
public:
	float3	mDirection;
	float3	mColor;
	float	mStrength;

public:
							DirectionalLight();
	[[nodiscard]] float3	Intensity(Intersection const& hit) const;
};

class Spotlight 
{
public:
	float3	mPosition;
	float3	mLookAt; 
	float3	mDirection;
	float3	mColor;
	float	mStrength;

private: 
	float	mInnerScalar;
	float	mOuterScalar;
	float	mEpsilon;

public:
							Spotlight(); 
	[[nodiscard]] float3	Intensity(Intersection const& hit) const;
	void					DirectionFromLookAt();
};

class TexturedSpotlight 
{
public:
	Texture<color>	mTexture;  
	Frustum			mFrustum; 
	float3			mPosition; 
	float3			mTarget;   
	float			mStrength; 

	float3			mAhead;
	float3			mRight;
	float3			mUp; 
	float			mFocusDist;
	float			mFov; 

	float3			mTopLeft;
	float3			mTopRight;
	float3			mBottomLeft;
	float3			mViewportU;
	float3			mViewportV;
	float			mViewportWidth;
	float			mViewportHeight; 
	float			mAspectRatio; 

public:
						TexturedSpotlight(); 
	[[nodiscard]] color	Intensity(Intersection const& hit) const; 
	void				Update();
};
