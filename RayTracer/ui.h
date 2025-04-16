#pragma once

static constexpr ImGuiWindowFlags UI_WINDOW_FLAGS =
	ImGuiWindowFlags_NoResize				|
	ImGuiWindowFlags_NoMove					|
	ImGuiWindowFlags_NoFocusOnAppearing		|
	ImGuiWindowFlags_NoBringToFrontOnFocus;

inline auto constexpr STR_RENDER_MODES = "Normals\0Depth\0Albedo\0Shaded\0";  

namespace Tmpl8
{
	class Renderer; 
}

class Ui
{
public:
	Renderer* mRenderer;

public:
			Ui();
	void	General() const;

private:
	void	Debug() const;
	void	Enhancements() const; 
};

