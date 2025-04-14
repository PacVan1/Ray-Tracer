#pragma once

#include "scene.h" 

int constexpr	INIT_DEBUG_VIEWER_ROW	= SCRHEIGHT / 2;
int constexpr	INIT_DEBUG_VIEWER_EVERY	= 15;
float constexpr INIT_DEBUG_VIEWER_ZOOM	= 100.0f; 

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
	void				RenderRay(Ray const& ray, float3 const& intersection, float3 const& normal, int const debug, int const bounces);
	void				Clear(); 

private:
	[[nodiscard]] int2	ToPixel(float3 const& position) const;
};

inline void				setSelected(int& debug)		{ debug |= (1 << 1); }
inline void				setInside(int& debug)		{ debug |= (1 << 2); }
inline void				setOutside(int& debug)		{ debug &= ~(1 << 2); }
inline bool				isSelected(int const debug) { return debug >> 1 & 1; }
inline bool				isInside(int const debug)	{ return debug >> 2 & 1; }

