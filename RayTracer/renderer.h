#pragma once

#include "lights.h" 
#include "ui.h" 
#include "scene.h"
#include "camera.h" 

enum renderModes : uint8_t
{
	RENDER_MODES_NORMALS,
	RENDER_MODES_DEPTH,
	RENDER_MODES_ALBEDO,
	RENDER_MODES_SHADED
};

int constexpr	INIT_RENDER_MODE	= RENDER_MODES_SHADED;
float constexpr INIT_EPS			= 1e-3f; 

namespace Tmpl8
{

class Renderer final : public TheApp
{
public:
	inline static float		sEps;

private:
	float4*					mAccumulator;
	Ui						mUi;  
	Scene					mScene;
	Camera					mCamera;
	DirectionalLight		mDirLight;
	std::vector<PointLight> mPointLights; 
	std::vector<SpotLight>	mSpotLights; 
	int						mRenderMode;

	Timer mTimer; 
	float mAvg = 10, mFps, mRps, mAlpha = 1;

public:
	void					Tick( float deltaTime ) override;

	void					SetRenderMode(int const renderMode);

	inline int				GetRenderMode() const	{ return mRenderMode; }
	inline float			GetFps() const			{ return mFps; }
	inline float			GetRps() const			{ return mRps; }
	inline float			GetAvg() const			{ return mAvg; }

private:
	[[nodiscard]] float3	Trace( Ray& ray ) const;
	[[nodiscard]] float3	CalcDirectLight(Scene const& scene, float3 const& intersection, float3 const& normal) const;
	void					ResetAccumulator() const; 
	void					PerformanceReport(); 

	void					UI() override;

	void					Init() override;
	void					Shutdown() override;
	inline void				InitUi();
	inline void				InitAccumulator(); 
};

} // namespace Tmpl8

[[nodiscard]] static float3 calcIntersection(Ray const& ray);  