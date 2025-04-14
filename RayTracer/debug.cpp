#include "precomp.h"
#include "debug.h"

DebugViewer2D::DebugViewer2D() :
	mTarget(SCRWIDTH, SCRHEIGHT),
	mPosition(0.0f),
	mZoom(INIT_DEBUG_VIEWER_ZOOM), 
	mEvery(INIT_DEBUG_VIEWER_EVERY),
	mSelected(0),
	mSelectionCount(0),
	mRow(INIT_DEBUG_VIEWER_ROW)
{
	mPrimRayColor.x = RED_U;
	mNormalColor.x	= BLUE_U;
	mHitColor.x		= WHITE_U;
	mInsideColor.x	= YELLOW_U;
	mOutsideColor.x = GREEN_U;
	mPrimRayColor.y = ScaleColor(RED_U, 100);
	mNormalColor.y	= ScaleColor(BLUE_U, 100);
	mHitColor.y		= ScaleColor(WHITE_U, 100);
	mInsideColor.y	= ScaleColor(YELLOW_U, 100);
	mOutsideColor.y = ScaleColor(GREEN_U, 100); 
}

void DebugViewer2D::RenderRay(Ray const& ray, float3 const& intersection, float3 const& normal, int const debug, int const bounces)
{
	if (debug == 0) return;

	uint rayColor		= mPrimRayColor.x;
	uint normalColor	= mNormalColor.x;
	uint hitColor		= mHitColor.x; 

	if (bounces > 0)
	{
		if (!isSelected(debug)) return;
		if (isInside(debug))
		{
			if (debug >> 3 & 1) rayColor = 0xff00ff;
			else rayColor = mInsideColor.x;
		}
		else
		{
			rayColor = mOutsideColor.x; 
		}
	}
	else
	{
		if (!isSelected(debug))
		{
			rayColor	= mPrimRayColor.y; 
			normalColor = mNormalColor.y; 
			hitColor	= mHitColor.y; 
		}
	}

	int2 iO = ToPixel(ray.O);
	int2 iI = ToPixel(intersection);
	int2 iN = ToPixel(intersection + normal);
	mTarget.Line(iO.x, iO.y, iI.x, iI.y, rayColor);
	mTarget.Line(iI.x, iI.y, iN.x, iN.y, normalColor);
	mTarget.Bar(iI.x - 1, iI.y - 1, iI.x + 1, iI.y + 1, hitColor); 
}

void DebugViewer2D::Clear()
{
	mTarget.Clear(0);  
}

int2 DebugViewer2D::ToPixel(float3 const& position) const
{
	float2 const coordinate = (float2(position.x, position.z) - mPosition) * mZoom;
	int2 const pixel		= int2(coordinate.x, -coordinate.y) + int2(SCRWIDTH / 2, SCRHEIGHT / 2);
	return pixel;
}
