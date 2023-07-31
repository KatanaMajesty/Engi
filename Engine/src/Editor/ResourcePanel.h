#pragma once

#include <Core/FileSystem.h>
#include "Utility/Memory.h"
#include "Renderer/Renderer.h"
#include "Renderer/Material.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ModelRegistry.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ModelLoader.h"

namespace engi
{

	class ResourcePanel
	{
	public:
		ResourcePanel(Renderer* renderer);

		void onUpdate() noexcept;
		void drawAllModels() noexcept;
		void drawAllMaterials() noexcept;
		void drawAllShaders() noexcept;
		void drawSelectedModel() noexcept;
		void drawSelectedMaterial() noexcept;
		void drawSelectedProgram() noexcept;
		SharedHandle<Model> getSelectedModel() noexcept { return m_selectedModel; }
		SharedHandle<Material> getSelectedMaterial() noexcept { return m_selectedMaterial; }
		ShaderProgram* getSelectedShader() noexcept { return m_selectedProgram; }

		ParsedModelInfo* LoadFromFBX(const std::string& filename, const SharedHandle<Material>& material = nullptr) noexcept;
		SharedHandle<Material> GetMaterial(MaterialType type) noexcept;
		Texture2D* GetTexture2D(const std::string& filename) noexcept;
		Texture2D* GetTextureAtlas(const std::string& filename, uint32_t numWidth, uint32_t numHeight) noexcept;
		TextureCube* GetTextureCube(const std::string& filename) noexcept;

	private:
		std::filesystem::path m_texturePath = FileSystem::getInstance().getAssetsPath() / "Textures";
		std::filesystem::path m_modelsPath = FileSystem::getInstance().getAssetsPath() / "Models";
		Renderer* m_renderer;
		ShaderProgram* m_selectedProgram = nullptr;
		SharedHandle<Material> m_selectedMaterial = nullptr;
		SharedHandle<Model> m_selectedModel = nullptr;
	};

}; // engi namespace