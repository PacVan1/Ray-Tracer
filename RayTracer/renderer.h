#pragma once

#include "lights.h"
#include "materials.h" 
#include "ui.h" 
#include "scene.h"
#include "camera.h"
#include "debug.h"
#include "skydome.h"
#include "textures.h" 
#include "bvh_scene.h" 

enum renderModes : uint8_t
{
	RENDER_MODES_NORMALS,
	RENDER_MODES_DEPTH,
	RENDER_MODES_ALBEDO,
	RENDER_MODES_SHADED
};

enum convergeModes : uint8_t
{
	CONVERGE_MODES_NONE, 
	CONVERGE_MODES_ACCUMULATION,  
	CONVERGE_MODES_REPROJECTION
};

inline auto constexpr MOVIE_FILE_PATH = "../assets/spline.txt";

color const		INIT_MISS						= WHITE * 0.4f;  
int constexpr	INIT_RENDER_MODE				= RENDER_MODES_NORMALS;
int constexpr	INIT_CONVERGE_MODE				= CONVERGE_MODES_ACCUMULATION;           
float constexpr INIT_EPS						= 1e-3f;
float constexpr INIT_HISTORY_WEIGHT				= 0.8f; 
int constexpr	INIT_MAX_BOUNCES				= 10;  

bool constexpr	INIT_LIGHTS_DIR_LIGHT_ACTIVE	= false;  
bool constexpr	INIT_LIGHTS_POINT_LIGHTS_ACTIVE	= false;
bool constexpr	INIT_LIGHTS_SPOT_LIGHTS_ACTIVE	= false;
bool constexpr	INIT_LIGHTS_SKYDOME_ACTIVE		= false;        

bool constexpr	INIT_DOF_ACTIVE					= false;  
bool constexpr	INIT_BREAK_PIXEL				= false; 
bool constexpr  INIT_PICTURE_MODE_ACTIVE		= false;
bool constexpr  INIT_MAX_FRAMES_ACTIVE			= false; 
bool constexpr	INIT_AA_ACTIVE					= false;
bool constexpr	INIT_AUTO_FOCUS_ACTIVE			= false;   

struct Settings  
{
	// MODES:
	int		mRenderMode; 
	int		mConvergeMode;
	// DEBUGGING:
	bool	mDebugViewerEnabled; 
	bool	mBreakPixelEnabled; 
	bool	mPictureModeEnabled; 
	bool	mMaxFramesEnabled; 
	// FEATURES: 
	bool	mDofEnabled;		// depth of field 
	bool	mAaEnabled;			// anti-aliasing
	bool	mAutoFocusEnabled;	// make depth of field automatically focus
	bool	mBlueNoiseEnabled; 
	bool	mStochasticLights; 
	// LIGHTS:
	bool	mDirLightEnabled;
	bool	mPointLightsEnabled;
	bool	mSpotlightsEnabled; 
	bool	mQuadLightEnabled;
	bool	mSkydomeEnabled;
	bool	mTexturedSpotlightEnabled; 
	// SLIDERS:
	int		mMaxBounces; 
	int		mMaxFrames;
};

struct Intersection
{
	float3			point	= 0.0f;
	float3			normal	= 0.0f;
	float3			in		= 0.0f;
	Material const* mat		= nullptr;
	Scene const*	scene	= nullptr;
	float			t		= RAY_FAR;
	bool			inside	= false;
	uint8_t			padding[7];
};

namespace Tmpl8
{
class Renderer final : public TheApp
{
public:
	inline static float		sEps;

public:
	Settings				mSet;    
	Ui						mUi;  
	DebugViewer2D			mDebugViewer; 
	Camera					mCamera;
	Scene					mScene;
	BVHScene				mBVHScene; 
	Spline					mMovieSpline; 
	SplineAnimator			mSplineAnimator; 
	bool					mAnimate; 

	Material				mSphereMaterial; 
	Material				mTorusMaterial; 
	Material				mCubeMaterial; 
	Material				mFloorMaterial;
	Material				mQuadMaterial; 

