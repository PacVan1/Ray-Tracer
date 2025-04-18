#pragma once

#include "materials.h" 

namespace Tmpl8
{
	class Renderer; 
}

static constexpr ImGuiWindowFlags UI_WINDOW_FLAGS =
	ImGuiWindowFlags_NoResize				|
	ImGuiWindowFlags_NoMove					|
	ImGuiWindowFlags_NoFocusOnAppearing		|
	ImGuiWindowFlags_NoBringToFrontOnFocus;

inline auto constexpr STR_RENDER_MODES = "Normals\0Depth\0Albedo\0Shaded\0";  

class Ui
{
public:
	Renderer* mRenderer;

public:
			Ui();
	void	General() const;

private:
	void	DebugUi() const;
	void	EnhancementsUi() const;
	void	CameraUi() const;
	void	MaterialsUi() const;
	void	LightsUi() const; 
};

