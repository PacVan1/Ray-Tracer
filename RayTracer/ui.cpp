#include "precomp.h"
#include "ui.h"

#include "renderer.h" 

Ui::Ui() :
	mRenderer(nullptr)
{}

void Ui::General() const
{
	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(350, SCRHEIGHT));
	ImGui::Begin("General", nullptr, UI_WINDOW_FLAGS); 

	ImGui::Text("Performance report"); 
	ImGui::Text("%5.2fms (%.1ffps) - %.1fMrays/s\n", mRenderer->GetAvg(), mRenderer->GetFps(), mRenderer->GetRps() / 1000);
	ImGui::Text("OpenMP thread count: %d", omp_get_num_threads());

	Settings& settings = mRenderer->GetSettings(); 

	ImGui::Separator();

	if (ImGui::BeginTabBar("Main"))
	{
		if (ImGui::BeginTabItem("Settings"))		{ SettingsUi();		ImGui::EndTabItem(); } 
		if (ImGui::BeginTabItem("Camera"))			{ CameraUi();		ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Scene"))			{ SceneUi(mRenderer->mBVHScene); ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Lights"))			{ LightsUi();		ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Debug"))			{ DebugUi();		ImGui::EndTabItem(); }
	}

	ImGui::End(); 
}

void Ui::DebugUi() const
{
	Settings& settings			= mRenderer->GetSettings();
	DebugViewer2D debugViewer	= mRenderer->GetDebugViewer(); 

	if (ImGui::CollapsingHeader("2D Slice Viewer"))
	{
		ImGui::Separator(); 
		ImGui::Checkbox("Enabled", &settings.mDebugViewerEnabled); 
		ImGui::DragFloat("Zoom", &debugViewer.mZoom, 0.1f, 1.0f, SCRHEIGHT);
		ImGui::DragFloat2("Position", debugViewer.mPosition.cell, 0.05f);
		if (ImGui::SliderInt("Every", &debugViewer.mEvery, 1, 15))
		{
			debugViewer.SetSelection(); 
		} 
		ImGui::SliderInt("Selected", &debugViewer.mSelected, 0, debugViewer.mSelectionCount); 
		ImGui::Separator();

		ImGui::Text("Color legenda"); 
		ImGui::Separator(); 
		ImGui::ColorButton("ColorBox", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(20, 20));
		ImGui::SameLine(); ImGui::Text("Primary Ray");
		ImGui::ColorButton("ColorBox", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(20, 20));
		ImGui::SameLine(); ImGui::Text("Inside Ray");
		ImGui::ColorButton("ColorBox", ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(20, 20));
		ImGui::SameLine(); ImGui::Text("Outside Ray");
		ImGui::ColorButton("ColorBox", ImVec4(0.0f, 0.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(20, 20));
		ImGui::SameLine(); ImGui::Text("Normal");
		ImGui::ColorButton("ColorBox", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(20, 20));
		ImGui::SameLine(); ImGui::Text("Hit Point");
		ImGui::Separator(); 
	}
	if (ImGui::CollapsingHeader("Pixel Break"))
	{
		ImGui::Separator();
		ImGui::Checkbox("Active", &settings.mBreakPixelEnabled);
		ImGui::Text("Pixel Coordinate: (%d, %d)", input.mMousePos.x, input.mMousePos.y);
		ImGui::Separator();
	}
	if (ImGui::CollapsingHeader("Frame Controller"))
	{
		ImGui::Separator();
		ImGui::Checkbox("Active", &settings.mMaxFramesEnabled); 
		ImGui::DragInt("Number of frames", &settings.mMaxFrames, 1, 1, 1000); 
		ImGui::ProgressBar(static_cast<float>(mRenderer->GetFrame()) / static_cast<float>(settings.mMaxFrames), ImVec2(0.0f, 0.0f), "Progress"); 
		ImGui::Separator();
	}
}

void Ui::SettingsUi() const
{
	Settings& settings = mRenderer->GetSettings();

	ImGui::Separator(); 
	if (ImGui::Combo("Converge mode", &settings.mConvergeMode, STR_CONVERGE_MODES)) mRenderer->ResetAccumulator();   
	if (ImGui::Combo("Render mode", &settings.mRenderMode, STR_RENDER_MODES))		mRenderer->ResetAccumulator();  
	if (ImGui::SliderInt("Max bounces", &settings.mMaxBounces, 1, 10))				mRenderer->ResetAccumulator(); 
	ImGui::Separator(); 
	if (ImGui::Checkbox("Anti-aliasing", &settings.mAaEnabled))						mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Depth of field", &settings.mDofEnabled))					mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Auto-focus", &settings.mAutoFocusEnabled))					mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Blue noise", &settings.mBlueNoiseEnabled))					mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Stochastic lights", &settings.mStochasticLights))			mRenderer->ResetAccumulator(); 
	ImGui::Separator();   
	if (ImGui::Checkbox("Point lights", &settings.mPointLightsEnabled))				mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Spotlights", &settings.mSpotlightsEnabled))				mRenderer->ResetAccumulator();
	if (ImGui::Checkbox("Directional light", &settings.mDirLightEnabled))			mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Quad light", &settings.mQuadLightEnabled))					mRenderer->ResetAccumulator();
	if (ImGui::Checkbox("Textured spotlight", &settings.mTexturedSpotlightEnabled)) mRenderer->ResetAccumulator(); 
	if (ImGui::Checkbox("Skydome illumination", &settings.mSkydomeEnabled))			mRenderer->ResetAccumulator();
	ImGui::Separator(); 
	if (ImGui::CollapsingHeader("Skydome"))  
	{
		TextureUi(mRenderer->mSkydome.mTexture);  
	}
	if (ImGui::CollapsingHeader("Accumulator"))
	{
		TextureUi(mRenderer->mAccumulator); 
	}
	if (ImGui::CollapsingHeader("Reprojector")) 
	{
		TextureUi(mRenderer->mHistory);  
		ImGui::DragFloat("History Weight", &mRenderer->mHistoryWeight, 0.01f, 0.0f, 1.0f); 
	}
}

