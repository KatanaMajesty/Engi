#include "ParticleSystem.h"

#include <algorithm>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Utility/Random.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/Texture2D.h"
#include "Renderer/Buffer.h"

namespace engi
{

	struct GPUBillboardParticle
	{
		GPUBillboardParticle(const math::Mat4x4& worldToEntity, const SmokeParticle& particle)
			: position(particle.position * worldToEntity)
			, color(particle.color)
			, initialSize(particle.initialSize)
			, deathSize(particle.deathSize)
			, time(particle.time)
			, lifetime(particle.lifetime)
			, initialRotation(particle.initialRotation)
			, rotation(particle.rotation)
		{
		}

		math::Vec3 position;
		math::Vec3 color;
		float initialSize;
		float deathSize;
		float time;
		float lifetime;
		float initialRotation;
		float rotation;
	};

	SmokeEmitter::SmokeEmitter(Renderer* renderer, InstanceTable* instanceTable, uint32_t entityID)
		: m_renderer(renderer)
		, m_instanceTable(instanceTable)
		, m_entityID(entityID)
	{
	}

	SmokeEmitter::SmokeEmitter(const SmokeEmitter& other)
		: m_renderer(other.m_renderer)
		, m_instanceTable(other.m_instanceTable)
		, m_entityID(other.m_entityID)
		, m_position(other.m_position)
		, m_timeSinceSpawn(other.m_timeSinceSpawn)
	{
		this->init(other.m_emitterSettings, other.m_textureArrayMVEA, other.m_textureArrayDBF, other.m_textureArrayRLU);
		
		// We do this after the initialization, because it would clear the std::vector
		m_particles = other.m_particles;
	}

	SmokeEmitter& SmokeEmitter::operator=(const SmokeEmitter& other)
	{
		if (this == &other)
			return *this;

		this->m_renderer = other.m_renderer;
		this->m_instanceTable = other.m_instanceTable;
		this->m_entityID = other.m_entityID;
		this->m_position = other.m_position;
		this->m_timeSinceSpawn = other.m_timeSinceSpawn;
		
		this->init(other.m_emitterSettings, other.m_textureArrayMVEA, other.m_textureArrayDBF, other.m_textureArrayRLU);

		// We do this after the initialization, because it would clear the std::vector
		m_particles = other.m_particles;
		return *this;
	}

	SmokeEmitter::~SmokeEmitter()
	{
		// For correct UniqueHandle destruction
	}

	bool SmokeEmitter::init(const EmitterSettings& settings, Texture2D* mvea, Texture2D* dbf, Texture2D* rlu) noexcept
	{
		ENGI_ASSERT(m_renderer && settings.numParticles > 0);
		m_particles.clear();

#ifndef NDEBUG
		// TODO: Revisit this issue
		// In release mode m_particles.reserve throws std::bad_alloc somehow
		m_particles.reserve(settings.numParticles);
#endif

		// If true, then we do not need to initialize it, as it is a hollow emitter (a dead one or not yet alive)
		if (!mvea || !dbf || !rlu)
		{
			ENGI_LOG_TRACE("Omitting initialization of hollow emitter");
			return false;
		}

		m_textureArrayMVEA = mvea;
		m_textureArrayDBF = dbf;
		m_textureArrayRLU = rlu;

		m_emitterSettings = settings;
		
		m_instanceBuffer.reset(m_renderer->createDynamicBuffer("SmokeEmitter_InstanceBuffer", nullptr, m_emitterSettings.numParticles, sizeof(GPUBillboardParticle)));
		if (!m_instanceBuffer)
		{
			ENGI_LOG_WARN("Failed to initialize dynamic buffer for smoke emitter");
			return false;
		}

		static constexpr uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
		m_indexBuffer.reset(m_renderer->createIndexBuffer("SmokeEmitter_IndexBuffer", indices, 6));
		if (!m_indexBuffer)
		{
			ENGI_LOG_WARN("Failed to initialize index buffer for smoke emitter");
			return false;
		}

		ShaderProgram* shader = m_renderer->getShaderLibrary()->createProgram("SmokeEmitter.hlsl", false, false);
		shader->setAttributeLayout({ {
			gfx::GpuInputAttributeDesc("BILLBOARD_POSITION", 0, gfx::GpuFormat::RGB32F, 0, false, offsetof(GPUBillboardParticle, position)),
			gfx::GpuInputAttributeDesc("BILLBOARD_COLOR", 0, gfx::GpuFormat::RGB32F, 0, false, offsetof(GPUBillboardParticle, color)),
			gfx::GpuInputAttributeDesc("BILLBOARD_INITIAL_SIZE", 0, gfx::GpuFormat::R32F, 0, false, offsetof(GPUBillboardParticle, initialSize)),
			gfx::GpuInputAttributeDesc("BILLBOARD_DEATH_SIZE", 0, gfx::GpuFormat::R32F, 0, false, offsetof(GPUBillboardParticle, deathSize)),
			gfx::GpuInputAttributeDesc("BILLBOARD_TIME", 0, gfx::GpuFormat::R32F, 0, false, offsetof(GPUBillboardParticle, time)),
			gfx::GpuInputAttributeDesc("BILLBOARD_LIFETIME", 0, gfx::GpuFormat::R32F, 0, false, offsetof(GPUBillboardParticle, lifetime)),
			gfx::GpuInputAttributeDesc("BILLBOARD_INITIAL_ROTATION", 0, gfx::GpuFormat::R32F, 0, false, offsetof(GPUBillboardParticle, initialRotation)),
			gfx::GpuInputAttributeDesc("BILLBOARD_ROTATION", 0, gfx::GpuFormat::R32F, 0, false, offsetof(GPUBillboardParticle, rotation)),
			} });

		m_emitterMaterial = m_renderer->getMaterialRegistry()->registerMaterial("SmokeEmitter_Mat");
		m_emitterMaterial->setPrimitiveType(gfx::GpuPrimitive::TRIANGLELIST);
		m_emitterMaterial->getBlendState(0).blendEnabled = true;
		m_emitterMaterial->getDepthStencilState().depthWritable = false;
		m_emitterMaterial->setShader(shader);
		if (!m_emitterMaterial->init())
		{
			ENGI_LOG_WARN("Failed to init emitter material");
			return false;
		}

		return true;
	}

