#pragma once

#include "materials.h" 
#include "textures.h" 
#include "bvh_scene.h"

namespace Tmpl8
{
	class Renderer;
}

static constexpr ImGuiWindowFlags UI_WINDOW_FLAGS =
ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoMove |
ImGuiWindowFlags_NoFocusOnAppearing |
ImGuiWindowFlags_NoBringToFrontOnFocus;

inline auto constexpr STR_RENDER_MODES = "Normals\0Depth\0Albedo\0Shaded\0";
inline auto constexpr STR_CONVERGE_MODES = "None\0Accumulation\0Reprojection\0";
inline auto constexpr STR_SAMPLE_MODES = "None\0Unsafe/Fast\0Looped\0Clamped\0";
inline auto constexpr STR_FILTER_MODES = "None\0Nearest\0Linear\0";
inline auto constexpr STR_LIGHT_TYPES = "Point\0Directional\0Spot\0";
inline auto constexpr STR_MATERIAL_TYPES = "Diffuse\0Metallic\0Dielectric\0Glossy\0Textured\0";

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
	void	MaterialUi(Material2& material, int const instIdx = 0) const;
	void	InstanceUi(BVHScene& scene, int const instIdx) const;
	void	SceneUi(BVHScene& scene) const;
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

inline void Ui::InstanceUi(BVHScene& s, int const instIdx) const
{
	ImGui::Text("InstIdx: %d", instIdx);
	if (ImGui::CollapsingHeader("Material"))
	{
		MaterialUi(s.mMatCopies[instIdx]);
	}
}

inline void Ui::SceneUi(BVHScene& s) const 
{
	ImGui::Text("Inst Count: %d", s.mInstances.size());
	for (int i = 0; i < s.mInstances.size(); i++)
	{
		if (ImGui::TreeNode(std::to_string(i).c_str()))  
		{
			InstanceUi(s, i); 
			ImGui::TreePop(); 
		}
	}
}

