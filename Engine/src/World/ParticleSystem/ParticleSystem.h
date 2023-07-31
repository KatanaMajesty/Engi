#pragma once

#include <vector>
#include "Utility/Memory.h"
#include "Utility/SolidVector.h"
#include "Renderer/Renderer.h"
#include "Renderer/DynamicBuffer.h"
#include "Renderer/Material.h"
#include "Renderer/InstanceTable.h"
#include "Math/Math.h"

namespace engi
{

	class Buffer;

	struct SmokeParticle
	{
		inline constexpr bool isAlive() const noexcept { return lifetime > 0.0f && time < lifetime; }

		math::Vec3 position;
		math::Vec3 speed;
		math::Vec3 color;
		float initialSize;
		float deathSize;
		float time;
		float lifetime;
		float initialRotation;
		float rotation;
	};

	struct EmitterSettings
	{
		uint32_t numParticles;
		math::Vec3 color;
		math::Vec3 minimumSpeed;
		math::Vec3 maximumSpeed;
		float minimumParticleSize;
		float maximumParticleSize;
		float minimumParticleGrowth;
		float maximumParticleGrowth;
		float minimumLifetime;
		float maximumLifetime;
		float spawnRate;
	};

	class SmokeEmitter
	{
	public:
		SmokeEmitter() = default;
		SmokeEmitter(Renderer* renderer, InstanceTable* instanceTable, uint32_t entityID);
		SmokeEmitter(const SmokeEmitter&);
		SmokeEmitter& operator=(const SmokeEmitter&);
		~SmokeEmitter();

		bool init(const EmitterSettings& settings, Texture2D* mvea, Texture2D* dbf, Texture2D* rlu) noexcept;
		void update(float timestep, const math::Vec3& cameraPos) noexcept;
		void render() noexcept;
		
		inline constexpr void setRelativePosition(const math::Vec3& position) noexcept { m_position = position; }
		inline constexpr const math::Vec3& getRelativePosition() const noexcept { return m_position; }

		inline constexpr void setSettings(const EmitterSettings& settings) noexcept { m_emitterSettings = settings; }
		inline constexpr EmitterSettings& getSettings() noexcept { return m_emitterSettings; }
		inline constexpr const EmitterSettings& getSettings() const noexcept { return m_emitterSettings; }

	private:
		const math::Mat4x4& getWorldToEntity() const noexcept;
		void addParticle(const SmokeParticle& particle) noexcept;
		void removeDeadParticles() noexcept;
		void sortAliveParticles(const math::Mat4x4& worldToEntity, const math::Vec3& cameraPos) noexcept;

		Renderer* m_renderer = nullptr;
		InstanceTable* m_instanceTable = nullptr;
		uint32_t m_entityID;

		math::Vec3 m_position;
		float m_timeSinceSpawn = 0.0f;
		EmitterSettings m_emitterSettings;

		UniqueHandle<DynamicBuffer> m_instanceBuffer = nullptr;
		UniqueHandle<IndexBuffer> m_indexBuffer = nullptr;

		SharedHandle<Material> m_emitterMaterial = nullptr;
		Texture2D* m_textureArrayMVEA = nullptr;
		Texture2D* m_textureArrayDBF = nullptr;
		Texture2D* m_textureArrayRLU = nullptr;
		std::vector<SmokeParticle> m_particles;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(Renderer* renderer);

		bool init() noexcept;
		void update(float timestep, const math::Vec3& cameraPos) noexcept;
		void render(Texture2D* depthResource) noexcept;
		uint32_t addEmitter(InstanceTable* instanceTable, uint32_t entityID, Texture2D* mvea, Texture2D* dbf, Texture2D* rlu, const EmitterSettings& settings = ParticleSystem::getGlobalEmitterSettings()) noexcept;

		SmokeEmitter& getEmitter(uint32_t emitterID) noexcept;
		const SmokeEmitter& getEmitter(uint32_t emitterID) const noexcept;

		void removeEmitter(uint32_t emitterID) noexcept;
		inline bool isEmitterID(uint32_t emitterID) const noexcept { return m_smokeEmitters.isOccupied(emitterID); }

		static EmitterSettings& getGlobalEmitterSettings() noexcept { return s_globalEmitterSettings; }
		inline Buffer* getIncinerationRangeBuffer() noexcept { return m_incinerationRangeBuffer.get(); }
		inline Buffer* getIncinerationStructuredBuffer() noexcept { return m_incinerationParticleStructuredBuffer.get(); }

	private:
		Renderer* m_renderer;
		SolidVector<SmokeEmitter> m_smokeEmitters;
		
		static EmitterSettings s_globalEmitterSettings;

		uint32_t m_numMaxIncinerationParticles = 0;

		// The layout of Incineration Range buffer is:
		// [0] - numbfer of active particles in the buffer
		// [1] - offset of the first active particle from the beginning of the buffer
		// [2] - the number of expired particles since previous frame
		// [3], [4], [5], [6], [7] - DrawInstancedIndexedIndirect params
		// [8], [9], [10] - DispatchIndirect params
		UniqueHandle<Buffer> m_incinerationRangeBuffer;
		UniqueHandle<Buffer> m_incinerationParticleStructuredBuffer;
	};

}; // engi namespace