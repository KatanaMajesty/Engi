#pragma once

#include <map>
#include <vector>
#include "Utility/SolidVector.h"
#include "Utility/Memory.h"
#include "Renderer/Model.h"
#include "Renderer/Material.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/ShaderProgram.h"

namespace engi
{

	class Renderer;
	class ConstantBuffer;
	class DynamicBuffer;
	class InstanceTable;

	class RenderBatch
	{
	public:
		RenderBatch(const MaterialInstance& materialInstance);
		~RenderBatch() = default;

		const MaterialInstance& getMaterialInstance() const noexcept { return m_materialInstance; }
		uint32_t getInstanceCount() const noexcept { return static_cast<uint32_t>(m_instanceDataIDs.size()); }
		void submitInstanceData(uint32_t instanceDataId) noexcept;
		bool removeInstanceData(uint32_t instanceDataId) noexcept;
		const auto& getAllInstanceIDs() const noexcept { return m_instanceDataIDs; }
		bool isEmpty() const noexcept { return getInstanceCount() == 0; }

	private:
		MaterialInstance m_materialInstance;
		std::vector<uint32_t> m_instanceDataIDs;
	};

	struct MeshGroup
	{
		RenderBatch* getRenderBatch(const MaterialInstance& materialInstance) noexcept;
		RenderBatch* addRenderBatch(const MaterialInstance& materialInstance) noexcept;
		bool removeRenderBatch(const MaterialInstance& materialInstance) noexcept;
		const auto& getAllRenderBatches() const noexcept { return renderBatches; }

		std::vector<RenderBatch> renderBatches;
	};

	class ModelGroup
	{
	public:
		ModelGroup() = default; // TODO: Remove
		ModelGroup(uint32_t numMeshes);
		~ModelGroup() = default;

		MeshGroup* getMeshGroup(uint32_t meshIndex) noexcept;
		const MeshGroup* getMeshGroup(uint32_t meshIndex) const noexcept;
		uint32_t getNumMeshes() const noexcept { return static_cast<uint32_t>(m_meshes.size()); }
		const auto& getAllMeshGroups() const noexcept { return m_meshes; }
	
	private:
		std::vector<MeshGroup> m_meshes;
	};

	struct MaterialGroup
	{
		ModelGroup* getModelGroup(const SharedHandle<Model>& model) noexcept;
		ModelGroup* addModelGroup(const SharedHandle<Model>& model) noexcept;
		const auto& getAllModelGroups() const noexcept { return modelMap; }

		std::map<SharedHandle<Model>, ModelGroup> modelMap;
	};

	class MeshManager
	{
	public:
		static std::array<gfx::GpuInputAttributeDesc, 21> getInputAttributes(uint32_t perVertexSlot, uint32_t perInstanceSlot) noexcept;

		MeshManager(Renderer* renderer, InstanceTable* instanceTable);
		MeshManager(const MeshManager&) = delete;
		MeshManager& operator=(const MeshManager&) = delete;
		~MeshManager();

		bool init() noexcept;
		void render() noexcept;
		void renderUsingMaterial(const SharedHandle<Material>& material) noexcept;
		
		bool submitInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceID) noexcept;
		bool removeInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceID) noexcept;
		bool updateInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceID, const MaterialInstance& newMat) noexcept;
		bool updateInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceID) noexcept;
		
		inline InstanceTable* getInstanceTable() noexcept { return m_instanceTable; }
		inline constexpr uint32_t getNumInstances() const noexcept { return m_bufferInstances; }
		inline constexpr void requestBufferUpdate() noexcept { m_bufferUpdateRequested = true; }

	private:
		bool isValid(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceDataId) const noexcept;
		bool updateInstanceBuffer() noexcept;
		bool resizeInstanceBuffer() noexcept;
		uint32_t renderMaterialGroup(MaterialGroup& materialGroup, uint32_t instanceOffset) noexcept;

		Renderer* m_renderer;
		InstanceTable* m_instanceTable;

		uint32_t m_bufferCapacity = 0;
		uint32_t m_bufferInstances = 0;
		bool m_bufferUpdateRequested = false;
		
		ShaderProgram* m_layoutProgram = nullptr;
		UniqueHandle<DynamicBuffer> m_instanceBuffer = nullptr;
		UniqueHandle<ConstantBuffer> m_meshData = nullptr; // TODO: Remove this
		UniqueHandle<ConstantBuffer> m_materialData = nullptr;
		
		std::map<SharedHandle<Material>, MaterialGroup> m_materialMap;
	};

}; // engi namespace