	void SmokeEmitter::update(float timestep, const math::Vec3& cameraPos) noexcept
	{
		this->removeDeadParticles();

		for (SmokeParticle& particle : m_particles)
		{
			particle.position = particle.position + particle.speed * timestep;
			particle.time += timestep;
		}

		// Add new particles, if we are able to
		m_timeSinceSpawn += timestep;
		EmitterSettings& settings = this->getSettings();
		if (m_timeSinceSpawn > settings.spawnRate && m_particles.size() < settings.numParticles)
		{
			m_timeSinceSpawn = 0.0f;

			SmokeParticle particle;
			particle.position = m_position;
			particle.time = 0.0f;
			particle.lifetime = Random::GenerateFloat(settings.minimumLifetime, settings.maximumLifetime);
			particle.speed = Random::GenerateFloat3(settings.minimumSpeed, settings.maximumSpeed);
			particle.initialSize = Random::GenerateFloat(settings.minimumParticleSize, settings.maximumParticleSize);
			particle.deathSize = particle.initialSize + Random::GenerateFloat(settings.minimumParticleGrowth, settings.maximumParticleGrowth);
			particle.initialRotation = math::toRadians(Random::GenerateFloat(0.0f, 90.0f));
			particle.rotation = math::toRadians(Random::GenerateFloat(0.0f, 60.0f));
			particle.color = settings.color;
			addParticle(particle);
		}

		math::Mat4x4 worldToEntity = this->getWorldToEntity();
		this->sortAliveParticles(worldToEntity, cameraPos);

		GPUBillboardParticle* particles = reinterpret_cast<GPUBillboardParticle*>(m_instanceBuffer->map());
		uint32_t numParticles = (uint32_t)m_particles.size();
		for (uint32_t i = 0; i < numParticles; ++i)
		{
			const SmokeParticle& particle = m_particles[i];
			particles[i] = GPUBillboardParticle(worldToEntity, particle);
		}
		m_instanceBuffer->unmap();
	}

	void SmokeEmitter::render() noexcept
	{
		uint32_t numParticles = (uint32_t)m_particles.size();

		// ENGI_ASSERT(m_textureArrayMVEA && m_textureArrayDBF && m_textureArrayRLU);
		m_textureArrayMVEA->bindShaderView(21, gfx::GpuShaderType::PIXEL_SHADER);
		m_textureArrayDBF->bindShaderView(22, gfx::GpuShaderType::PIXEL_SHADER);
		m_textureArrayRLU->bindShaderView(23, gfx::GpuShaderType::PIXEL_SHADER);

		m_instanceBuffer->bind(0, 0);
		m_indexBuffer->bind(0);
		m_emitterMaterial->bind();
		m_renderer->drawInstancedIndexed(6, numParticles, 0, 0, 0);
	}

	const math::Mat4x4& SmokeEmitter::getWorldToEntity() const noexcept
	{
		ENGI_ASSERT(m_instanceTable);
		return m_instanceTable->getInstanceData(m_entityID).modelToWorld;
	}

	void SmokeEmitter::addParticle(const SmokeParticle& particle) noexcept
	{
		m_particles.push_back(particle);
	}

	void SmokeEmitter::removeDeadParticles() noexcept
	{
		std::erase_if(m_particles, [](const SmokeParticle& particle) { return !particle.isAlive(); });
	}

