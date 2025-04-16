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
		if (ImGui::BeginTabItem("Debug"))			{ Debug();			ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Enhancements"))	{ Enhancements();	ImGui::EndTabItem(); }
	}

	ImGui::End(); 
}

void Ui::Debug() const
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
}

void Ui::Enhancements() const
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
