#include "precomp.h"
#include "renderer.h"

void Renderer::Tick(float deltaTime)
{
#if DEBUG_MODE
	mTimer.reset();
	if (mFrame < mMaxFrames || !mMaxFramesActive) 
	{
		mBreakPixel = input.IsKeyReleased(CONTROLS_BREAK_PIXEL) && mBreakPixelActive;
		float const scale = 1.0f / static_cast<float>(mSpp++); mFrame++; 
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

				if (mDebugViewerActive && y == mDebugViewer.mRow && x % mDebugViewer.mEvery == 0)
				{
					debug debug = {};
					debugRayIdx++;
					debug.mIsSelected = debugRayIdx == mDebugViewer.mSelected;
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

color Renderer::Trace(Ray& ray) const 
{
	color light			= BLACK; 
	color throughput	= WHITE;
	for (int bounce = 0; bounce < mMaxBounces; bounce++)
	{
		mScene.FindNearest(ray);
		if (DidHit(ray)) 
		{
			HitInfo const info = CalcHitInfo(ray);  

			color albedo	= mScene.GetAlbedo(ray.objIdx, info.mI);  
			color emission	= BLACK;   

			if (info.mMat)
			{
				albedo		= info.mMat->GetAlbedo(); 
				emission	= info.mMat->GetEmission(); 

				Ray		scattered;
				color	attenuation = albedo;  
				if (info.mMat->Scatter(ray, info, scattered, attenuation)) 
				{
					color const directLight		= CalcDirectLight2(mScene, info) * albedo; 
					color const indirectLight	= attenuation; 
					light		+= (directLight + emission) * throughput; 
					throughput	*= indirectLight; // indirect light  
					ray = scattered;
					continue; 
				}
			}

			light += (CalcDirectLightWithArea2(mScene, info) * albedo + emission) * throughput;  
			return light; 
		}
		else
		{
			light += Miss(ray.D) * throughput;
			return light; 
		}
	}
	return light;    
}

color Renderer::TraceDebug(Ray& ray, debug debug)
{
	color light = BLACK;
	color throughput = WHITE;
	for (int bounce = 0; bounce < mMaxBounces; bounce++)
	{
		mScene.FindNearest(ray);
		if (ray.objIdx == -1) return throughput * Miss(ray.D) + light;
		HitInfo const info = CalcHitInfo(ray);

		debug.mIsPrimary	= bounce == 0;
		debug.mIsInside		= ray.inside;
		mDebugViewer.RenderRay(ray.O, info.mI, info.mN, debug);

		Ray		scattered;
		color	attenuation;  
		if (info.mMat->Scatter(ray, info, scattered, attenuation)) 
		{
			color const directLight = CalcDirectLight2(mScene, info) * info.mMat->GetAlbedo();
			light		+= (directLight + info.mMat->GetEmission()) * throughput; 
			throughput	*= attenuation; // indirect light
			ray = scattered;
			continue;
		}

		light += (CalcDirectLightWithArea2(mScene, info) * info.mMat->GetAlbedo() + info.mMat->GetEmission()) * throughput; 
		return light;
	}
	return light;
}

color Renderer::TraceRecursive(Ray& ray, int const bounces) const
{
	if (bounces >= mMaxBounces) return BLACK; 
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return Miss(ray.D);
	HitInfo const info	= CalcHitInfo(ray);
	color albedo		= mScene.GetAlbedo(ray.objIdx, info.mI); 

	if (ray.objIdx == mScene.sphere.objIdx || ray.objIdx == mScene.cube.objIdx || ray.objIdx == mScene.torus.objIdx)
	{
		albedo = mLambertian3.GetAlbedo();
		Ray		scattered;
 		color	scatteredColor;   
		if (mLambertian3.Scatter(ray, info, scattered, scatteredColor)) 
		{
			color const directLight		= CalcDirectLight2(mScene, info) * albedo; 
			color const indirectLight	= TraceRecursive(scattered, bounces + 1) * scatteredColor;
			return indirectLight + directLight;  
		}
	}

	return CalcDirectLightWithArea2(mScene, info) * albedo; 
}

color Renderer::TraceDebugRecursive(Ray& ray, debug debug, int const bounces)
{
	if (bounces >= mMaxBounces) return BLACK;
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return Miss(ray.D); 
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
			return TraceDebugRecursive(scattered, debug, (bounces + 1));
		}
	}
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		Ray scattered;
		if (mDielectric.Scatter(ray, scattered, intersection, normal))
		{
			return TraceDebugRecursive(scattered, debug, (bounces + 1));
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
	if (mDirLightActive)	result += mDirLight.Intensity(scene, intersection, normal);
	if (mPointLightsActive) for (PointLight const& pointLight : mPointLights) result += pointLight.Intensity(scene, intersection, normal);
	if (mSpotLightsActive)	for (SpotLight const& spotLight : mSpotLights) result += spotLight.Intensity(scene, intersection, normal);
	return result; 
}

color Renderer::CalcDirectLight2(Scene const& scene, HitInfo const& info) const
{
	color result = BLACK;
	if (mDirLightActive)	result += mDirLight.Intensity2(scene, info);
	if (mPointLightsActive) for (PointLight const& pointLight : mPointLights) result += pointLight.Intensity2(scene, info);
	if (mSpotLightsActive)	for (SpotLight const& spotLight : mSpotLights) result += spotLight.Intensity2(scene, info);
	return result;
}  

color Renderer::CalcDirectLightWithArea(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	color result = BLACK; 
	result += CalcDirectLight(scene, intersection, normal);
	result += mSkydomeActive ? mSkydome.Intensity(scene, intersection, normal) : MissIntensity(scene, intersection, normal);  
	return result; 
}

color Renderer::CalcDirectLightWithArea2(Scene const& scene, HitInfo const& info) const
{
	color result = BLACK;
	result += CalcDirectLight2(scene, info);

	if (mQuadLightActive) result += CalcQuadLight(scene, info); 
	result += mSkydomeActive ? mSkydome.Intensity2(scene, info) : MissIntensity2(scene, info);

	return result;
}

color Renderer::CalcQuadLight(Scene const& scene, HitInfo const& info) const 
{
	float3 dir			= info.mI - scene.RandomPointOnLight(RandomFloat(), RandomFloat());  
	float const dist	= length(dir); 
	dir					= normalize(dir);   

	Ray shadow = Ray(info.mI - dir * sEps, -dir, dist - sEps);  
	if (scene.IsOccluded(shadow)) return BLACK;  

	float const cosa		= max(0.0f, dot(info.mN, -dir));
	float const attenuation = (1.0f / (dist * dist));
	float const probability = scene.GetLightArea() * scene.GetLightCount();

	return attenuation * cosa * probability;  
}

color Renderer::Miss(float3 const direction) const
{
	return mSkydomeActive ? mSkydome.Sample(direction) : mMiss;   
}

color Renderer::MissIntensity(Scene const& scene, float3 const& intersection, float3 const& normal) const
{
	float3 const random = randomUnitOnHemisphere(normal);
	if (scene.IsOccluded({ intersection + random * sEps, random })) return BLACK;
	float const cosa = max(0.0f, dot(normal, random));
	return cosa * mMiss; 
}

color Renderer::MissIntensity2(Scene const& scene, HitInfo const& info) const
{
	float3 const random = randomUnitOnHemisphere(info.mN); 
	if (scene.IsOccluded({ info.mI + random * sEps, random })) return BLACK;
	float const cosa = max(0.0f, dot(info.mN, random));
	return cosa * mMiss;
}

HitInfo Renderer::CalcHitInfo(Ray const& ray) const
{
	HitInfo info;
	info.mI		= calcIntersection(ray);
	info.mN		= mScene.GetNormal(ray.objIdx, info.mI, ray.D);
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		info.mMat = mSphereMaterial;   
	}
	else if (ray.objIdx == mScene.torus.objIdx)
	{
		info.mMat = mTorusMaterial;
	}
	else if (ray.objIdx == mScene.cube.objIdx)
	{
		info.mMat = mCubeMaterial;
	}
	else if (ray.objIdx == mScene.plane[2].objIdx)  
	{
		info.mMat = mFloorMaterial;  
	}
#ifdef FOURLIGHTS  
	else if (ray.objIdx == mScene.quad[0].objIdx)
	{
		info.mMat = mQuadMaterial; 
	}
#else
	else if (ray.objIdx == mScene.quad.objIdx) 
	{
		info.mMat = mQuadMaterial; 
	}
#endif
	else
	{
		info.mMat = nullptr;  
	}
	return info;
}

void Renderer::ResetAccumulator()
{
	if (!mAccumActive) return; 

	mSpp = 1;
	mFrame = 0; 
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

	mMiss				= INIT_MISS;
	sEps				= INIT_EPS;
	mRenderMode			= INIT_RENDER_MODE;
	mMaxBounces			= INIT_MAX_BOUNCES;

	mDirLightActive		= INIT_LIGHTS_DIR_LIGHT_ACTIVE;
	mPointLightsActive	= INIT_LIGHTS_POINT_LIGHTS_ACTIVE;
	mSpotLightsActive	= INIT_LIGHTS_SPOT_LIGHTS_ACTIVE;
	mSkydomeActive		= INIT_LIGHTS_SKYDOME_ACTIVE;

	mDofActive			= INIT_DOF_ACTIVE; 
	mBreakPixel			= INIT_BREAK_PIXEL;
	mAaActive			= INIT_AA_ACTIVE;
	mAccumActive		= INIT_ACCUM_ACTIVE;
	mAutoFocusActive	= INIT_AUTO_FOCUS_ACTIVE;

	mDirLight.mDirection	= float3(0.8f, -0.3f, 0.5f);
	mDirLight.mDirection	= normalize(mDirLight.mDirection); 
	mDirLight.mStrength		= 1.0f;
	mDirLight.mColor		= WHITE;   

	mSphereMaterial = &mDielectric;        
	mTorusMaterial	= &mDielectric;         
	mCubeMaterial	= &mDielectric;        
	mFloorMaterial	= new Lambertian3();   
	mQuadMaterial	= new Glossy2();
	mQuadMaterial->mAlbedo		= WHITE;
	mQuadMaterial->mEmission	= WHITE;
	//mSphereMaterial->mAlbedo	= RED;  
	//mSphereMaterial->mEmission	= RED * 2.0f;       

	mSkydome = Skydome("../assets/hdr/kloppenheim_06_puresky_4k.hdr");  
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

bool Renderer::DidHit(Ray const& ray) const  
{
	return ray.objIdx != -1; 
}

void Renderer::Shutdown()
{
	delete[] mAccumulator;  
}