#include "precomp.h"
#include "renderer.h"

#include "lights_stochastic.h" 
#include "lights_vectorized.h" 

Lights lights;    
Lights2 lights2;    
NewLights lights3; 

void Renderer::Tick(float deltaTime)
{
#if DEBUG_MODE
	mTimer.reset();

	//if (mAnimating) mScene.SetTime(mAnimTime += deltaTime * 0.002f);   

	if (mFrame < mSet.mMaxFrames || !mSet.mMaxFramesEnabled) 
	{
		float const scale = 1.0f / static_cast<float>(mSpp++);  
		int			debugRayIdx	= 0;
#pragma omp parallel for schedule(dynamic)
		for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++) 
		{
			int const pixelIdx = x + y * SCRWIDTH; 
			blueSeed seed = { x, y, mFrame };  

			if (mBreakPixel && input.mMousePos.x == x && input.mMousePos.y == y)
			{
				__debugbreak(); 
			}

			switch (mSet.mRenderMode) 
			{
			case RENDER_MODES_NORMALS:
			{
				float2 const pixelCoord = float2(static_cast<float>(x), static_cast<float>(y));
				Ray primRay = mCamera.GenPrimaryRay(pixelCoord);
				tinybvh::Ray primRay2 = { primRay.O, primRay.D };
				color const pixel = TraceNormals(primRay2);  
				mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);
				break;
			}
			case RENDER_MODES_DEPTH:
			{
				float2 const pixelCoord = float2(static_cast<float>(x), static_cast<float>(y));
				Ray primRay = mCamera.GenPrimaryRay(pixelCoord);
				tinybvh::Ray primRay2 = { primRay.O, primRay.D };
				color const pixel = TraceDepth(primRay2); 
				mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);
				break;
			}
			case RENDER_MODES_ALBEDO:
			{
				float2 const pixelCoord = float2(static_cast<float>(x), static_cast<float>(y));
				Ray primRay = mCamera.GenPrimaryRay(pixelCoord); 
				tinybvh::Ray primRay2 = { primRay.O, primRay.D }; 
				color const pixel = TraceAlbedo(primRay2);  
				mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(pixel);
				break;
			}
			case RENDER_MODES_SHADED:
			{
				switch (mSet.mConvergeMode)  
				{
				case CONVERGE_MODES_ACCUMULATION:
				{
					float2 const pixelCoord		= mSet.mAaEnabled ? mSet.mBlueNoiseEnabled ? RandomOnPixel(seed) : RandomOnPixel(x, y) : CenterOfPixel(x, y);
					Ray primRay					= mSet.mDofEnabled ? mSet.mBlueNoiseEnabled ? mCamera.GenPrimaryRayFocused(pixelCoord, seed) : mCamera.GenPrimaryRayFocused(pixelCoord) : mCamera.GenPrimaryRay(pixelCoord);    
					tinybvh::Ray primRay2 = { primRay.O, primRay.D };  
					color const pixel = Trace(primRay2);  
					//mAccumulator[pixelIdx]		+= mSet.mBlueNoiseEnabled ? Trace(primRay, seed) : Trace(primRay); 
					mAccumulator[pixelIdx] += pixel; 
					color const average			= mAccumulator[pixelIdx] * scale;
					mScreen->pixels[pixelIdx]	= RGBF32_to_RGB8(average);
					break;
				}
				case CONVERGE_MODES_REPROJECTION:
				{
					float2 const pixelCoord		= CenterOfPixel(x, y);  
					Ray primRay					= mCamera.GenPrimaryRay(pixelCoord);    
					color const sample			= mSet.mBlueNoiseEnabled ? Trace(primRay, seed) : Trace(primRay); 
					color const reprojected		= Reproject(primRay, sample); 
					mAccumulator[pixelIdx]		= reprojected; 
					mScreen->pixels[pixelIdx]	= RGBF32_to_RGB8(reprojected); 
					break; 
				}
				default:
				{
					float2 const pixelCoord = mSet.mAaEnabled ? mSet.mBlueNoiseEnabled ? RandomOnPixel(seed) : RandomOnPixel(x, y) : CenterOfPixel(x, y); 
					Ray primRay				= mSet.mDofEnabled ? mSet.mBlueNoiseEnabled ? mCamera.GenPrimaryRayFocused(pixelCoord, seed) : mCamera.GenPrimaryRayFocused(pixelCoord) : mCamera.GenPrimaryRay(pixelCoord);
					color sample			= BLACK; 

					if (mSet.mDebugViewerEnabled && y == mDebugViewer.mRow && x % mDebugViewer.mEvery == 0)
					{
						debug debug = {};
						debugRayIdx++;
						debug.mIsSelected = debugRayIdx == mDebugViewer.mSelected;
						sample = TraceDebug(primRay, debug);
					}
					else
					{  
						sample = mSet.mBlueNoiseEnabled ? Trace(primRay, seed) : Trace(primRay); 
					}

					mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(sample);
					break;
				}
				}

				break;
			}
			default: break; 
			}
		}  
		mFrame++; 
	}

	if (mSet.mDebugViewerEnabled) RenderDebugViewer();
	if (mSet.mBreakPixelEnabled)
	{
		mScreen->Line(static_cast<float>(input.mMousePos.x), 0, static_cast<float>(input.mMousePos.x), SCRHEIGHT - 1, RED_U);
		mScreen->Line(0, static_cast<float>(input.mMousePos.y), SCRWIDTH - 1, static_cast<float>(input.mMousePos.y), RED_U); 
	}

	if (mSet.mConvergeMode == CONVERGE_MODES_REPROJECTION) 
	{
		mCamera.BuildFrustum();  
		mCamera.Update(deltaTime); 
		swap(mHistory, mAccumulator);   

	}
	else if (mSet.mConvergeMode == CONVERGE_MODES_ACCUMULATION)
	{
		if (mCamera.Update(deltaTime))
		{
			//if (mSet.mAutoFocusEnabled) mCamera.Focus(mScene); 
			if (mSet.mAutoFocusEnabled) mCamera.Focus(mBVHScene); 
			ResetAccumulator(); 
		}
	}
	else
	{
		if (mCamera.Update(deltaTime))
		{
			if (mSet.mAutoFocusEnabled) mCamera.Focus(mScene);
		}
	}
	PerformanceReport();

	if (mAnimate)
	{
		mCamera.SetPosition(mSplineAnimator.interpNode.position);  
		mCamera.SetTarget(mSplineAnimator.interpNode.target);  
		mSet.mSkydomeEnabled = mSplineAnimator.interpNode.skydomeEnabled; 
		ResetAccumulator();  
		if (mSplineAnimator.Play(deltaTime))
		{
			mAnimate = false;
		}
	}

