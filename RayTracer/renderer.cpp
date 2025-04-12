#include "precomp.h"

void Renderer::Init()
{
	mRenderMode = RENDER_MODES_SHADED;  

	// create fp32 rgb pixel buffer to render to
	mAccumulator = (float4*)MALLOC64( SCRWIDTH * SCRHEIGHT * 16 );
	memset( mAccumulator, 0, SCRWIDTH * SCRHEIGHT * 16 );
}

float3 Renderer::Trace( Ray& ray )
{
	mScene.FindNearest( ray );
	if (ray.objIdx == -1) return 0; // or a fancy sky color
	float3 const I = ray.O + ray.t * ray.D;
	float3 const N = mScene.GetNormal( ray.objIdx, I, ray.D );
	float3 const albedo = mScene.GetAlbedo( ray.objIdx, I );

	switch (mRenderMode)
	{
	case RENDER_MODES_NORMALS:
	{
		return (N + 1) * 0.5f;
		break; 
	}
	case RENDER_MODES_DEPTH:
	{
		return 0.1f * float3(ray.t, ray.t, ray.t);
		break;
	}
	case RENDER_MODES_ALBEDO:
	{
		return albedo; 
		break;
	}
	case RENDER_MODES_SHADED: 
	{
		return mDirLight.Intensity(mScene, I, N) * albedo;
		break;
	}
	default: break;
	}
}

void Renderer::Tick( float deltaTime )
{
	// pixel loop
	Timer t;
	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for (int x = 0; x < SCRWIDTH; x++)
		{
			float4 pixel = float4( Trace( mCamera.GetPrimaryRay( (float)x, (float)y ) ), 0 );
			// translate accumulator contents to rgb32 pixels
			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8( &pixel );
			mAccumulator[x + y * SCRWIDTH] = pixel; 
		}
	}
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf( "%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000 );
	// handle user input
	mCamera.HandleInput( deltaTime );
}

void Renderer::UI()
{
	// ray query on mouse
	//Ray r = mCamera.GetPrimaryRay( (float)mousePos.x, (float)mousePos.y );
	//mScene.FindNearest( r );
	//ImGui::Text( "Object id: %i", r.objIdx );
}