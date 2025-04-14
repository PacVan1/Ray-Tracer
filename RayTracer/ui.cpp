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
}

void Ui::Lights() const
{
}