	void SmokeEmitter::sortAliveParticles(const math::Mat4x4& worldToEntity, const math::Vec3& cameraPos) noexcept
	{
		using namespace math;
		Vec3 cameraLocalPos = cameraPos * worldToEntity;
#if 1 // TOGGLE BLEND SORTING
		std::ranges::sort(m_particles, [&](const SmokeParticle& lhs, const SmokeParticle& rhs) -> bool
			{
				float d1 = cameraLocalPos.dot(lhs.position);
				float d2 = cameraLocalPos.dot(rhs.position);
				return d1 < d2;
			});
#endif
	}

	ParticleSystem::ParticleSystem(Renderer* renderer)
		: m_renderer(renderer)
	{
	}

	struct GpuIncinerationParticle
	{
		math::Vec3 worldPos;
		math::Vec3 velocity;
		math::Vec3 emissive;
		float lifetime;
		uint32_t parentInstanceID;
	};

	bool ParticleSystem::init() noexcept
	{
		// The layout of Incineration Range buffer is:
		// [0] - numbfer of active particles in the buffer
		// [1] - offset of the first active particle from the beginning of the buffer
		// [2] - the number of expired particles since previous frame
		// [3], [4], [5], [6], [7] - DrawInstancedIndexedIndirect params
		// [8], [9], [10] - DispatchIndirect params
		uint32_t initialRangeData[11]{
			0, 0, 0,
			0, 0, 0, 0, 0,
			0, 1, 1
		};
		m_incinerationRangeBuffer = makeUnique<Buffer>(m_renderer->createResourceBuffer("ParticleSystem_IncinerationRange_Buffer"));
		if (!m_incinerationRangeBuffer->init(initialRangeData, 11, sizeof(uint32_t), gfx::GpuFormat::R32U))
		{
			ENGI_LOG_WARN("Failed to create incineration range buffer");
			return false;
		}

		
		// m_incinerationComputeDataBuffer = makeUnique<Buffer>(m_renderer->createResourceBuffer("ParticleSystem_IncinerationComputeData_Buffer"));
		// uint32_t initialComputeData[3]{};
		// if (!m_incinerationComputeDataBuffer->init(initialComputeData, 3, sizeof(uint32_t), gfx::GpuFormat::R32U))
		// {
		// 	ENGI_LOG_WARN("Failed to create incineration compute data buffer");
		// 	return false;
		// }

		m_numMaxIncinerationParticles = 2048;
		m_incinerationParticleStructuredBuffer = makeUnique<Buffer>(m_renderer->createResourceBuffer("ParticleSystem_IncinerationParticles_StructuredBuffer"));
		if (!m_incinerationParticleStructuredBuffer->initAsStructured(nullptr, m_numMaxIncinerationParticles, sizeof(GpuIncinerationParticle)))
		{
			ENGI_LOG_WARN("Failed to create incineration structured buffer");
			return false;
		}

		return true;
	}

	void ParticleSystem::update(float timestep, const math::Vec3& cameraPos) noexcept
	{
		for (SmokeEmitter& emitter : m_smokeEmitters)
		{
			emitter.update(timestep, cameraPos);
		}
	}

	void ParticleSystem::render(Texture2D* depthResource) noexcept
	{
		ENGI_ASSERT(depthResource);

		depthResource->bindShaderView(24, gfx::PIXEL_SHADER);
		for (SmokeEmitter& emitter : m_smokeEmitters)
		{
			emitter.render();
		}
	}

	uint32_t ParticleSystem::addEmitter(InstanceTable* instanceTable, uint32_t entityID, Texture2D* mvea, Texture2D* dbf, Texture2D* rlu, const EmitterSettings& settings) noexcept
	{
		ENGI_ASSERT(m_renderer);
		uint32_t emitterID = m_smokeEmitters.insert(SmokeEmitter(m_renderer, instanceTable, entityID));

		SmokeEmitter& emitter = m_smokeEmitters.at(emitterID);
		if (!emitter.init(settings, mvea, dbf, rlu))
		{
			ENGI_LOG_ERROR("Failed to create smoke emitter");
			ENGI_ASSERT(false);
		}

		return emitterID;
	}

	SmokeEmitter& ParticleSystem::getEmitter(uint32_t emitterID) noexcept
	{
		ENGI_ASSERT(m_smokeEmitters.isOccupied(emitterID));
		return m_smokeEmitters.at(emitterID);
	}

	const SmokeEmitter& ParticleSystem::getEmitter(uint32_t emitterID) const noexcept
	{
		ENGI_ASSERT(m_smokeEmitters.isOccupied(emitterID));
		return m_smokeEmitters.at(emitterID);
	}

	void ParticleSystem::removeEmitter(uint32_t emitterID) noexcept
	{
		ENGI_ASSERT(m_smokeEmitters.isOccupied(emitterID));
		m_smokeEmitters.erase(emitterID);
	}

	EmitterSettings ParticleSystem::s_globalEmitterSettings = EmitterSettings();

}; // engi namespace