void Ui::CameraUi() const
{
	Camera& camera = mRenderer->mCamera;  
	float3 position = camera.GetPosition();
	if (ImGui::DragFloat3("Position", position.cell, 0.1f))
	{
		camera.SetPosition(position);
		mRenderer->ResetAccumulator();
		camera.Focus(mRenderer->mScene);
	}
	float3 target = camera.GetTarget();
	if (ImGui::DragFloat3("Target", target.cell, 0.1f))
	{
		camera.SetTarget(target);
		mRenderer->ResetAccumulator();
		camera.Focus(mRenderer->mScene); 
	}
	if (ImGui::DragFloat("Speed", &camera.mSpeed, 0.01f, 0.0f, 10.0f));
	if (ImGui::DragFloat("Sensitivity", &camera.mSensitivity, 0.01f, 0.0f, 10.0f));
	float focusDist = camera.GetFocusDist();
	if (ImGui::DragFloat("Focus distance", &focusDist, 0.01f, 0.0f, INIT_CAMERA_FOCUS_DIST))
	{
		camera.SetFocusDist(focusDist);
		mRenderer->ResetAccumulator();
	}
	float defocusAngle = camera.GetDefocusAngle();
	if (ImGui::DragFloat("Defocus angle", &defocusAngle, 0.005f, 0.0f, INIT_CAMERA_MAX_FOCUS_DIST))
	{
		camera.SetDefocusAngle(defocusAngle);
		mRenderer->ResetAccumulator();
	}
	float fov = camera.GetFov();
	if (ImGui::DragFloat("Field of view", &fov, 0.2f, 0.0f, INIT_CAMERA_MAX_FOV)) 
	{
		camera.SetFov(fov);
		mRenderer->ResetAccumulator();
	}
}

void Ui::MaterialsUi() const
{
	//Dielectric& dielectric = mRenderer->mDielectric;

	//ImGui::DragFloat("Refractive Index", &dielectric.mIor, 0.02f, 1.0f, 3.0f);
	//ImGui::DragFloat3("Absorption", dielectric.mAbsorption.cell, 0.02f, 0.0f, 10.0f);
}

void Ui::LightsUi() const
{
	static int lightTypeIdx = 0;
	if (ImGui::Combo("Light Type", &lightTypeIdx, STR_LIGHT_TYPES));  
	if (ImGui::Button("Add Light"))
	{
		switch (lightTypeIdx)
		{
		case 0: mRenderer->mPointLights.emplace_back(); break; 
		case 2: mRenderer->mSpotLights.emplace_back(); break; 
		default: break;  
		}
	}

	ImGui::Separator(); 

	if (ImGui::TreeNode("Point Lights"))
	{
		for (int i = 0; i < mRenderer->mPointLights.size(); i++)  
		{
			std::string strLightIdx = "##" + std::to_string(i);
			if (ImGui::TreeNode(("Point Light" + strLightIdx).c_str()))
			{
				PointLight& l = mRenderer->mPointLights[i];  
				std::string strLightIdx = "##" + std::to_string(i); 
				if (ImGui::DragFloat3(("Position" + strLightIdx).c_str(), l.mPosition.cell, 0.005f))	mRenderer->ResetAccumulator(); 
				if (ImGui::ColorEdit3(("Color" + strLightIdx).c_str(), l.mColor.cell))					mRenderer->ResetAccumulator();
				if (ImGui::DragFloat(("Strength" + strLightIdx).c_str(), &l.mStrength, 0.005f, 0.0f))	mRenderer->ResetAccumulator();
				ImGui::TreePop();
			}
		}
		ImGui::TreePop(); // Points Lights 
	}
}

void Ui::MaterialUi(Material2& m, int const instIdx) const
{
	static int type = m.type;
	if (ImGui::Combo("Type", &type, STR_MATERIAL_TYPES))
	{
		mRenderer->mBVHScene.SetInstanceMaterial(instIdx, type);
	}
	switch (MATERIAL_TYPES_DIELECTRIC)
	{
	case MATERIAL_TYPES_GLOSSY:
	{
		ImGui::ColorEdit3("Albedo", m.glossy.albedo.cell);
		ImGui::DragFloat("Glossiness", &m.glossy.glossiness, 0.005f, 0.0f, 1.0f);
		break;
	}
	case MATERIAL_TYPES_DIELECTRIC:
	{
		ImGui::ColorEdit3("Absorption", m.dielectric.absorption.cell);
		ImGui::DragFloat("IOR", &m.dielectric.ior, 0.005f, 1.0f, 3.0f);
		break;
	}
	case MATERIAL_TYPES_TEXTURED:
	{
		if (ImGui::CollapsingHeader("Texture"))
		{
			TextureUi(m.textured.texture, 0);
		}
		break;
	}
	}
}