	std::vector<PointLight> mPointLights; 
	std::vector<Spotlight>	mSpotLights; 
	TexturedSpotlight		mTexturedSpotlight; 
	DirectionalLight		mDirLight;
	Skydome					mSkydome;  
	color					mMiss; 

	Texture<float4>			mAccumulator;
	Texture<float4>			mHistory;  
	float					mHistoryWeight; 

	int						mSpp;
	int						mFrame; 
	bool					mBreakPixel; 

	Timer					mTimer; 
	float					mAvg = 10, mFps, mRps, mAlpha = 1;

	bool					mAnimating = true;  
	float					mAnimTime = 0.0f;  

public:
	void						Tick( float deltaTime ) override;
	void						ResetAccumulator(); 
	void						ResetHistory(); 

	inline Settings&			GetSettings()			{ return mSet; } 
	inline DebugViewer2D&		GetDebugViewer()		{ return mDebugViewer; }

	inline int					GetSpp() const			{ return mSpp; }
	inline int					GetFrame() const		{ return mFrame; }
	inline float				GetFps() const			{ return mFps; }
	inline float				GetRps() const			{ return mRps; }
	inline float				GetAvg() const			{ return mAvg; }

private:
	[[nodiscard]] color			Trace(Ray& primRay) const;  
	[[nodiscard]] color			Trace(tinybvh::Ray& primRay); 
	[[nodiscard]] color			Trace(Ray& primRay, blueSeed& seed) const;    
	[[nodiscard]] color			TraceDebug(Ray& ray, debug debug = {});
	[[nodiscard]] color			TraceNormals(Ray& ray) const;  
	[[nodiscard]] color			TraceNormals(tinybvh::Ray& ray);  
	[[nodiscard]] color			TraceDepth(Ray& ray) const; 
	[[nodiscard]] color			TraceDepth(tinybvh::Ray& ray) const; 
	[[nodiscard]] color			TraceAlbedo(Ray& ray) const; 
	[[nodiscard]] color			TraceAlbedo(tinybvh::Ray& ray); 
	[[nodiscard]] color			CalcDirectLight(Intersection const& hit) const; 
	[[nodiscard]] color			CalcDirectLight(tinybvh::Ray const& ray) const; 
	[[nodiscard]] color			CalcDirectLightWithArea(Intersection const& info) const;
	[[nodiscard]] color			CalcDirectLightWithArea(tinybvh::Ray const& ray) const; 
	[[nodiscard]] color			CalcDirectLightWithArea(Intersection const& hit, blueSeed const seed) const;
	[[nodiscard]] color			CalcQuadLight(Intersection const& hit) const;
	[[nodiscard]] color			CalcQuadLight(Intersection const& hit, blueSeed const seed) const; 
	[[nodiscard]] color			Miss(float3 const direction) const;
	[[nodiscard]] color			MissIntensity(Intersection const& hit) const; 
	[[nodiscard]] color			MissIntensity(tinybvh::Ray const& ray) const;  
	[[nodiscard]] Intersection	CalcIntersection(Ray const& ray) const;
	[[nodiscard]] color			Reproject(Ray const& primRay, color const& sample) const;  
	void						PerformanceReport();  

	[[nodiscard]] inline float2		RandomOnPixel(int const x, int const y) const;  
	[[nodiscard]] inline float2		RandomOnPixel(blueSeed const seed) const;  
	[[nodiscard]] inline float2		CenterOfPixel(int const x, int const y) const;   
	[[nodiscard]] inline bool		DidHit(Ray const& ray) const;
	[[nodiscard]] inline bool		DidHit(tinybvh::Ray const& ray) const; 

	void					UI() override;
	void					Input() override;
	void					RenderDebugViewer(); 

	void					Init() override;
	void					Shutdown() override;
	inline void				InitUi();
	inline void				InitAccumulator(); 
};

} // namespace Tmpl8