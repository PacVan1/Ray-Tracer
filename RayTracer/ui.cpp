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

	ImGui::Separator();

	int renderMode = mRenderer->GetRenderMode(); 
	if (ImGui::Combo("Render Mode", &renderMode, STR_RENDER_MODES))
	{
		mRenderer->SetRenderMode(renderMode); 
	}
	int maxBounces = mRenderer->GetMaxBounces();
	if (ImGui::SliderInt("Max Bounces", &maxBounces, 1, 10))
	{
		mRenderer->SetMaxBounces(maxBounces);
	}

	if (ImGui::BeginTabBar("Main"))
	{
		if (ImGui::BeginTabItem("Camera"))			{ CameraUi();		ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Materials"))		{ MaterialsUi();	ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Lights"))			{ LightsUi();		ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Debug"))			{ DebugUi();		ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Enhancements"))	{ EnhancementsUi();	ImGui::EndTabItem(); }
	}

	ImGui::End(); 
}

void Ui::DebugUi() const
{
	if (ImGui::CollapsingHeader("2D Slice Viewer"))
	{
		ImGui::Separator(); 
		ImGui::Checkbox("Active", &mRenderer->mDebugViewerActive);
		ImGui::DragFloat("Zoom", &mRenderer->mDebugViewer.mZoom, 0.1f, 1.0f, SCRHEIGHT);
		ImGui::DragFloat2("Position", mRenderer->mDebugViewer.mPosition.cell, 0.05f);
		if (ImGui::SliderInt("Every", &mRenderer->mDebugViewer.mEvery, 1, 15)) 
		{
			mRenderer->mDebugViewer.SetSelection();
		}
		ImGui::SliderInt("Selected", &mRenderer->mDebugViewer.mSelected, 0, mRenderer->mDebugViewer.mSelectionCount);
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
		ImGui::Checkbox("Active", &mRenderer->mBreakPixelActive);
		ImGui::Text("Pixel Coordinate: (%d, %d)", input.mMousePos.x, input.mMousePos.y);
		ImGui::Separator();
	}
	if (ImGui::CollapsingHeader("Frame Controller"))
	{
		ImGui::Separator();
		ImGui::Checkbox("Active", &mRenderer->mMaxFramesActive); 
		ImGui::DragInt("Number of frames", &mRenderer->mMaxFrames, 1, 1, 1000); 
		ImGui::ProgressBar(static_cast<float>(mRenderer->GetFrame()) / static_cast<float>(mRenderer->mMaxFrames), ImVec2(0.0f, 0.0f), "Progress"); 
		ImGui::Separator();
	}
}

void Ui::EnhancementsUi() const
{
	bool aa = mRenderer->GetAa();
	if (ImGui::Checkbox("Anti-aliasing", &aa))
	{
		mRenderer->SetAa(aa);
	}
	bool accum = mRenderer->GetAccum();
	if (ImGui::Checkbox("Accumulator", &accum)) 
	{
		mRenderer->SetAccum(accum);
	}
	bool dof = mRenderer->GetDof();
	if (ImGui::Checkbox("Depth of field", &dof))
	{
		mRenderer->SetDof(dof);
	}
	bool autoFocus = mRenderer->GetAutoFocus();
	if (ImGui::Checkbox("Auto-focus", &autoFocus))
	{
		mRenderer->SetAutoFocus(autoFocus);
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
	if (ImGui::DragFloat("Speed", &camera.mSpeed, 0.05f, 0.0f, 10.0f));
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
	Dielectric& dielectric = mRenderer->mDielectric;

	ImGui::DragFloat("Refractive Index", &dielectric.mRefractiveIdx, 0.02f, 1.0f, 3.0f);
	ImGui::DragFloat3("Absorption", dielectric.mAbsorption.cell, 0.02f, 0.0f, 10.0f);
}

void Ui::LightsUi() const
{
	if (ImGui::Checkbox("Directional Light", &mRenderer->mDirLightActive))		mRenderer->ResetAccumulator();
	if (ImGui::Checkbox("Point Lights", &mRenderer->mPointLightsActive))		mRenderer->ResetAccumulator();
	if (ImGui::Checkbox("Spot Lights", &mRenderer->mSpotLightsActive))			mRenderer->ResetAccumulator();
	if (ImGui::Checkbox("Skydome Illumination", &mRenderer->mSkydomeActive))	mRenderer->ResetAccumulator();
}
