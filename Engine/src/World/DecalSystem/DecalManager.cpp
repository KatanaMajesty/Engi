#include "DecalManager.h"

#include <algorithm>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/StaticMesh.h"
#include "Renderer/Renderer.h"
#include "Renderer/Material.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/Model.h"
#include "Renderer/ModelRegistry.h"
#include "Renderer/DynamicBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/ImmutableBuffer.h"
#include "Renderer/ConstantBuffer.h"

namespace engi
{

	struct alignas(16) GpuDecal
	{
		math::Mat4x4 decalToWorld;
		math::Mat4x4 worldToDecal;
		math::Vec3 albedo;
		float metalness;
		math::Vec3 emissive;
		float roughness;
		uint32_t parentInstanceID;
	};

	struct alignas(16) GpuDecalGlobalProperties
	{
		float normalThreshold;
	};

	DecalManager::DecalManager(Renderer* renderer)
		: m_renderer(renderer)
	{
		ENGI_ASSERT(renderer);
	}

	bool DecalManager::init(InstanceTable* instanceTable) noexcept
	{
		if (!instanceTable)
		{
			ENGI_LOG_ERROR("Failed to initialize DecalManager: Provided instance table was NULL");
			return false;
		}

		m_instanceTable = instanceTable;

		using namespace gfx;

		ShaderLibrary* shaderLibrary = m_renderer->getShaderLibrary();
		ShaderProgram* shader = shaderLibrary->createProgram("GBuffer_Decal.hlsl", false, false);
		shader->setAttributeLayout({ {
			GpuInputAttributeDesc("STATIC_MESH_POSITION", 0, GpuFormat::RGB32F, 0, true, offsetof(StaticMeshVertex, position)),
			GpuInputAttributeDesc("STATIC_MESH_NORMAL", 0, GpuFormat::RGB32F, 0, true, offsetof(StaticMeshVertex, normal)),
			GpuInputAttributeDesc("DECAL_TO_WORLD", 0, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, decalToWorld) + 0),
			GpuInputAttributeDesc("DECAL_TO_WORLD", 1, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, decalToWorld) + 16),
			GpuInputAttributeDesc("DECAL_TO_WORLD", 2, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, decalToWorld) + 32),
			GpuInputAttributeDesc("DECAL_TO_WORLD", 3, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, decalToWorld) + 48),
			GpuInputAttributeDesc("WORLD_TO_DECAL", 0, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, worldToDecal) + 0),
			GpuInputAttributeDesc("WORLD_TO_DECAL", 1, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, worldToDecal) + 16),
			GpuInputAttributeDesc("WORLD_TO_DECAL", 2, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, worldToDecal) + 32),
			GpuInputAttributeDesc("WORLD_TO_DECAL", 3, GpuFormat::RGBA32F, 1, false, offsetof(GpuDecal, worldToDecal) + 48),
			GpuInputAttributeDesc("DECAL_ALBEDO", 0, GpuFormat::RGB32F, 1, false, offsetof(GpuDecal, albedo)),
			GpuInputAttributeDesc("DECAL_METALNESS", 0, GpuFormat::R32F, 1, false, offsetof(GpuDecal, metalness)),
			GpuInputAttributeDesc("DECAL_EMISSIVE", 0, GpuFormat::RGB32F, 1, false, offsetof(GpuDecal, emissive)),
			GpuInputAttributeDesc("DECAL_ROUGHNESS", 0, GpuFormat::R32F, 1, false, offsetof(GpuDecal, roughness)),
			GpuInputAttributeDesc("PARENT_INSTANCE_ID", 0, GpuFormat::R32U, 1, false, offsetof(GpuDecal, parentInstanceID)),
			} });

		MaterialRegistry* materialRegsitry = m_renderer->getMaterialRegistry();
		m_decalMaterial = materialRegsitry->registerMaterial("ENGI_Decal_Mat");
		m_decalMaterial->setShader(shader);
		m_decalMaterial->getDesc().independentBlend = true;
		m_decalMaterial->getBlendState(0).blendEnabled = true; // Albedo
		m_decalMaterial->getBlendState(1).blendEnabled = false; // Normals (we will do that manually)
		m_decalMaterial->getBlendState(2).blendEnabled = true; // Emissive
		m_decalMaterial->getBlendState(3).blendEnabled = true; // Roughness-Metalness
		m_decalMaterial->getDepthStencilState().depthEnabled = false;
		m_decalMaterial->getRasterizerState().culling = CULLING_FRONT;
		m_decalMaterial->getRasterizerState().ccwFront = true;
		if (!m_decalMaterial->init())
		{
			ENGI_LOG_WARN("Failed to initialize DecalManager: Failed to create decal material");
			return false;
		}

		ModelRegistry* modelRegistry = m_renderer->getModelRegistry();
		m_unitCubeModel = modelRegistry->getModel(MODEL_TYPE_CUBE);
		ENGI_ASSERT(m_unitCubeModel && "Internal model registry error");

		// By default 8 instances
		m_bufferCapacity = 8;
		m_bufferInstances = 0;
		m_instanceBuffer = makeUnique<DynamicBuffer>(m_renderer->createDynamicBuffer("DecalManager::InstanceBuffer", nullptr, m_bufferCapacity, sizeof(GpuDecal)));
		ENGI_ASSERT(m_instanceBuffer);

		m_decalProperties = makeUnique<ConstantBuffer>(m_renderer->createConstantBuffer("DecalManager::Properties", sizeof(GpuDecalGlobalProperties)));
		if (!m_decalProperties)
		{
			ENGI_LOG_WARN("Failed to create Decal manager property buffer");
			return false;
		}

		return true;
	}

	void DecalManager::update() noexcept
	{
		if (m_bufferInstances >= m_bufferCapacity)
		{
			// Perform resize;
			if (!this->resizeInstanceBuffer())
			{
				ENGI_LOG_CRITICAL("Failed to perform instance buffer resize when requested! skipping the update");
				return;
			}
		}

		uint32_t copiedInstances = 0;
		GpuDecal* mapping = reinterpret_cast<GpuDecal*>(m_instanceBuffer->map());
		for (const DecalRenderGroup& renderGroup : m_decalGroups)
		{
			for (uint32_t decalID : renderGroup.decalIDs)
			{
				const DecalInstance& decal = this->getDecal(decalID);
				GpuDecal& currentGpuDecal = mapping[copiedInstances++];
				currentGpuDecal.albedo = decal.albedo;
				currentGpuDecal.emissive = decal.emissive;
				currentGpuDecal.metalness = decal.metalness;
				currentGpuDecal.roughness = decal.roughness;
				currentGpuDecal.worldToDecal = decal.getWorldToDecal();
				currentGpuDecal.decalToWorld = decal.getDecalToWorld();
				currentGpuDecal.parentInstanceID = decal.parentInstanceID;
			}
		}
		
		ENGI_ASSERT(copiedInstances == m_bufferInstances && "Incorrect instance placement or instance count in DecalManager");
		m_instanceBuffer->unmap();
	}

	void DecalManager::render(Texture2D* gbufferObjectID, Texture2D* gbufferNormalResource, Texture2D* depthBufferResource) noexcept
	{
		ENGI_ASSERT(gbufferObjectID && gbufferNormalResource && depthBufferResource);

		m_renderer->bindShaderResource2D(gbufferObjectID, 25, gfx::PIXEL_SHADER);
		m_renderer->bindShaderResource2D(depthBufferResource, 26, gfx::PIXEL_SHADER);
		m_renderer->bindShaderResource2D(gbufferNormalResource, 27, gfx::PIXEL_SHADER);
		m_decalMaterial->bind();
		m_unitCubeModel->getIBO()->bind(0);
		m_unitCubeModel->getVBO()->bind(0, 0);
		m_instanceBuffer->bind(1, 0);

		GpuDecalGlobalProperties properties;
		properties.normalThreshold = math::cos(math::toRadians(m_normalThreshold));
		m_decalProperties->upload(0, &properties, sizeof(GpuDecalGlobalProperties));
		m_decalProperties->bind(4, gfx::PIXEL_SHADER);

		uint32_t numRenderedInstances = 0;
		for (const DecalRenderGroup& renderGroup : m_decalGroups)
		{
			renderGroup.materialInstance.bindTexture(TEXTURE_NORMAL, 1, gfx::PIXEL_SHADER);

			uint32_t numInstances = static_cast<uint32_t>(renderGroup.decalIDs.size());

			const MeshRange& range = m_unitCubeModel->getStaticMeshEntries()[0].range;
			m_renderer->drawInstancedIndexed(range.numIndices, numInstances, range.iboOffset, range.vboOffset, numRenderedInstances);
			numRenderedInstances += numInstances;
		}

		ENGI_ASSERT(numRenderedInstances == m_bufferInstances && "Incorrect amount of decals was rendered");

		m_renderer->bindShaderResource2D(nullptr, 25, gfx::PIXEL_SHADER);
		m_renderer->bindShaderResource2D(nullptr, 26, gfx::PIXEL_SHADER);
		m_renderer->bindShaderResource2D(nullptr, 27, gfx::PIXEL_SHADER);
	}

	uint32_t DecalManager::createDecal(uint32_t instanceID, Texture2D* normalMap, const math::Mat4x4& decalToWorld, const math::Vec3& emissive, const math::Vec3& albedo, float metalness, float roughness) noexcept
	{
		ENGI_ASSERT(m_instanceTable && m_instanceTable->isOccupied(instanceID));

		MaterialInstance materialInstance("Decal_Mat", m_decalMaterial);
		
		// REMARK: Right now we only support normal maps
		// This assertion is already present in DecalDesc, but just in case we want it here
		ENGI_ASSERT(normalMap);
		materialInstance.setTexture(TEXTURE_NORMAL, normalMap);
		
		DecalRenderGroup* renderGroup = this->findRenderGroup(materialInstance);
		if (!renderGroup)
			renderGroup = this->createRenderGroup(materialInstance);

		ENGI_ASSERT(renderGroup); // Just a healthy check. May be removed
		
		uint32_t decalID = m_decals.insert(DecalInstance());
		DecalInstance& decalInstance = m_decals[decalID];
		decalInstance.setNormalMap(normalMap);
		decalInstance.setParentInstance(m_instanceTable, instanceID);
		decalInstance.setLocalSpaceTransform(decalToWorld);

		decalInstance.setAlbedo(albedo);
		decalInstance.setEmissive(emissive);
		decalInstance.setMetalness(metalness);
		decalInstance.setRoughness(roughness);
		ENGI_ASSERT(decalInstance.isValid());

		++m_bufferInstances;
		renderGroup->submitDecal(decalID);
		return decalID;
	}

	const DecalInstance& DecalManager::getDecal(uint32_t decalID) const noexcept
	{
		ENGI_ASSERT(m_decals.isOccupied(decalID));
		return m_decals[decalID];
	}

	void DecalManager::removeDecal(uint32_t decalID) noexcept
	{
#if 1 // Healthy check of runtime
		if (decalID == uint32_t(-1))
		{
			ENGI_LOG_WARN("Provided invalid decalID == uint32_t(-1)");
			return;
		}
#endif

		ENGI_ASSERT(m_decals.isOccupied(decalID) && "No decal here! Something is wrong");

		for (DecalRenderGroup& renderGroup : m_decalGroups)
		{
			if (renderGroup.removeDecal(decalID))
			{
				--m_bufferInstances;
			}

			// If there are no more decals left in the render group - remove it.
			// It may be done in constant complexity by using std library, but im lazy, thus we will do this in linear complexity
			if (renderGroup.decalIDs.empty())
				this->removeRenderGroup(renderGroup.materialInstance);
		}

		m_decals.erase(decalID);
	}

	void DecalManager::DecalRenderGroup::submitDecal(uint32_t decalID) noexcept
	{
		// Runtime healthy check
		// We don't want to have 2 decalIDs in a single render group
		// Basically we don't want to render 1 decal twice in neither of groups but its harder to check in different render groups
#if 1 
		ENGI_ASSERT(std::ranges::find(this->decalIDs, decalID) == this->decalIDs.end());
#endif
		this->decalIDs.push_back(decalID);
	}

	bool DecalManager::DecalRenderGroup::removeDecal(uint32_t decalID) noexcept
	{
		auto it = std::ranges::find(this->decalIDs, decalID);

		// The iterator pos must be valid and dereferenceable
		// Thus the end() iterator (which is valid, but is not dereferenceable) cannot be used as a value for pos
		if (it != this->decalIDs.end())
		{
			this->decalIDs.erase(it);
			return true;
		}

		return false;
	}

	DecalManager::DecalRenderGroup* DecalManager::findRenderGroup(const MaterialInstance& materialInstance) noexcept
	{
		auto it = std::ranges::find_if(m_decalGroups, [&](const DecalRenderGroup& rg) -> bool
			{
				return rg.materialInstance == materialInstance;
			});
		return it == m_decalGroups.end() ? nullptr : &*it;
	}

	DecalManager::DecalRenderGroup* DecalManager::createRenderGroup(const MaterialInstance& materialInstance) noexcept
	{
		DecalRenderGroup& renderGroup = m_decalGroups.emplace_back();
		renderGroup.materialInstance = materialInstance;
		return &renderGroup;
	}

	void DecalManager::removeRenderGroup(const MaterialInstance& materialInstance) noexcept
	{
		auto it = std::ranges::find_if(m_decalGroups, [&](const DecalRenderGroup& rg) -> bool
			{
				return rg.materialInstance == materialInstance;
			});

		if (it != m_decalGroups.end())
			m_decalGroups.erase(it);
	}

	bool DecalManager::resizeInstanceBuffer() noexcept
	{
		uint32_t newCap = m_bufferCapacity * 2;
		m_bufferCapacity = (m_bufferInstances > newCap) ? m_bufferInstances + 1 : newCap;
		DynamicBuffer* buffer = m_renderer->createDynamicBuffer("DecalManager::InstanceBuffer", nullptr, m_bufferCapacity, sizeof(GpuDecal));
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

}; // engi namespace