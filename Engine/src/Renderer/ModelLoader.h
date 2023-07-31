#pragma once

#include <vector>
#include <unordered_map>
#include "Utility/Memory.h"
#include "Core/FileSystem.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/Material.h"
#include "Renderer/ModelRegistry.h"
#include "Renderer/TextureLoader.h"
#include "Renderer/MaterialRegistry.h"

namespace engi
{

	namespace gfx { class IGpuDevice; }

	struct ParsedModelInfo
	{
		SharedHandle<Model> model;
		std::vector<MaterialInstance> materials;
	};

	class ModelLoader
	{
	public:
		ModelLoader(TextureLoader* textureLoader, ModelRegistry* modelRegistry, MaterialRegistry* materialRegistry);

		ParsedModelInfo* loadFromFBX(const std::string& filepath, const std::string& name, const SharedHandle<Material>& material) noexcept;
		ParsedModelInfo* getParsedModel(const std::string& filepath) noexcept;

	private:
		TextureLoader* m_textureLoader;
		ModelRegistry* m_modelRegistry;
		MaterialRegistry* m_materialRegistry;
		std::unordered_map<std::string, ParsedModelInfo> m_parsedFiles;
	};

}; // engi namespace