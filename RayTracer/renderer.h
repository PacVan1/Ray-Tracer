#pragma once

#include "lights.h"
#include "materials.h" 
#include "ui.h" 
#include "scene.h"
#include "camera.h"
#include "debug.h"
#include "skydome.h"
#include "hitinfo.h" 
#include "textures.h" 

enum renderModes : uint8_t
{
	RENDER_MODES_NORMALS,
	RENDER_MODES_DEPTH,
	RENDER_MODES_ALBEDO,
	RENDER_MODES_SHADED
};

enum accumModes : uint8_t
{
	ACCUM_MODES_NONE, 
	ACCUM_MODES_ACCUMULATION, 
	ACCUM_MODES_REPROJECTION
};

color const		INIT_MISS						= BLACK;  
int constexpr	INIT_RENDER_MODE				= RENDER_MODES_SHADED;
int constexpr	INIT_ACCUM_MODE					= ACCUM_MODES_ACCUMULATION;        
float constexpr INIT_EPS						= 1e-3f;
float constexpr INIT_HISTORY_WEIGHT				= 0.8f; 
int constexpr	INIT_MAX_BOUNCES				= 10; 

bool constexpr	INIT_LIGHTS_DIR_LIGHT_ACTIVE	= false;  
bool constexpr	INIT_LIGHTS_POINT_LIGHTS_ACTIVE	= false;
bool constexpr	INIT_LIGHTS_SPOT_LIGHTS_ACTIVE	= false;
bool constexpr	INIT_LIGHTS_SKYDOME_ACTIVE		= true;    

bool constexpr	INIT_DOF_ACTIVE					= true;  
bool constexpr	INIT_BREAK_PIXEL				= false;      
bool constexpr	INIT_AA_ACTIVE					= true;
bool constexpr	INIT_ACCUM_ACTIVE				= false;
bool constexpr	INIT_AUTO_FOCUS_ACTIVE			= true;  

namespace Tmpl8
{

class Renderer final : public TheApp
{
public:
	inline static float		sEps;

public:
	DebugViewer2D			mDebugViewer; 
	bool					mDebugViewerActive	= false;
	bool					mBreakPixelActive	= false;
	bool					mPictureModeActive	= false;
	bool					mMaxFramesActive	= false;
	int						mMaxFrames			= 1;
	Camera					mCamera;
	Scene					mScene;

	Metallic				mMetallic;
	Dielectric				mDielectric; 
	Glossy					mGlossy;  
	Glossy2					mGlossy2;  
	Lambertian				mLambertian;
	Lambertian2				mLambertian2;
	Lambertian3				mLambertian3;

	Material*				mSphereMaterial; 
	Material*				mTorusMaterial; 
	Material*				mCubeMaterial; 
	Material*				mFloorMaterial;
	Material*				mQuadMaterial; 

	std::vector<PointLight> mPointLights; 
	std::vector<Spotlight>	mSpotLights; 
	TexturedSpotlight		mTexturedSpotlight; 
	Skydome					mSkydome;  

	bool					mDirLightActive;
	bool					mPointLightsActive;
	bool					mSpotLightsActive; 
	bool					mQuadLightActive = true;  
	bool					mSkydomeActive;

	Ui						mUi;

	// BUFFERS
	Texture<float4>			mAccumulator;
	Texture<float4>			mHistory;  
	float					mHistoryWeight; 

	DirectionalLight		mDirLight;
	color					mMiss;

	int						mRenderMode;
	int						mAccumMode; 
	int						mMaxBounces;
	bool					mDofActive;
	bool					mBreakPixel; 
	bool					mAaActive;
	bool					mAccumActive;
	bool					mAutoFocusActive;
	int						mSpp;
	int						mFrame; 

	Timer					mTimer; 
	float					mAvg = 10, mFps, mRps, mAlpha = 1;

	bool					mAnimating = true;  
	float					mAnimTime = 0.0f;  

public:
	void					Tick( float deltaTime ) override;
	void					ResetAccumulator(); 
	void					ResetHistory(); 

	void					SetRenderMode(int const renderMode);
	void					SetMaxBounces(int const maxBounces);
	void					SetAa(bool const aaActive); 
	void					SetAccum(bool const accumActive); 
	void					SetAccumMode(int const accumMode);  
	void					SetAutoFocus(bool const autoFocusActive); 
	void					SetDof(bool const dofActive); 

	inline int				GetRenderMode() const	{ return mRenderMode; }
	inline int				GetMaxBounces() const	{ return mMaxBounces; }
	inline int				GetAa() const			{ return mAaActive; }
	inline int				GetAccum() const		{ return mAccumActive; } 
	inline int				GetAccumMode() const	{ return mAccumMode; }
	inline int				GetAutoFocus() const	{ return mAutoFocusActive; } 
	inline int				GetDof() const			{ return mDofActive; }
	inline int				GetSpp() const			{ return mSpp; }
	inline int				GetFrame() const		{ return mFrame; }
	inline float			GetFps() const			{ return mFps; }
	inline float			GetRps() const			{ return mRps; }
	inline float			GetAvg() const			{ return mAvg; }

private:
	[[nodiscard]] color		Trace(Ray& ray) const;
	[[nodiscard]] color		Trace2(Ray& primRay) const;  
	[[nodiscard]] color		TraceDebug(Ray& ray, debug debug = {});
	[[nodiscard]] color		TraceRecursive(Ray& ray, int const bounces = 0) const;
	[[nodiscard]] color		TraceDebugRecursive(Ray& ray, debug debug = {}, int const bounces = 0);
	[[nodiscard]] color		TraceNormals(Ray& ray) const; 
	[[nodiscard]] color		TraceDepth(Ray& ray) const; 
	[[nodiscard]] color		TraceAlbedo(Ray& ray) const; 
	[[nodiscard]] color		CalcDirectLight(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] color		CalcDirectLight2(Scene const& scene, HitInfo const& info) const;
	[[nodiscard]] color		CalcDirectLightWithArea(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] color		CalcDirectLightWithArea2(Scene const& scene, HitInfo const& info) const;
	[[nodiscard]] color		CalcQuadLight(Scene const& scene, HitInfo const& info) const;
	[[nodiscard]] color		Miss(float3 const direction) const;
	[[nodiscard]] color		MissIntensity(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	[[nodiscard]] color		MissIntensity2(Scene const& scene, HitInfo const& info) const;
	[[nodiscard]] HitInfo	CalcHitInfo(Ray const& ray) const;
	[[nodiscard]] color		Reproject(Ray const& primRay, color const& sample) const;  
	[[nodiscard]] color		Reproject2(Ray const& primRay, color const& sample) const; 
	void					PerformanceReport();  

	[[nodiscard]] inline float2		RandomOnPixel(int const x, int const y) const; 
	[[nodiscard]] inline bool		DidHit(Ray const& ray) const;

	void					UI() override;
	void					Input() override;
	void					RenderDebugViewer(); 

	void					Init() override;
	void					Shutdown() override;
	inline void				InitUi();
	inline void				InitAccumulator(); 
};

} // namespace Tmpl8