#else
	mTimer.reset();
	float const scale = 1.0f / static_cast<float>(mSpp++); 
	#pragma omp parallel for schedule(dynamic) 
	for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++)
	{
		int const	pixelIdx = x + y * SCRWIDTH;
		mAccumulator[pixelIdx] += Trace(mCamera.GetPrimaryRayFocused(RandomOnPixel(x, y))); 
		mScreen->pixels[pixelIdx] = RGBF32_to_RGB8(mAccumulator[pixelIdx] * scale); 
	}
	if (mCamera.Update(deltaTime))
	{
		mCamera.Focus(mScene); 
		ResetAccumulator();  
	}
	PerformanceReport();
#endif
}

color Renderer::Trace(Ray& primRay) const
{
	color	light		= BLACK; 
	color	throughput	= WHITE; 

	mScene.FindNearest(primRay);  
	Ray ray = primRay;  

	for (int bounce = 0; bounce < mSet.mMaxBounces; bounce++) 
	{
		if (DidHit(ray))
		{
			Intersection const hit = CalcIntersection(ray);  

			color albedo	= hit.mat->GetAlbedo();    
			color emission	= hit.mat->GetEmission();   

			Ray		scattered;
			color	indirect = albedo; 
			if (hit.mat->Scatter(hit, scattered, indirect))  
			{
				color const direct = CalcDirectLight(hit) * albedo;  
				light += (direct + emission) * throughput;  
				throughput *= indirect;   
				ray = scattered;  
				mScene.FindNearest(ray); 
				continue;
			}

			light += (CalcDirectLightWithArea(hit) * albedo + emission) * throughput;  
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

color Renderer::Trace(tinybvh::Ray& primRay)		
{
	color	light		= BLACK;
	color	throughput	= WHITE;
	if (!mBVHScene.Intersect(primRay)) return Miss(primRay.D);  
	tinybvh::Ray ray = primRay;
	for (int bounce = 0; bounce < mSet.mMaxBounces; bounce++) 
	{
		float const emissivity = ray.hit.mat->emissivity;
		tinybvh::Ray scattered;
		color indirect = ray.hit.albedo; 
		if (scatter(*ray.hit.mat, ray, scattered, indirect))    
		{ 
			//if (dot(ray.hit.normal, ray.D) < 0.0f) indirect *= expf(-0.06f * ray.hit.t); 
			color const direct = CalcDirectLight(ray) * ray.hit.albedo;  
			light += (direct + ray.hit.albedo * emissivity) * throughput;
			throughput *= indirect; ray = scattered;
			if (!mBVHScene.Intersect(ray)) return light + Miss(ray.D) * throughput; 
			continue; 
		}
		return light + (CalcDirectLightWithArea(ray) * ray.hit.albedo + ray.hit.albedo * emissivity) * throughput; 
	}
	return light;
}

color Renderer::Trace(Ray& primRay, blueSeed& seed) const    
{
	color	light		= BLACK;
	color	throughput	= WHITE;

	mScene.FindNearest(primRay);
	Ray ray = primRay;

	for (int bounce = 0; bounce < mSet.mMaxBounces; bounce++)
	{
		if (DidHit(ray))
		{
			Intersection const hit = CalcIntersection(ray);

			seed.mBounce = bounce; 

			color albedo	= hit.mat->GetAlbedo();
			color emission	= hit.mat->GetEmission(); 

			Ray		scattered;
			color	indirect = albedo;
			if (hit.mat->Scatter(hit, seed, scattered, indirect))
			{
				color const direct = CalcDirectLight(hit) * albedo;   
				light += (direct + emission) * throughput; 
				throughput *= indirect;   
				ray = scattered;
				mScene.FindNearest(ray);
				continue;
			}

			light += (CalcDirectLightWithArea(hit, seed) * albedo + emission) * throughput;  
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
	color light			= BLACK;
	color throughput	= WHITE;
	for (int bounce = 0; bounce < mSet.mMaxBounces; bounce++) 
	{
		mScene.FindNearest(ray);
		if (ray.objIdx == -1) return throughput * Miss(ray.D) + light;
		Intersection const hit = CalcIntersection(ray);

		debug.mIsPrimary	= bounce == 0;
		debug.mIsInside		= ray.inside;
		mDebugViewer.RenderRay(ray.O, hit.point, hit.normal, debug);  

		Ray		scattered;
		color	attenuation;  
		if (hit.mat->Scatter(hit, scattered, attenuation))   
		{
			color const directLight = CalcDirectLight(hit) * WHITE;  
			light		+= (directLight + BLACK * throughput);  
			throughput	*= attenuation; // indirect light
			ray = scattered;
			continue;
		}

		light += (CalcDirectLightWithArea(hit) * WHITE) * throughput;    
		return light;
	}
	return light;
}

color Renderer::TraceNormals(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK;

	float3 const intersection	= calcIntersectionPoint(ray); 
	float3 const normal			= mScene.GetNormal(ray.objIdx, intersection, ray.D);

	return (normal + 1.0f) * 0.5f;
}
 
color Renderer::TraceNormals(tinybvh::Ray& ray)
{
	if (!mBVHScene.Intersect(ray)) return Miss(ray.D);
	return (ray.hit.normal + 1.0f) * 0.5f;  
}

color Renderer::TraceDepth(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK;

	return 0.1f * float3(ray.t, ray.t, ray.t);
}

color Renderer::TraceDepth(tinybvh::Ray& ray) const 
{
	mBVHScene.mTlas.Intersect(ray); 
	if (!DidHit(ray)) return Miss(ray.D); 

	return 0.01f * float3(ray.hit.t, ray.hit.t, ray.hit.t); 
}

color Renderer::TraceAlbedo(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (!DidHit(ray)) return Miss(ray.D); // or a fancy sky color  

	float3 const intersection	= calcIntersectionPoint(ray); 
	color const	 albedo			= mScene.GetAlbedo(ray.objIdx, intersection);

	return albedo; 
}

color Renderer::TraceAlbedo(tinybvh::Ray& ray) 
{
	if (!mBVHScene.Intersect(ray)) return Miss(ray.D);
	return ray.hit.albedo; 
}

color Renderer::CalcDirectLight(Intersection const& hit) const
{
	color result = BLACK;
	if (mSet.mDirLightEnabled)	result += mDirLight.Intensity(hit);
	if (mSet.mTexturedSpotlightEnabled) result += mTexturedSpotlight.Intensity(hit); 
	//result += mSet.mStochasticLights ? lights.EvaluateStochastic(info) : lights.Evaluate(info); 
	//result += lights3.Evaluate(info);  
	return result;
}  

color Renderer::CalcDirectLight(tinybvh::Ray const& ray) const
{
	color result = BLACK; 
	if (mSet.mDirLightEnabled)	result += mDirLight.Intensity(mBVHScene, ray); 
	if (mSet.mTexturedSpotlightEnabled) result += mTexturedSpotlight.Intensity(mBVHScene, ray); 

	return result; 
}

color Renderer::CalcDirectLightWithArea(Intersection const& hit) const
{
	color result = BLACK;
	result += CalcDirectLight(hit);  

	if (mSet.mQuadLightEnabled) result += CalcQuadLight(hit);
	result += mSet.mSkydomeEnabled ? mSkydome.Intensity(hit) : MissIntensity(hit);   

	return result;
} 

color Renderer::CalcDirectLightWithArea(tinybvh::Ray const& ray) const
{
	color result = BLACK;
	result += CalcDirectLight(ray); 

	//if (mSet.mQuadLightEnabled) result += CalcQuadLight(hit);
	result += mSet.mSkydomeEnabled ? mSkydome.Intensity(mBVHScene, ray) : MissIntensity(ray); 

	return result;
}

color Renderer::CalcDirectLightWithArea(Intersection const& hit, blueSeed const seed) const  
{
	color result = BLACK;
	result += CalcDirectLight(hit); 

	if (mSet.mQuadLightEnabled) result += CalcQuadLight(hit, seed); 
	result += mSet.mSkydomeEnabled ? mSkydome.Intensity(hit, seed) : MissIntensity(hit);   

	return result;
}

color Renderer::CalcQuadLight(Intersection const& hit) const 
{
	float3 dir			= hit.point - hit.scene->RandomPointOnLight(RandomFloat(), RandomFloat());   
	float const dist	= length(dir); 
	dir					= normalize(dir);   

	Ray shadow = Ray(hit.point - dir * sEps, -dir, dist - sEps);
	if (hit.scene->IsOccluded(shadow)) return BLACK;

	float const cosa		= max(0.0f, dot(hit.normal, -dir));  
	float const attenuation = (1.0f / (dist * dist));
	float const probability = hit.scene->GetLightArea() * hit.scene->GetLightCount(); 

	return attenuation * cosa * probability;  
}

color Renderer::CalcQuadLight(Intersection const& hit, blueSeed const seed) const  
{
	float2 const noise = BlueNoise::GetInstance().Float2(seed);
	float3 dir = hit.point - hit.scene->RandomPointOnLight(noise.x, noise.y);  
	float const dist = length(dir); 
	dir = normalize(dir);

	Ray shadow = Ray(hit.point - dir * sEps, -dir, dist - sEps);
	if (hit.scene->IsOccluded(shadow)) return BLACK;

	float const cosa = max(0.0f, dot(hit.normal, -dir));
	float const attenuation = (1.0f / (dist * dist));
	float const probability = hit.scene->GetLightArea() * hit.scene->GetLightCount(); 

	return attenuation * cosa * probability;
}

color Renderer::Miss(float3 const direction) const
{
	return mSet.mSkydomeEnabled ? mSkydome.Sample(direction) : mMiss;
}

color Renderer::MissIntensity(Intersection const& hit) const
{
	float3 const random = randomUnitOnHemisphere(hit.normal);
	if (hit.scene->IsOccluded({ hit.point + random * sEps, random })) return BLACK; 
	float const cosa = max(0.0f, dot(hit.normal, random));
	return cosa * mMiss;
}

color Renderer::MissIntensity(tinybvh::Ray const& ray) const
{
	float3 const random = randomUnitOnHemisphere(ray.hit.normal); 
	if (mBVHScene.IsOccluded({ ray.hit.point + random * sEps, random })) return BLACK; 
	float const cosa = max(0.0f, dot(ray.hit.normal, random)); 
	return cosa * mMiss;
}

Intersection Renderer::CalcIntersection(Ray const& ray) const
{
	Intersection hit; 
	hit.point		= calcIntersectionPoint(ray);  
	hit.normal		= mScene.GetNormal(ray.objIdx, hit.point, ray.D); 
	hit.in			= ray.D; 
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		hit.mat = &mSphereMaterial;
	}
	else if (ray.objIdx == mScene.torus.objIdx)
	{
		hit.mat = &mTorusMaterial;
	}
	else if (ray.objIdx == mScene.cube.objIdx)
	{
		hit.mat = &mCubeMaterial;
	}
	else if (ray.objIdx == mScene.plane[2].objIdx)  
	{
		hit.mat = &mFloorMaterial;
	}
#ifdef FOURLIGHTS  
	else if (ray.objIdx == mScene.quad[0].objIdx)
	{
		hit.mat = &mQuadMaterial;
	}
#else
	else if (ray.objIdx == mScene.quad.objIdx) 
	{
		hit.mat = &mQuadMaterial;
	}
#endif
	else
	{
		hit.mat = nullptr; 
	}
	hit.scene		= &mScene;
	hit.t			= ray.t; 
	return hit; 
}

color Renderer::Reproject(Ray const& primRay, color const& sample) const
{
	if (!DidHit(primRay)) return sample;  

	color historySample = 0.0f;
	float historyWeight = 0.0f;

	float3 const primI	= calcIntersectionPoint(primRay);
	Frustum const& frustum = mCamera.GetPrevFrustum();  
	float const dLeft	= distanceToFrustum(frustum.mPlanes[0], primI); 
	float const dRight	= distanceToFrustum(frustum.mPlanes[1], primI);
	float const dTop	= distanceToFrustum(frustum.mPlanes[2], primI);
	float const dBottom = distanceToFrustum(frustum.mPlanes[3], primI);
	float x = dLeft / (dLeft + dRight); 
	float y = dTop  / (dTop + dBottom);  
	float const prevX	= SCRWIDTH * x - 1.0f; 
	float const prevY	= SCRHEIGHT * y - 1.0f;    
	x = prevX / SCRWIDTH; 
	y = prevY / SCRHEIGHT;  

	if (prevX >= 1 && prevX <= SCRWIDTH - 2 && prevY >= 1 && prevY <= SCRHEIGHT - 2)  
	{
		Ray repRay = Ray(mCamera.GetPrevPosition(), normalize(primI - mCamera.GetPrevPosition())); 
		mScene.FindNearest(repRay);
		float3 const repI = calcIntersectionPoint(repRay); 
		if (sqrLength(primI - repI) < 0.0001f)  
		{
			historyWeight = mHistoryWeight;   
			float4 const data = mHistory.Sample(float2(x, y));      
			historySample = static_cast<color>(data);  
		}
	}

	return historyWeight * historySample + (1.0f - historyWeight) * sample;
}

void Renderer::ResetAccumulator()
{
	mSpp = 1;
	mFrame = 0; 
	mAccumulator.Clear();   
} 

void Renderer::ResetHistory()
{
	mHistory.Clear();  
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
	if (!mSet.mPictureModeEnabled) mUi.General(); 
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
	ResourceManager::Init();  
	InitUi();
	InitAccumulator(); 
	mSplineAnimator = SplineAnimator(&mMovieSpline);  

	//lights2.Add(LIGHT_TYPES_TEXTURED_SPOTLIGHT2); 
	//lights2.mData[0].mTs.mIntensity = 20.0f;     
	//lights2.mData[0].mTs.mTexture = loadTextureI("../assets/LDR_RG01_0.png");
	//lights2.mData[0].mTs.mTexture.mSampleMode = TEXTURE_SAMPLE_MODES_CLAMPED;
	//lights2.mData[0].mTs.mTexture.mFilterMode = TEXTURE_FILTER_MODES_NEAREST;  
	//transformTexturedSpotlight(lights2.mData[0], { 0.0f, 4.0f, -2.5f }, 0.0f); 

	lights3.Add(NEW_LIGHT_TYPES_POINT_LIGHT); 
	lights3.mData[0].mPs[0].mColor = WHITE; 
	lights3.mData[0].mPs[1].mColor = WHITE;
	lights3.mData[0].mPs[2].mColor = WHITE;  
	lights3.mData[0].mPs[3].mColor = WHITE;   
	//lights3.mData[0].mPs.mR4 = _mm_set1_ps(1.0f);  
	//lights3.mData[0].mPs.mR4 = _mm_set1_ps(1.0f);  
	//lights3.mData[0].mPs.mB4 = _mm_set1_ps(1.0f);  
	//lights3.mData[0].mPs.mI4 = _mm_set1_ps(1.0f);   

	mFrame = 0;  
	 
	mMiss						= INIT_MISS; 
	mSet.mRenderMode			= INIT_RENDER_MODE;
	mSet.mConvergeMode			= INIT_CONVERGE_MODE;
	sEps						= INIT_EPS;
	mHistoryWeight				= INIT_HISTORY_WEIGHT;  
	mSet.mMaxBounces			= INIT_MAX_BOUNCES;

	mSet.mDirLightEnabled		= INIT_LIGHTS_DIR_LIGHT_ACTIVE;
	mSet.mPointLightsEnabled	= INIT_LIGHTS_POINT_LIGHTS_ACTIVE;
	mSet.mSpotlightsEnabled		= INIT_LIGHTS_SPOT_LIGHTS_ACTIVE; 
	mSet.mSkydomeEnabled		= INIT_LIGHTS_SKYDOME_ACTIVE;

	mSet.mDofEnabled			= INIT_DOF_ACTIVE;
	mSet.mBreakPixelEnabled		= INIT_BREAK_PIXEL;
	mSet.mPictureModeEnabled	= INIT_PICTURE_MODE_ACTIVE;
	mSet.mMaxFramesEnabled		= INIT_MAX_FRAMES_ACTIVE;
	mSet.mAaEnabled				= INIT_AA_ACTIVE;
	mSet.mAutoFocusEnabled		= INIT_AUTO_FOCUS_ACTIVE;

	mDirLight.mDirection	= float3(0.8f, -0.3f, 0.5f);
	mDirLight.mDirection	= normalize(mDirLight.mDirection); 
	mDirLight.mStrength		= 1.0f;
	mDirLight.mColor		= WHITE;    

	mSphereMaterial = Material();     
	mTorusMaterial	= Material();  
	mCubeMaterial	= Material();  
	mFloorMaterial	= Material();  
	mQuadMaterial	= Material(); 

	mCubeMaterial.mGlossy.emission = WHITE * 2.0f;   

	mSkydome = Skydome("../assets/hdr/kloppenheim_06_puresky_4k.hdr");    
}

inline void Renderer::InitUi()
{
	mUi.mRenderer = this; 
}

inline void Renderer::InitAccumulator()
{
	mAccumulator	= Texture<float4>(SCRWIDTH, SCRHEIGHT); 
	mHistory		= Texture<float4>(SCRWIDTH, SCRHEIGHT);  
	//mAccumulator.mOwnData	= true; 
	//mHistory.mOwnData		= true; 
	mHistory.mSampleMode	= TEXTURE_SAMPLE_MODES_CLAMPED;    
	mHistory.mFilterMode	= TEXTURE_FILTER_MODES_LINEAR;     
}

float2 Renderer::RandomOnPixel(int const x, int const y) const
{
	return { static_cast<float>(x) + RandomFloat(), static_cast<float>(y) + RandomFloat() }; 
}

float2 Renderer::RandomOnPixel(blueSeed const seed) const 
{
	return float2(static_cast<float>(seed.mX), static_cast<float>(seed.mY)) + BlueNoise::GetInstance().Float2(seed);  
}

float2 Renderer::CenterOfPixel(int const x, int const y) const
{
	return { static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f };  
}

bool Renderer::DidHit(Ray const& ray) const  
{
	return ray.objIdx != -1; 
}
 
bool Renderer::DidHit(tinybvh::Ray const& ray) const 
{
	return ray.hit.t < BVH_FAR;
}

void Renderer::Shutdown()
{
}

void Renderer::Input()
{
	mBreakPixel			= input.IsKeyReleased(CONTROLS_BREAK_PIXEL) && mSet.mBreakPixelEnabled; 
	mSet.mPictureModeEnabled = input.IsKeyReleased(CONTROLS_PICTURE_MODE) ? !mSet.mPictureModeEnabled : mSet.mPictureModeEnabled; 
}
