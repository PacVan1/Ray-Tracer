#pragma once

#include "lights.h" 

auto const RED		= float3(1.0f, 0.0f, 0.0f); 
auto const GREEN	= float3(0.0f, 1.0f, 0.0f); 
auto const BLUE		= float3(0.0f, 0.0f, 1.0f); 
auto const YELLOW	= float3(1.0f, 1.0f, 0.0f); 
auto const PURPLE	= float3(1.0f, 0.0f, 1.0f); 
auto const CYAN		= float3(0.0f, 1.0f, 1.0f); 
auto const BLACK	= float3(0.0f, 0.0f, 0.0f); 
auto const WHITE	= float3(1.0f, 1.0f, 1.0f); 

enum renderModes : uint8_t
{
	RENDER_MODES_NORMALS,
	RENDER_MODES_DEPTH,
	RENDER_MODES_ALBEDO,
	RENDER_MODES_SHADED
};

namespace Tmpl8
{

class Renderer final : public TheApp
{
public: 
	float4*				mAccumulator;
	Scene				mScene;
	Camera				mCamera;
	PointLight			mPointLight;
	DirectionalLight	mDirLight;
	SpotLight			mSpotLight;
	int					mRenderMode;

public:
	float3 Trace( Ray& ray );

	float3 CalcDirectLight(Scene const& scene, float3 const& intersection, float3 const& normal) const;  

	void Init() override;
	void Tick( float deltaTime ) override;
	void UI() override;
	void Shutdown() override {}

	void MouseUp( int button ) override {}
	void MouseDown( int button ) override {}
	void MouseMove( int x, int y ) override {}
	void MouseWheel( float y ) override {}
	void KeyUp( int key ) override {}
	void KeyDown( int key ) override {}
};

} // namespace Tmpl8