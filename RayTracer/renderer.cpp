#include "precomp.h"
#include "renderer.h"

void Renderer::Tick( float deltaTime )
{
	mTimer.reset(); 
	
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		for (int x = 0; x < SCRWIDTH; x++)
		{
			float4 pixel = float4( Trace( mCamera.GetPrimaryRay( (float)x, (float)y ) ), 0 );
			
			mScreen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8( &pixel );
			mAccumulator[x + y * SCRWIDTH] = pixel; 
		}
	}

	PerformanceReport();

	mCamera.HandleInput( deltaTime );
}

void Renderer::SetRenderMode(int const renderMode)
{
	mRenderMode = renderMode;
	ResetAccumulator();
}

float3 Renderer::Trace( Ray& ray ) const
{
	mScene.FindNearest( ray );
	if (ray.objIdx == -1) return BLACK; // or a fancy sky color
	float3 const intersection = calcIntersection(ray); 
	float3 const normal = mScene.GetNormal( ray.objIdx, intersection, ray.D );
	float3 const albedo = mScene.GetAlbedo( ray.objIdx, intersection);

	switch (mRenderMode)
	{
	case RENDER_MODES_NORMALS:
	{
		return (normal + 1) * 0.5f; 
	}
	case RENDER_MODES_DEPTH:
	{
		return 0.1f * float3(ray.t, ray.t, ray.t);
	}
	case RENDER_MODES_ALBEDO:
	{
		return albedo; 
	}
	case RENDER_MODES_SHADED: 
	{
		return CalcDirectLight(mScene, intersection, normal) * albedo;
	}
	default: return BLACK;
	}
}

float3 Renderer::CalcDirectLight(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 result = BLACK;  
	result += mDirLight.Intensity(scene, intersection, normal);
	for (PointLight const& pointLight : mPointLights)
	{
		result += pointLight.Intensity(scene, intersection, normal); 
	}
	for (SpotLight const& spotLight : mSpotLights)
	{
		result += spotLight.Intensity(scene, intersection, normal);
	}
	return result; 
}

void Renderer::ResetAccumulator() const
{
	memset(mAccumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float4));
}

void Renderer::PerformanceReport()
{
	mAvg = (1 - mAlpha) * mAvg + mAlpha * mTimer.elapsed() * 1000;
	if (mAlpha > 0.05f) mAlpha *= 0.5f;
	mFps = 1000.0f / mAvg;
	mRps = (SCRWIDTH * SCRHEIGHT) / mAvg; 
}

void Renderer::UI()
{
	mUi.General();
}

void Renderer::Init()
{
	InitUi();
	InitAccumulator(); 

	mRenderMode = INIT_RENDER_MODE; 
	sEps		= INIT_EPS;

	mDirLight.mDirection	= float3(1.0f, -0.3f, 0.8f);
	mDirLight.mDirection	= normalize(mDirLight.mDirection); 
	mDirLight.mStrength		= 4.0f;
	mDirLight.mColor		= WHITE; 
}

inline void Renderer::InitUi()
{
	mUi.mRenderer = this; 
}

inline void Renderer::InitAccumulator()
{
	mAccumulator = static_cast<float4*>(MALLOC64(SCRWIDTH * SCRHEIGHT * sizeof(float4)));
	memset(mAccumulator, 0, SCRWIDTH * SCRHEIGHT * sizeof(float4)); 
}

void Renderer::Shutdown()
{
	delete[] mAccumulator;  
}

float3 calcIntersection(Ray const& ray)
{
	return ray.O + ray.D * ray.t; 
}