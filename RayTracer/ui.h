#pragma once

#include "materials.h" 
#include "textures.h" 

namespace Tmpl8
{
	class Renderer; 
}

static constexpr ImGuiWindowFlags UI_WINDOW_FLAGS =
	ImGuiWindowFlags_NoResize				|
	ImGuiWindowFlags_NoMove					|
	ImGuiWindowFlags_NoFocusOnAppearing		|
	ImGuiWindowFlags_NoBringToFrontOnFocus;

inline auto constexpr STR_RENDER_MODES		= "Normals\0Depth\0Albedo\0Shaded\0"; 
inline auto constexpr STR_CONVERGE_MODES	= "None\0Accumulation\0Reprojection\0";
inline auto constexpr STR_SAMPLE_MODES		= "None\0Unsafe/Fast\0Looped\0Clamped\0"; 
inline auto constexpr STR_FILTER_MODES		= "None\0Nearest\0Linear\0";  
inline auto constexpr STR_LIGHT_TYPES		= "Point\0Directional\0Spot\0"; 

class Ui
{
public:
	Renderer* mRenderer;

public:
			Ui();
	void	General() const;

private:
	void	DebugUi() const;
	void	SettingsUi() const;
	void	CameraUi() const;
	void	MaterialsUi() const;
	void	LightsUi() const; 

	template <typename T> 
	void	TextureUi(Texture<T>& texture, int const id = 0) const; 
};

template<typename T>
inline void Ui::TextureUi(Texture<T>& texture, int const id) const    
{
	ImGui::Text("Width: %d", texture.mWidth); 
	ImGui::Text("Height: %d", texture.mHeight); 
	std::string strTextureId = "##" + std::to_string(id);  
	int sampleMode = texture.mSampleMode; 
	if (ImGui::Combo(("Sample Mode" + strTextureId).c_str(), &sampleMode, STR_SAMPLE_MODES)) 
	{
		texture.mSampleMode = sampleMode; 
		mRenderer->ResetAccumulator(); 
	}
	int filterMode = texture.mFilterMode; 
	if (ImGui::Combo(("Filter Mode" + strTextureId).c_str(), &filterMode, STR_FILTER_MODES)) 
	{
		texture.mFilterMode = filterMode; 
		mRenderer->ResetAccumulator();
	}
}
