#pragma once

#include "lights.h"
#include "materials.h" 
#include "ui.h" 
#include "scene.h"
#include "camera.h"
#include "debug.h" 
#include "textures.h" 

enum renderModes : uint8_t
{
	RENDER_MODES_NORMALS,
	RENDER_MODES_DEPTH,
	RENDER_MODES_ALBEDO,
	RENDER_MODES_SHADED
};

int constexpr	INIT_RENDER_MODE	= RENDER_MODES_SHADED;
float constexpr INIT_EPS			= 1e-3f;
int constexpr	INIT_MAX_BOUNCES	= 10; 

namespace Tmpl8
{

class Renderer final : public TheApp
{
public:
	inline static float		sEps;

public:
	DebugViewer2D			mDebugViewer; 
	bool					mDebugViewerActive = false;

private:
	Ui						mUi;

	float4*					mAccumulator;
	Scene					mScene;
	Camera					mCamera;
	DirectionalLight		mDirLight;
	DirectionalLight		mDirLight2;
	std::vector<PointLight> mPointLights; 
	std::vector<SpotLight>	mSpotLights;

	Metallic				mMetallic;
	Dielectric				mDielectric; 

	HdrTexture				mHdrTexture;

	int						mRenderMode;
	int						mMaxBounces;

	Timer mTimer; 
	float mAvg = 10, mFps, mRps, mAlpha = 1;

public:
	void					Tick( float deltaTime ) override;

	void					SetRenderMode(int const renderMode);
	void					SetMaxBounces(int const maxBounces); 

	inline int				GetRenderMode() const	{ return mRenderMode; }
	inline int				GetMaxBounces() const	{ return mMaxBounces; }
	inline float			GetFps() const			{ return mFps; }
	inline float			GetRps() const			{ return mRps; }
	inline float			GetAvg() const			{ return mAvg; }

private:
	[[nodiscard]] color		Trace(Ray& ray, int const bounces = 0) const;
	[[nodiscard]] color		TraceDebug(Ray& ray, debug debug = {}, int const bounces = 0);
	[[nodiscard]] color		TraceNormals(Ray& ray) const; 
	[[nodiscard]] color		TraceDepth(Ray& ray) const; 
	[[nodiscard]] color		TraceAlbedo(Ray& ray) const; 
	[[nodiscard]] color		CalcDirectLight(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	void					ResetAccumulator() const; 
	void					PerformanceReport(); 

	void					UI() override;
	void					RenderDebugViewer(); 

	void					Init() override;
	void					Shutdown() override;
	inline void				InitUi();
	inline void				InitAccumulator(); 
};

} // namespace Tmpl8