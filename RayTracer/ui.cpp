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
	ImGui::Begin("Debug", nullptr, UI_WINDOW_FLAGS); 

	ImGui::Text("Performance report"); 
	ImGui::Text("%5.2fms (%.1ffps) - %.1fMrays/s\n", mRenderer->GetAvg(), mRenderer->GetFps(), mRenderer->GetRps() / 1000);

	ImGui::Separator();

	static int renderMode; 
	if (ImGui::Combo("Render Mode", &renderMode, STR_RENDER_MODES))
	{
		mRenderer->SetRenderMode(renderMode); 
	}

	ImGui::Checkbox("TIR", &mRenderer->mTIR); 

	DebugViewer(); 
}

void Ui::DebugViewer() const
{
	ImGui::Checkbox("Active", &mRenderer->mDebugViewerActive);
	ImGui::DragFloat("Zoom", &mRenderer->mDebugViewer.mZoom, 0.1f, 1.0f, SCRHEIGHT);
	ImGui::DragFloat2("Position", mRenderer->mDebugViewer.mPosition.cell, 0.05f);
	if (ImGui::SliderInt("Every", &mRenderer->mDebugViewer.mEvery, 1, 15)) 
	{
		mRenderer->mDebugViewer.mSelectionCount = floor(SCRWIDTH / mRenderer->mDebugViewer.mEvery);
		mRenderer->mDebugViewer.mSelected = min(mRenderer->mDebugViewer.mSelectionCount, mRenderer->mDebugViewer.mSelected); 
	}
	ImGui::SliderInt("Selected", &mRenderer->mDebugViewer.mSelected, 0, mRenderer->mDebugViewer.mSelectionCount); 
}
