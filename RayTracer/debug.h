#pragma once

int constexpr	INIT_DEBUG_VIEWER_ROW	= SCRHEIGHT / 2;
int constexpr	INIT_DEBUG_VIEWER_EVERY	= 15;
float constexpr INIT_DEBUG_VIEWER_ZOOM	= 250.0f; 

typedef struct
{
	unsigned int mIsInside		: 1; 
	unsigned int mIsSelected	: 1;
	unsigned int mIsPrimary		: 1;
	unsigned int mPadding		: 1; 
} debug;

class DebugViewer2D
{
public:
	Surface		mTarget;
	float2		mPosition;
	float		mZoom;
	int			mEvery;
	int			mSelected;
	int			mSelectionCount;
	int			mRow;

	uint2		mPrimRayColor;
	uint2		mNormalColor;
	uint2		mHitColor; 
	uint2		mInsideColor;
	uint2		mOutsideColor; 

public:
						DebugViewer2D();
	void				RenderRay(float3 const& origin, float3 const& intersection, float3 const& normal, debug const debug); 
	void				Clear();
	void				SetSelection(); 

private:
	[[nodiscard]] int2	ToPixel(float3 const& position) const;
};
