#include "Renderer/MeshManager.h"

#include "Math/Vec3.h"
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/Renderer.h"
#include "Renderer/InstanceTable.h"
#include "Renderer/DynamicBuffer.h"
#include "Renderer/ConstantBuffer.h"
#include "Renderer/InstanceData.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/ImmutableBuffer.h"

namespace engi
{

	RenderBatch::RenderBatch(const MaterialInstance& materialInstance)
		: m_materialInstance(materialInstance)
	{
		ENGI_ASSERT(!materialInstance.isEmpty() && "Material instance cannot be empty");
	}

	void RenderBatch::submitInstanceData(uint32_t instanceDataId) noexcept
	{
		m_instanceDataIDs.push_back(instanceDataId);
	}

	bool RenderBatch::removeInstanceData(uint32_t instanceDataId) noexcept
	{
		auto it = std::ranges::find(m_instanceDataIDs, instanceDataId);
		if (it == m_instanceDataIDs.end())
			return false;

		m_instanceDataIDs.erase(it);
		return true;
	}

	RenderBatch* MeshGroup::getRenderBatch(const MaterialInstance& materialInstance) noexcept
	{
		auto it = std::ranges::find_if(this->renderBatches, [&](const auto& batch)
			{
				return batch.getMaterialInstance() == materialInstance;
			});

		if (it == this->renderBatches.end())
			return nullptr;

		return &(*it);
	}

	RenderBatch* MeshGroup::addRenderBatch(const MaterialInstance& materialInstance) noexcept
	{
		RenderBatch* batch = getRenderBatch(materialInstance);
		if (batch)
			return batch;

		this->renderBatches.push_back(RenderBatch(materialInstance));
		return &this->renderBatches.back();
	}

	bool MeshGroup::removeRenderBatch(const MaterialInstance& materialInstance) noexcept
	{
		auto it = std::ranges::find_if(this->renderBatches, [&](const auto& batch)
			{
				return batch.getMaterialInstance() == materialInstance;
			});

		if (it == this->renderBatches.end())
			return false;

		this->renderBatches.erase(it);
		return true;
	}

	ModelGroup::ModelGroup(uint32_t numMeshes)
		: m_meshes(static_cast<size_t>(numMeshes), MeshGroup())
	{
	}

	const MeshGroup* ModelGroup::getMeshGroup(uint32_t meshIndex) const noexcept
	{
		ENGI_ASSERT(meshIndex < getNumMeshes());
		return &m_meshes[meshIndex];
	}

	MeshGroup* ModelGroup::getMeshGroup(uint32_t meshIndex) noexcept
	{
		ENGI_ASSERT(meshIndex < getNumMeshes());
		return &m_meshes[meshIndex];
	}

	ModelGroup* MaterialGroup::getModelGroup(const SharedHandle<Model>& model) noexcept
	{
		auto it = this->modelMap.find(model);
		if (it == this->modelMap.end())
			return nullptr;

		return &(it->second);
	}

	ModelGroup* MaterialGroup::addModelGroup(const SharedHandle<Model>& model) noexcept
	{
		ModelGroup* group = getModelGroup(model);
		if (group)
			return group;

		ENGI_ASSERT(model->getNumStaticMeshes() != 0);
		this->modelMap[model] = ModelGroup(model->getNumStaticMeshes());
		return &this->modelMap.at(model);
	}

	std::array<gfx::GpuInputAttributeDesc, 21> MeshManager::getInputAttributes(uint32_t perVertexSlot, uint32_t perInstanceSlot) noexcept
	{
		std::array<gfx::GpuInputAttributeDesc, 21> layout;
		std::ranges::copy(StaticMeshVertex::getInputAttributes(perVertexSlot), layout.begin());
		std::ranges::copy(InstanceData::getInputAttributes(perInstanceSlot), layout.begin() + 5);
		return layout;
	}

	MeshManager::MeshManager(Renderer* renderer, InstanceTable* instanceTable)
		: m_renderer(renderer)
		, m_instanceTable(instanceTable)
	{
		ENGI_ASSERT(renderer && "Logical device cannot be nullptr");
		ENGI_ASSERT(instanceTable && "Instance data table cannot be nullptr");
	}

	MeshManager::~MeshManager()
	{
		// TODO: Clean data table
	}

	struct alignas(16) MeshData
	{
		math::Mat4x4 meshToModel;
		math::Mat4x4 modelToMesh;
		uint32_t hasTexCoords;
	};

	struct alignas(16) ENGI_MaterialData
	{
		ENGI_MaterialData(const MaterialConstant& materialCB)
			: roughness(materialCB.roughness)
			, metallic(materialCB.metallic)
			, useAlbedoTexture(materialCB.useAlbedoTexture)
			, useNormalMap(materialCB.useNormalMap)
			, useMetalnessMap(materialCB.useMetalnessMap)
			, useRoughnessMap(materialCB.useRoughnessMap)
		{
		}

