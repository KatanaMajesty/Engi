#pragma once

#include <filesystem>
#include <unordered_map>
#include <map>
#include "Utility/ArrayView.h"
#include "Renderer/Model.h"
#include "Renderer/StaticMesh.h"

namespace engi
{

	namespace gfx { class IGpuDevice; }

	// ModelType enum represents models that are handled by Engi and may be used by user
	enum ModelType
	{
		MODEL_TYPE_CUBE,
		MODEL_TYPE_SPHERE,
	};

	class ModelRegistry
	{
	public:
		ModelRegistry(gfx::IGpuDevice* device);
		ModelRegistry(const ModelRegistry&) = delete;
		ModelRegistry& operator=(const ModelRegistry&) = delete;
		~ModelRegistry();

		bool init() noexcept;
		SharedHandle<Model> addModel(const std::string& name, uint32_t numMeshes) noexcept;
		SharedHandle<Model> addModel(const std::string& name, const std::string& filepath, uint32_t numMeshes) noexcept;
		SharedHandle<Model> addModel(ModelType type, uint32_t numMeshes) noexcept;
		SharedHandle<Model> getModel(const std::string& name) noexcept;
		SharedHandle<Model> getModel(ModelType type) noexcept;
		bool removeModel(const std::string& filepath) noexcept;
		bool removeModel(ModelType type) noexcept;
		inline const uint32_t getNumModels() const noexcept { return static_cast<uint32_t>(m_loadedModels.size()); }
		const auto& getAllModels() const noexcept { return m_loadedModels; }

	private:
		gfx::IGpuDevice* m_device;
		std::unordered_map<std::string, SharedHandle<Model>> m_loadedModels;
	};

}; // engi namespace