#include "precomp.h"
#include "ui.h"

#include "renderer.h" 

Ui::Ui() :
	mRenderer(nullptr)
{}

void Ui::General()
{
	static constexpr ImGuiWindowFlags WINDOW_FLAGS =
		ImGuiWindowFlags_NoResize				|
		ImGuiWindowFlags_NoMove					|
		ImGuiWindowFlags_NoFocusOnAppearing		|
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(350, SCRHEIGHT));
	ImGui::Begin("Debug", nullptr, WINDOW_FLAGS); 

	ImGui::Text("%5.2fms (%.1ffps) - %.1fMrays/s\n", mRenderer->GetAvg(), mRenderer->GetFps(), mRenderer->GetRps() / 1000);
}