		uint32_t useAlbedoTexture;
		uint32_t useNormalMap;
		uint32_t useMetalnessMap;
		uint32_t useRoughnessMap;
		float roughness;
		float metallic;
	};

	bool MeshManager::init() noexcept
	{
		m_materialMap.clear();
		m_bufferCapacity = 64;
		m_bufferInstances = 0;

		m_instanceBuffer.reset(m_renderer->createDynamicBuffer("MeshManager::InstanceBuffer", nullptr, m_bufferCapacity, sizeof(InstanceData)));
		if (!m_instanceBuffer)
		{
			ENGI_LOG_ERROR("Failed to init instnace buffer");
			return false;
		}

		m_meshData.reset(m_renderer->createConstantBuffer("MeshManager::MeshData", sizeof(MeshData)));
		if (!m_meshData)
		{
			ENGI_LOG_ERROR("Failed to initialize mesh data");
			return false;
		}

		m_materialData.reset(m_renderer->createConstantBuffer("MeshManager::MaterialData", sizeof(ENGI_MaterialData)));
		if (!m_materialData)
		{
			ENGI_LOG_ERROR("Failed to init material data");
			return false;
		}
		return true;
	}

	void MeshManager::render() noexcept
	{
		if (m_bufferUpdateRequested)
			updateInstanceBuffer();

		m_instanceBuffer->bind(1, 0);
		uint32_t numRenderedInstances = 0;
		for (auto& [material, materialGroup] : m_materialMap)
		{
			material->bind();
			numRenderedInstances += renderMaterialGroup(materialGroup, numRenderedInstances);
		}
		ENGI_ASSERT(numRenderedInstances == m_bufferInstances && "Internal error");
	}

	void MeshManager::renderUsingMaterial(const SharedHandle<Material>& material) noexcept
	{
		if (!material)
			return;

		if (m_bufferUpdateRequested)
			updateInstanceBuffer();

		m_instanceBuffer->bind(1, 0);
		material->bind();
		uint32_t numRenderedInstances = 0;
		for (auto& [material, materialGroup] : m_materialMap)
			numRenderedInstances += renderMaterialGroup(materialGroup, numRenderedInstances);

		ENGI_ASSERT(numRenderedInstances == m_bufferInstances && "Internal error");
	}

