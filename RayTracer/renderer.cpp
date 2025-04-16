#include "precomp.h"
#include "renderer.h"

void Renderer::Tick(float deltaTime)
{
#if DEBUG_MODE
	mTimer.reset();
	if (mSpp < mMaxFrames)
	{
		mBreakPixel = input.IsKeyReleased(CONTROLS_BREAK_PIXEL) && mBreakPixelActive;
		float const scale		= 1.0f / static_cast<float>(mSpp++);   
		int			debugRayIdx	= 0;
#pragma omp parallel for schedule(dynamic)
		for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++) 
		{
			int const pixelIdx = x + y * SCRWIDTH; 

			if (mBreakPixel && input.mMousePos.x == x && input.mMousePos.y == y)
			{
				__debugbreak(); 
			}

			switch (mRenderMode)
			{
			case RENDER_MODES_NORMALS:
			{
				float2 const pixelCoord = float2(static_cast<float>(x), static_cast<float>(y));
				Ray primRay = mCamera.GetPrimaryRay(pixelCoord);
				color const pixel = TraceNormals(primRay);  
				mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);
				break;
			}
			case RENDER_MODES_DEPTH:
			{
				float2 const pixelCoord = float2(static_cast<float>(x), static_cast<float>(y));
				Ray primRay = mCamera.GetPrimaryRay(pixelCoord);
				color const pixel = TraceDepth(primRay);
				mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);
				break;
			}
			case RENDER_MODES_ALBEDO:
			{
				float2 const pixelCoord = float2(static_cast<float>(x), static_cast<float>(y));
				Ray primRay = mCamera.GetPrimaryRay(pixelCoord); 
				color const pixel = TraceAlbedo(primRay);
				mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);
				break;
			}
			case RENDER_MODES_SHADED:
			{
				float2 pixelCoord = float2(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
				if (mAaActive) pixelCoord += float2(RandomFloat() - 0.5f, RandomFloat() - 0.5f);
				Ray primRay;
				if (mDofActive) primRay = mCamera.GetPrimaryRayFocused(pixelCoord);
				else			primRay = mCamera.GetPrimaryRay(pixelCoord); 
				color pixel = BLACK;

				if (mDebugViewerActive)
				{
					debug debug = {};
					if (y == mDebugViewer.mRow && x % mDebugViewer.mEvery == 0)
					{
						debug.mIsDebug		= true; debugRayIdx++;
						debug.mIsSelected	= debugRayIdx == mDebugViewer.mSelected;
					}
					pixel = TraceDebug(primRay, debug);
				}
				else
				{
					pixel = Trace(primRay);
				}

				if (mAccumActive)
				{
					mAccumulator[pixelIdx] += pixel; 
					color const average = mAccumulator[pixelIdx] * scale;
					mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(average);
				}
				else
				{
					mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);  
				}

				break;
			}
			default: break; 
			}
		}
	}
	PerformanceReport();

	if (mDebugViewerActive) RenderDebugViewer();
	if (mBreakPixelActive)
	{
		mScreen->Line(static_cast<float>(input.mMousePos.x), 0, static_cast<float>(input.mMousePos.x), SCRHEIGHT - 1, RED_U);
		mScreen->Line(0, static_cast<float>(input.mMousePos.y), SCRWIDTH - 1, static_cast<float>(input.mMousePos.y), RED_U); 
	}
	if (mCamera.Update(deltaTime))
	{
		if (mAutoFocusActive) mCamera.Focus(mScene);  
		ResetAccumulator(); 
	}
#endif
}

void Renderer::SetRenderMode(int const renderMode)
{
	mRenderMode = renderMode;
	ResetAccumulator();
}

void Renderer::SetMaxBounces(int const maxBounces)
{
	mMaxBounces = maxBounces;
	ResetAccumulator(); 
}

void Renderer::SetAa(bool const aaActive)
{
	mAaActive = aaActive;
	ResetAccumulator();
}

void Renderer::SetAccum(bool const accumActive)
{
	mAccumActive = accumActive;
	ResetAccumulator(); 
}

void Renderer::SetAutoFocus(bool const autoFocusActive)
{
	mAutoFocusActive = autoFocusActive;
	mCamera.Focus(mScene);
	ResetAccumulator();
}

