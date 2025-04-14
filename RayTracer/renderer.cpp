#include "precomp.h"
#include "renderer.h"

#include "settings.h" 

void Renderer::Tick( float deltaTime )
{
#if DEBUG_MODE
	mTimer.reset();
	int debugRay = 0; 
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++)
	{
		switch (mRenderMode)
		{
		case RENDER_MODES_NORMALS:
		{
			float3 const pixel = TraceNormals(mCamera.GetPrimaryRay((float)x, (float)y)); 
			mScreen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(pixel); 
			break;
		}
		case RENDER_MODES_DEPTH:
		{
			float3 const pixel = TraceDepth(mCamera.GetPrimaryRay((float)x, (float)y));
			mScreen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(pixel);
			break;
		}
		case RENDER_MODES_ALBEDO:
		{
			float3 const pixel = TraceAlbedo(mCamera.GetPrimaryRay((float)x, (float)y));
			mScreen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(pixel);
			break;
		}
		case RENDER_MODES_SHADED:
		{
			Ray primRay = mCamera.GetPrimaryRay(static_cast<float>(x), static_cast<float>(y));
			if (mDebugViewerActive)
			{
				int debug = 0; 
				if (y == mDebugViewer.mRow && x % mDebugViewer.mEvery == 0)
				{
					debug = 1; debugRay++; 
					if (debugRay == mDebugViewer.mSelected) setSelected(debug); // set ray to selected 
				}

				float3 const pixel = TraceDebug(primRay, debug);   
				mScreen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(pixel);
			}
			else
			{
				float3 const pixel = Trace(primRay);
				mScreen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(pixel); 
			}
			break;
		}
		default: break; 
		}
	}

	PerformanceReport();
	if (mDebugViewerActive) RenderDebugViewer(); 

	mCamera.HandleInput( deltaTime );
#endif
}

void Renderer::SetRenderMode(int const renderMode)
{
	mRenderMode = renderMode;
	ResetAccumulator();
}

float3 Renderer::Trace(Ray& ray, int const bounces) const
{
	if (bounces >= mMaxBounces) return BLACK;
	mScene.FindNearest( ray );
	if (ray.objIdx == -1) return BLACK; // or a fancy sky color
	float3 const intersection	= calcIntersection(ray); 
	float3 const normal			= mScene.GetNormal( ray.objIdx, intersection, ray.D );
	color const	 albedo			= mScene.GetAlbedo( ray.objIdx, intersection);

	if (ray.objIdx == mScene.cube.objIdx)
	{
		float3 tempN = normal;
		float ior = 1.0f / 1.3f;

		if (ray.inside) // inside 
		{ 
			ior = 1.3f;
		}

		float const cosTheta = std::fmin(dot(-ray.D, tempN), 1.0f);
		float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
		float const fresnel = schlickApprox(cosTheta, ior);

		if (ior * sinTheta > 1.0f && mTIR) 
		{
			float3 reflectedDir = reflect(ray.D, tempN);
			Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
			reflected.inside = ray.inside;
			return Trace(reflected, (bounces + 1));
		}

		float3 reflectedDir = reflect(ray.D, tempN);
		Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
		reflected.inside = ray.inside;

		float3 rPerp = ior * (ray.D + cosTheta * tempN);
		float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
		float3 refractedDir = rPerp + rPara;
		Ray refracted = Ray(intersection + (-normal) * sEps, refractedDir);
		refracted.inside = !ray.inside;

		return Trace(reflected, (bounces + 1)) * fresnel + Trace(refracted, (bounces + 1)) * (1.0f - fresnel);
	}
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		float3 tempN = normal;
		float ior = 1.0f / 1.33f;

		if (ray.inside) // inside 
		{
			ior = 1.33f;
		}

		float const cosTheta = std::fmin(dot(-ray.D, tempN), 1.0f);
		float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
		float const fresnel = schlickApprox(cosTheta, ior);

		if (ior * sinTheta > 1.0f && mTIR)
		{
			float3 reflectedDir = reflect(ray.D, tempN);
			Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
			reflected.inside = ray.inside;
			return Trace(reflected, (bounces + 1));
		}

		float3 reflectedDir = reflect(ray.D, tempN);
		Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
		reflected.inside = ray.inside;

		float3 rPerp = ior * (ray.D + cosTheta * tempN);
		float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
		float3 refractedDir = rPerp + rPara;
		Ray refracted = Ray(intersection + (-normal) * sEps, refractedDir);
		refracted.inside = !ray.inside;

		return Trace(reflected, (bounces + 1)) * fresnel + Trace(refracted, (bounces + 1)) * (1.0f - fresnel);
	}

	return CalcDirectLight(mScene, intersection, normal) * albedo;
}