	bool MeshManager::submitInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceDataId) noexcept
	{
		if (!isValid(model, meshIndex, material, instanceDataId))
			return false;

		MaterialGroup& materialGroup = m_materialMap[material.getMaterial()];
		ModelGroup* modelGroup = materialGroup.addModelGroup(model);
		MeshGroup* meshGroup = modelGroup->getMeshGroup(meshIndex);
		RenderBatch* rb = meshGroup->addRenderBatch(material);
		rb->submitInstanceData(instanceDataId);
		++m_bufferInstances;

		requestBufferUpdate();
		return true;
	}

	bool MeshManager::removeInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceDataId) noexcept
	{
		if (!isValid(model, meshIndex, material, instanceDataId))
			return false;

		auto it = m_materialMap.find(material.getMaterial());
		if (it == m_materialMap.end())
			return false;

		MaterialGroup& materialGroup = it->second;
		ModelGroup* modelGroup = materialGroup.getModelGroup(model);
		if (!modelGroup)
			return false;

		MeshGroup* meshGroup = modelGroup->getMeshGroup(meshIndex);
		if (!meshGroup)
			return false;

		RenderBatch* rb = meshGroup->getRenderBatch(material);
		if (!rb)
			return false;

		if (!rb->removeInstanceData(instanceDataId))
			return false;

		if (rb->isEmpty())
			meshGroup->removeRenderBatch(material);

		--m_bufferInstances;
		requestBufferUpdate();
		return true;
	}

	bool MeshManager::updateInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceDataId, const MaterialInstance& newMat) noexcept
	{
		if (!isValid(model, meshIndex, material, instanceDataId))
			return false;

		bool successfullyRemoved = removeInstance(model, meshIndex, material, instanceDataId);
		if (!submitInstance(model, meshIndex, newMat, instanceDataId))
			return false;

		//ENGI_ASSERT(successfullyRemoved);
		return successfullyRemoved;
	}

	bool MeshManager::updateInstance(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceDataId) noexcept
	{
		if (!isValid(model, meshIndex, material, instanceDataId))
			return false;

		requestBufferUpdate();
		return true;
	}

	bool MeshManager::isValid(const SharedHandle<Model>& model, uint32_t meshIndex, const MaterialInstance& material, uint32_t instanceDataId) const noexcept
	{
		if (!model || material.isEmpty())
			return false;

		uint32_t numMeshes = model->getNumStaticMeshes();
		ENGI_ASSERT(numMeshes != 0 && "Weird model was loaded");
		if (meshIndex >= numMeshes)
			return false;

		ENGI_ASSERT(m_instanceTable->isOccupied(instanceDataId) && "Invalid id provided");
		return true;
	}

	bool MeshManager::updateInstanceBuffer() noexcept
	{
		m_bufferUpdateRequested = false;
		if (m_bufferInstances >= m_bufferCapacity)
		{
			if (!resizeInstanceBuffer())
				return false;
		}
		uint32_t copiedInstances = 0;
		InstanceData* mapping = reinterpret_cast<InstanceData*>(m_instanceBuffer->map());
		for (auto& [material, matGroup] : m_materialMap)
		{
			for (auto& [model, modelGroup] : matGroup.getAllModelGroups())
			{
				for (const MeshGroup& meshGroup : modelGroup.getAllMeshGroups())
				{
					for (const RenderBatch& rb : meshGroup.getAllRenderBatches())
					{
						for (uint32_t instanceDataId : rb.getAllInstanceIDs())
						{
							InstanceData& data = m_instanceTable->getInstanceData(instanceDataId);
							mapping[copiedInstances] = data;
							++copiedInstances;
						}
					}
				}
			}
		}
		ENGI_ASSERT(copiedInstances == m_bufferInstances && "Internal error");
		m_instanceBuffer->unmap();
		return true;
	}

	bool MeshManager::resizeInstanceBuffer() noexcept
	{
		uint32_t newCap = m_bufferCapacity * 2;
		m_bufferCapacity = (m_bufferInstances > newCap) ? m_bufferInstances + 1 : newCap;
		DynamicBuffer* buffer = m_renderer->createDynamicBuffer("MeshManager::InstanceBuffer", nullptr, m_bufferCapacity, sizeof(InstanceData));
		if (!buffer)
		{
			ENGI_LOG_WARN("Failed to resize instance buffer");
			delete buffer;
			return false;
		}

		buffer->copyFrom(m_instanceBuffer.get(), 0);
		m_instanceBuffer.reset(buffer);
		return true;
	}

	uint32_t MeshManager::renderMaterialGroup(MaterialGroup& materialGroup, uint32_t instanceOffset) noexcept
	{
		using namespace gfx;

		uint32_t resultOffset = 0;
		for (auto& [model, modelGroup] : materialGroup.getAllModelGroups())
		{
			model->getVBO()->bind(0, 0);
			model->getIBO()->bind(0);

			uint32_t numMeshes = modelGroup.getNumMeshes();
			for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
			{
				const MeshGroup* meshGroup = modelGroup.getMeshGroup(meshIndex);
				ENGI_ASSERT(meshGroup);

				const StaticMeshEntry& meshEntry = model->getStaticMeshEntries()[meshIndex];
				const MeshRange& meshRange = meshEntry.range;

				MeshData meshData;
				meshData.meshToModel = meshEntry.mesh.getMeshToModel();
				meshData.modelToMesh = meshEntry.mesh.getModelToMesh();
				meshData.hasTexCoords = (uint32_t)meshEntry.mesh.hasTexCoords();
				m_meshData->upload(0, &meshData, sizeof(MeshData));
				m_meshData->bind(1, VERTEX_SHADER | PIXEL_SHADER);

				for (const RenderBatch& rb : meshGroup->getAllRenderBatches())
				{
					uint32_t numInstances = rb.getInstanceCount();
					if (numInstances == 0)
						continue;

					const MaterialInstance& mi = rb.getMaterialInstance();
					mi.bindTexture(TEXTURE_ALBEDO, 0, PIXEL_SHADER);
					mi.bindTexture(TEXTURE_NORMAL, 1, PIXEL_SHADER);
					mi.bindTexture(TEXTURE_METALNESS, 2, PIXEL_SHADER);
					mi.bindTexture(TEXTURE_ROUGHNESS, 3, PIXEL_SHADER);
					
					ENGI_MaterialData materialData(mi.getData());
					m_materialData->upload(0, &materialData, sizeof(ENGI_MaterialData));
					m_materialData->bind(2, VERTEX_SHADER | PIXEL_SHADER);

					m_renderer->drawInstancedIndexed(meshRange.numIndices, numInstances, meshRange.iboOffset, meshRange.vboOffset, instanceOffset);
					resultOffset += numInstances;
					instanceOffset += numInstances;
				}
			}
		}
		return resultOffset;
	}

}; // engi namespace