void Renderer::SetDof(bool const dofActive)
{
	mDofActive = dofActive;
	ResetAccumulator();
}

color Renderer::Trace(Ray& ray, int const bounces) const
{
	if (bounces >= mMaxBounces) return BLACK;
	mScene.FindNearest( ray );
	if (ray.objIdx == -1) return mHdrTexture.Sample(ray.D); // or a fancy sky color 
	float3 const intersection	= calcIntersection(ray); 
	float3 const normal			= mScene.GetNormal( ray.objIdx, intersection, ray.D );
	color const	 albedo			= mScene.GetAlbedo( ray.objIdx, intersection);

	if (ray.objIdx == mScene.cube.objIdx)
	{
		Ray scattered;
		if (mMetallic.Scatter(ray, scattered, intersection, normal))
		{
			return Trace(scattered, (bounces + 1));
		}
	}
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		Ray scattered;
		if (mDielectric.Scatter(ray, scattered, intersection, normal))
		{
			return Trace(scattered, (bounces + 1));
		}
	}

	return CalcDirectLight(mScene, intersection, normal) * albedo;
}

color Renderer::TraceDebug(Ray& ray, debug debug, int const bounces)
{
	if (bounces >= mMaxBounces) return BLACK;
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return mHdrTexture.Sample(ray.D);
	float3 const intersection = calcIntersection(ray);
	float3 const normal = mScene.GetNormal(ray.objIdx, intersection, ray.D);
	color const	 albedo = mScene.GetAlbedo(ray.objIdx, intersection);

	debug.mIsPrimary	= bounces == 0; 
	debug.mIsInside		= ray.inside;  
	mDebugViewer.RenderRay(ray.O, intersection, normal, debug);   

	if (ray.objIdx == mScene.cube.objIdx)
	{
		Ray scattered;
		if (mMetallic.Scatter(ray, scattered, intersection, normal))
		{
			return TraceDebug(scattered, debug, (bounces + 1));
		}
	}
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		Ray scattered;
		if (mDielectric.Scatter(ray, scattered, intersection, normal))
		{
			return TraceDebug(scattered, debug, (bounces + 1));
		}
	}

	return CalcDirectLight(mScene, intersection, normal) * albedo;
}

color Renderer::TraceNormals(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK;

	float3 const intersection	= calcIntersection(ray);
	float3 const normal			= mScene.GetNormal(ray.objIdx, intersection, ray.D);

	return (normal + 1) * 0.5f;
}

color Renderer::TraceDepth(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK;

	return 0.1f * float3(ray.t, ray.t, ray.t);
}

color Renderer::TraceAlbedo(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK; // or a fancy sky color
	float3 const intersection	= calcIntersection(ray);
	color const	 albedo			= mScene.GetAlbedo(ray.objIdx, intersection);
	return albedo; 
}

color Renderer::CalcDirectLight(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	color result = BLACK;
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

void Renderer::ResetAccumulator()
{
	if (!mAccumActive) return; 

	mSpp = 1; 
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

void Renderer::RenderDebugViewer()
{
	int addr = 0;
	for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++, addr++)
	{
		mScreen->pixels[addr] = ScaleColor(mScreen->pixels[addr], 100);
		mScreen->pixels[addr] = AddBlend(mScreen->pixels[addr], mDebugViewer.mTarget.pixels[addr]); 
	}

	mScreen->Line(0, mDebugViewer.mRow, SCRWIDTH, mDebugViewer.mRow, WHITE_U);  

	mDebugViewer.Clear(); 
}

void Renderer::Init()
{
	InitUi();
	InitAccumulator(); 

	sEps		= INIT_EPS;
	mRenderMode = INIT_RENDER_MODE;
	mMaxBounces = INIT_MAX_BOUNCES;

	mDirLight.mDirection	= float3(0.8f, -0.3f, 0.5f);
	mDirLight.mDirection	= normalize(mDirLight.mDirection); 
	mDirLight.mStrength		= 1.0f;
	mDirLight.mColor		= WHITE;

	mHdrTexture = HdrTexture("../assets/hdr/kloppenheim_06_puresky_4k.hdr");  
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