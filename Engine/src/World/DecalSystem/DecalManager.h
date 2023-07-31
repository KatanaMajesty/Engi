#pragma once

#include <vector>
#include "Utility/SolidVector.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/InstanceTable.h"
#include "Renderer/Texture2D.h"
#include "Decal.h"

namespace engi
{

	class Renderer;
	class Material;
	class Model;
	class DynamicBuffer;
	class ConstantBuffer;

	class DecalManager
	{
	public:
		DecalManager(Renderer* renderer);

		bool init(InstanceTable* instanceTable) noexcept;
		void update() noexcept;
		void render(Texture2D* gbufferObjectID, Texture2D* gbufferNormalResource, Texture2D* depthBufferResource) noexcept;

		uint32_t createDecal(uint32_t instanceID, Texture2D* normalMap, const math::Mat4x4& decalToWorld, const math::Vec3& emissive, const math::Vec3& albedo, float metalness, float roughness) noexcept;
		const DecalInstance& getDecal(uint32_t decalID) const noexcept;
		void removeDecal(uint32_t decalID) noexcept;

		inline float& normalThreshold() noexcept { return m_normalThreshold; }

	private:
		Renderer* m_renderer;

		InstanceTable* m_instanceTable = nullptr;

		SharedHandle<Material> m_decalMaterial;
		SharedHandle<Model> m_unitCubeModel;

		struct DecalRenderGroup
		{
			void submitDecal(uint32_t decalID) noexcept;
			bool removeDecal(uint32_t decalID) noexcept;

			std::vector<uint32_t> decalIDs;
			MaterialInstance materialInstance = MaterialInstance::empty();
		};

		DecalRenderGroup* findRenderGroup(const MaterialInstance& materialInstance) noexcept;
		DecalRenderGroup* createRenderGroup(const MaterialInstance& materialInstance) noexcept;
		void removeRenderGroup(const MaterialInstance& materialInstance) noexcept;

		SolidVector<DecalInstance> m_decals;
		std::vector<DecalRenderGroup> m_decalGroups;

		bool resizeInstanceBuffer() noexcept;

		uint32_t m_bufferCapacity = 0;
		uint32_t m_bufferInstances = 0;
		UniqueHandle<DynamicBuffer> m_instanceBuffer;

		float m_normalThreshold = 60.0f;
		UniqueHandle<ConstantBuffer> m_decalProperties;
	};

}; // engi namespace