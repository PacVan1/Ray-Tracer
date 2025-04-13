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

auto const RED		= float3(1.0f, 0.0f, 0.0f); 
auto const GREEN	= float3(0.0f, 1.0f, 0.0f); 
auto const BLUE		= float3(0.0f, 0.0f, 1.0f); 
auto const YELLOW	= float3(1.0f, 1.0f, 0.0f); 
auto const PURPLE	= float3(1.0f, 0.0f, 1.0f); 
auto const CYAN		= float3(0.0f, 1.0f, 1.0f); 
auto const BLACK	= float3(0.0f, 0.0f, 0.0f); 
auto const WHITE	= float3(1.0f, 1.0f, 1.0f); 

int constexpr INIT_RENDER_MODE = RENDER_MODES_NORMALS;

namespace Tmpl8
{

class Renderer final : public TheApp
{
private:
	float4*				mAccumulator;
	Ui					mUi;  
	Scene				mScene;
	Camera				mCamera;
	PointLight			mPointLight;
	DirectionalLight	mDirLight;
	SpotLight			mSpotLight;
	int					mRenderMode;

	Timer mTimer; 
	float mAvg = 10, mFps, mRps, mAlpha = 1;

public:
	void					Tick( float deltaTime ) override;

	void					SetRenderMode(int const renderMode);

	inline float			GetFps() const { return mFps; }
	inline float			GetRps() const { return mRps; }
	inline float			GetAvg() const { return mAvg; }

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