float3 Renderer::TraceDebug(Ray& ray, int debug, int const bounces)
{
	if (bounces >= mMaxBounces) return BLACK;
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK; // or a fancy sky color
	float3 const intersection = calcIntersection(ray);
	float3 const normal = mScene.GetNormal(ray.objIdx, intersection, ray.D);
	color const	 albedo = mScene.GetAlbedo(ray.objIdx, intersection);

	if (ray.inside) setInside(debug); else setOutside(debug);

	mDebugViewer.RenderRay(ray, intersection, normal, debug, bounces);   

	if (ray.objIdx == mScene.cube.objIdx)
	{
		float3 tempN = normal;
		float ior = 1.0f / 1.3f;

		if (ray.inside) // inside 
		{
			ior = 1.3f;
		}

		float const cosTheta	= std::fmin(dot(-ray.D, tempN), 1.0f);
		float const sinTheta	= std::sqrt(1.0f - cosTheta * cosTheta);
		float const fresnel		= schlickApprox(cosTheta, ior);

		if (ior * sinTheta > 1.0f)
		{
			debug |= (1 << 3);

			float3 reflectedDir = reflect(ray.D, tempN);
			Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
			reflected.inside = ray.inside;
			return TraceDebug(reflected, debug, (bounces + 1));
		}
		else
		{
			debug &= ~(1 << 3);
		}

		float3 reflectedDir = reflect(ray.D, tempN);
		Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
		reflected.inside = ray.inside;

		float3 rPerp = ior * (ray.D + cosTheta * tempN);
		float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
		float3 refractedDir = rPerp + rPara;
		Ray refracted = Ray(intersection + (-normal) * sEps, refractedDir);
		refracted.inside = !ray.inside;

		return TraceDebug(reflected, debug, (bounces + 1)) * fresnel + TraceDebug(refracted, debug, (bounces + 1)) * (1.0f - fresnel);
	}
	if (ray.objIdx == mScene.sphere.objIdx)
	{
		float3 tempN = normal;
		float ior = 1.0f / 1.3f;

		if (ray.inside) // inside 
		{
			ior = 1.3f;
		}

		float const cosTheta = std::fmin(dot(-ray.D, tempN), 1.0f);
		float const sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
		float const fresnel = schlickApprox(cosTheta, ior);

		if (ior * sinTheta > 1.0f)
		{
			debug |= (1 << 3);

			float3 reflectedDir = reflect(ray.D, tempN);
			Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
			reflected.inside = ray.inside;
			return TraceDebug(reflected, debug, (bounces + 1));
		}
		else
		{
			debug &= ~(1 << 3);
		}

		float3 reflectedDir = reflect(ray.D, tempN);
		Ray reflected = Ray(intersection + normal * sEps, reflectedDir);
		reflected.inside = ray.inside;

		float3 rPerp = ior * (ray.D + cosTheta * tempN);
		float3 rPara = -std::sqrt(std::fabs(1.0f - sqrLength(rPerp))) * tempN;
		float3 refractedDir = rPerp + rPara;
		Ray refracted = Ray(intersection + (-normal) * sEps, refractedDir);
		refracted.inside = !ray.inside;

		return TraceDebug(reflected, debug, (bounces + 1)) * fresnel + TraceDebug(refracted, debug, (bounces + 1)) * (1.0f - fresnel);
	}

	return CalcDirectLight(mScene, intersection, normal) * albedo;
}

float3 Renderer::TraceNormals(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK;

	float3 const intersection	= calcIntersection(ray);
	float3 const normal			= mScene.GetNormal(ray.objIdx, intersection, ray.D);

	return (normal + 1) * 0.5f;
}

float3 Renderer::TraceDepth(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK;

	return 0.1f * float3(ray.t, ray.t, ray.t);
}

float3 Renderer::TraceAlbedo(Ray& ray) const
{
	mScene.FindNearest(ray);
	if (ray.objIdx == -1) return BLACK; // or a fancy sky color
	float3 const intersection	= calcIntersection(ray);
	color const	 albedo			= mScene.GetAlbedo(ray.objIdx, intersection);
	return albedo; 
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

	mDirLight.mDirection	= float3(0.0f, -1.0f, 0.0f);
	mDirLight.mDirection	= normalize(mDirLight.mDirection); 
	mDirLight.mStrength		= 1.0f;
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