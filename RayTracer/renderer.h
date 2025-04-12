#pragma once

#include "lights.h" 

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
	float4*		mAccumulator;
	Scene		mScene;
	Camera		mCamera;
	PointLight	mPointLight;
	int			mRenderMode;

public:
	float3 Trace( Ray& ray );

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