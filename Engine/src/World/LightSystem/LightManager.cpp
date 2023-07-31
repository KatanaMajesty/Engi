#include "LightManager.h"

#include <vector>
#include "Math/Math.h"
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/Renderer.h"
#include "Renderer/TextureLibrary.h"
#include "Renderer/Buffer.h"
#include "Shaders/LightCasters.hlsli"

namespace engi
{

	LightManager::LightManager(Renderer* renderer)
		: m_renderer(renderer)
	{
		ENGI_ASSERT(renderer && "Renderer cannot be nullptr");
	}

	LightManager::~LightManager()
	{
	}

	bool LightManager::init() noexcept
	{
		m_dirLights.clear();
		m_spotLights.clear();
		m_pointLights.clear();
		
		// TODO: If we were to have different SceneRenderers, thus different LightManagers, this would result in some naming warnings (not errors)
		m_dirLightBuffer = makeUnique<Buffer>(m_renderer->createResourceBuffer("LightManager::DirLightBuffer"));
		if (!m_dirLightBuffer->initAsStructured(nullptr, this->getMaxDirLights(), sizeof(ENGI_DirectionalLight)))
		{
			ENGI_LOG_WARN("Failed to create directional light resource buffer");
			return false;
		}

		// TODO: If we were to have different SceneRenderers, thus different LightManagers, this would result in some naming warnings (not errors)
		m_spotLightBuffer = makeUnique<Buffer>(m_renderer->createResourceBuffer("LightManager::SpotLightBuffer"));
		if (!m_spotLightBuffer->initAsStructured(nullptr, this->getMaxSpotLights(), sizeof(ENGI_SpotLight)))
		{
			ENGI_LOG_WARN("Failed to create spot light resource buffer");
			return false;
		}

		// TODO: If we were to have different SceneRenderers, thus different LightManagers, this would result in some naming warnings (not errors)
		m_pointLightBuffer = makeUnique<Buffer>(m_renderer->createResourceBuffer("LightManager::PointLightBuffer"));
		if (!m_pointLightBuffer->initAsStructured(nullptr, this->getMaxPointLights(), sizeof(ENGI_PointLight)))
		{
			ENGI_LOG_WARN("Failed to create point light resource buffer");
			return false;
		}

		if (!createDirLightArray())
		{
			ENGI_LOG_WARN("Failed to create dirlight array");
			return false;
		}

		if (!createSpotLightArray())
		{
			ENGI_LOG_WARN("Failed to create spotlight array");
			return false;
		}

		if (!createPointLightArray())
		{
			ENGI_LOG_WARN("Failed to create pointlight array");
			return false;
		}

		return true;
	}

	void LightManager::update() noexcept
	{
		this->updateBuffer(m_dirLights.data(), getNumDirLights(), 0);
		this->updateBuffer(m_spotLights.data(), getNumSpotLights(), 0);
		this->updateBuffer(m_pointLights.data(), getNumPointLights(), 0);
	}

	uint32_t LightManager::createDirLight(const math::Vec3& color, const math::Vec3& direction, float radius) noexcept
	{
		uint32_t arrayslice = this->getNumDirLights();
		if (arrayslice >= this->getMaxDirLights())
		{
			ENGI_LOG_WARN("No more directional lights can be created");
			return uint32_t(-1);
		}

		uint32_t cubeSize = this->getDirectionalDepthmapArray()->getWidth();
		uint32_t ID = m_dirLights.insert(DirectionalLight(color, direction, radius, arrayslice, cubeSize));
		this->updateBuffer(m_dirLights.data(), 1, m_dirLights.getIndex(ID));
		return ID;
	}

	uint32_t LightManager::createSpotLight(const math::Vec3& color, float intensity, float radius) noexcept
	{
		uint32_t arrayslice = this->getNumSpotLights();
		if (arrayslice >= this->getMaxDirLights())
		{
			ENGI_LOG_WARN("No more directional lights can be created");
			return uint32_t(-1);
		}

		uint32_t ID = m_spotLights.insert(SpotLight(arrayslice, color, intensity, radius));
		this->updateBuffer(m_spotLights.data(), 1, m_spotLights.getIndex(ID));
		return ID;
	}

	uint32_t LightManager::createPointLight(const math::Vec3& color, float intensity, float radius) noexcept
	{
		uint32_t arrayslice = this->getNumPointLights();
		if (arrayslice >= this->getMaxPointLights())
		{
			ENGI_LOG_WARN("No more point lights can be created");
			return uint32_t(-1);
		}

		uint32_t ID = m_pointLights.insert(PointLight(arrayslice, color, intensity, radius));
		this->updateBuffer(m_pointLights.data(), 1, m_pointLights.getIndex(ID));
		return ID;
	}

	void LightManager::removeDirLight(uint32_t ID) noexcept 
	{ 
		ENGI_ASSERT(m_dirLights.isOccupied(ID)); 
		m_dirLights.erase(ID);
	}

	void LightManager::removeSpotLight(uint32_t ID) noexcept 
	{ 
		ENGI_ASSERT(m_spotLights.isOccupied(ID));
		m_spotLights.erase(ID); 
	}

	void LightManager::removePointLight(uint32_t ID) noexcept 
	{ 
		ENGI_ASSERT(m_pointLights.isOccupied(ID)); 
		m_pointLights.erase(ID);
	} 

	Texture2D* LightManager::getDirectionalDepthmapArray() noexcept
	{
		return m_renderer->getTextureLibrary()->getTexture2D("LightManager::DirLightDepthMapArray");
	}

	Texture2D* LightManager::getSpotDepthmapArray() noexcept
	{
		return m_renderer->getTextureLibrary()->getTexture2D("LightManager::SpotLightDepthMapArray");
	}

	TextureCube* LightManager::getPointDepthmapArray() noexcept
	{
		return m_renderer->getTextureLibrary()->getTextureCube("LightManager::PointLightDepthMapArray");
	}

	template<typename TextureType>
	using ShaderViewFunction = bool(TextureType::*)(gfx::GpuFormat);

	template<typename TextureType>
	static bool createArrayOfDepthmaps(TextureType* texture, const ShaderViewFunction<TextureType>& srvCallback, uint32_t width, uint32_t height, uint32_t numTextures)
	{
		using namespace gfx;
		if (!texture->init(width, height, 1, numTextures, GpuFormat::R24G8T, GpuBinding::DEPTH_STENCIL | GpuBinding::SHADER_RESOURCE))
		{
			ENGI_LOG_ERROR("Failed to init a texture array: {}", texture->getName());
			return false;
		}

		auto r = std::invoke(srvCallback, texture, GpuFormat::R24UNX8T);
		if (!r)
		{
			ENGI_LOG_ERROR("Failed to init a texture array shader view: {}", texture->getName());
			return false;
		}

		return true;
	}

	bool LightManager::createDirLightArray() noexcept
	{
		Texture2D* texture = m_renderer->getTextureLibrary()->createTexture2D("LightManager::DirLightDepthMapArray");
		return createArrayOfDepthmaps(texture, &Texture2D::initShaderView, 4096, 4096, this->getMaxDirLights());
	}

	bool LightManager::createSpotLightArray() noexcept
	{
		Texture2D* texture = m_renderer->getTextureLibrary()->createTexture2D("LightManager::SpotLightDepthMapArray");
		return createArrayOfDepthmaps(texture, &Texture2D::initShaderView, 512, 512, this->getMaxSpotLights());
	}

	bool LightManager::createPointLightArray() noexcept
	{
		using namespace gfx;
		TextureCube* texture = m_renderer->getTextureLibrary()->createTextureCube("LightManager::PointLightDepthMapArray");
		return createArrayOfDepthmaps(texture, &TextureCube::initCubeShaderView, 512, 512, this->getMaxPointLights());
	}

	void LightManager::updateBuffer(const DirectionalLight* lights, uint32_t numLights, uint32_t offset)
	{
		if (!lights || numLights == 0)
			return;

		Buffer* buffer = m_dirLightBuffer.get();
		ENGI_ASSERT(buffer);
		ENGI_ASSERT(numLights + offset <= buffer->getNumElements());
		if (numLights + offset > buffer->getNumElements())
		{
			ENGI_LOG_WARN("Couldn't update the buffer {}: out of bounds", buffer->getName());
			return;
		}

		std::vector<ENGI_DirectionalLight> bufferLights;
		bufferLights.reserve(numLights);
		for (uint32_t i = 0; i < numLights; ++i)
		{
			uint32_t lightIndex = offset + i;
			const DirectionalLight* light = lights + lightIndex;

			ENGI_DirectionalLight& bufferLight = bufferLights.emplace_back();
			bufferLight.ambient = light->getColor();
			bufferLight.direction = light->getDirection();
			bufferLight.radius = light->getRadius();
			bufferLight.depthArraySlice = lightIndex;
			bufferLight.worldToLight = light->getView() * light->getProjection();
		}

		buffer->upload(offset, bufferLights.data(), numLights);
	}

	void LightManager::updateBuffer(const PointLight* lights, uint32_t numLights, uint32_t offset)
	{
		if (!lights || numLights == 0)
			return;

		Buffer* buffer = m_pointLightBuffer.get();
		ENGI_ASSERT(buffer);
		ENGI_ASSERT(numLights + offset <= buffer->getNumElements());
		if (numLights + offset > buffer->getNumElements())
		{
			ENGI_LOG_WARN("Couldn't update the buffer {}: out of bounds", buffer->getName());
			return;
		}

		std::vector<ENGI_PointLight> bufferLights;
		bufferLights.reserve(numLights);
		for (uint32_t i = 0; i < numLights; ++i)
		{
			uint32_t lightIndex = offset + i;
			const PointLight* light = lights + lightIndex;

			ENGI_PointLight& bufferLight = bufferLights.emplace_back();
			bufferLight.intensity = light->getIntensity();
			bufferLight.color = light->getColor();
			bufferLight.position = light->getPosition();
			bufferLight.radius = light->getRadius();
			bufferLight.depthArraySlice = light->getDepthmapArrayslice();

			math::Mat4x4 proj = light->getProjection();
			for (uint32_t i = 0; i < 6; ++i)
				bufferLight.worldToLight[i] = light->getView(i) * proj;
		}

		buffer->upload(offset, bufferLights.data(), numLights);
	}

	void LightManager::updateBuffer(const SpotLight* lights, uint32_t numLights, uint32_t offset)
	{
		if (!lights || numLights == 0)
			return;

		Buffer* buffer = m_spotLightBuffer.get();
		ENGI_ASSERT(buffer);
		ENGI_ASSERT(numLights + offset <= buffer->getNumElements());
		if (numLights + offset > buffer->getNumElements())
		{
			ENGI_LOG_WARN("Couldn't update the buffer {}: out of bounds", buffer->getName());
			return;
		}

		std::vector<ENGI_SpotLight> bufferLights;
		bufferLights.reserve(numLights);
		for (uint32_t i = 0; i < numLights; ++i)
		{
			uint32_t lightIndex = offset + i;
			const SpotLight* light = lights + lightIndex;

			ENGI_SpotLight& bufferLight = bufferLights.emplace_back();
			bufferLight.intensity = light->getIntensity();
			bufferLight.color = light->getColor();
			bufferLight.cutoff = math::cos(math::toRadians(light->getCutoff()));
			bufferLight.outerCutoff = math::cos(math::toRadians(light->getCutoff() + light->getSmoothing()));
			bufferLight.useSpotlightMask = light->getCookie() ? 1 : 0; // T : F
			bufferLight.direction = light->getDirection();
			bufferLight.position = light->getPosition();
			bufferLight.worldToLocal = light->getView();
			bufferLight.worldToLight = bufferLight.worldToLocal * light->getProjection();
			bufferLight.depthArraySlice = light->getDepthmapArrayslice();
			bufferLight.radius = light->getRadius();
		}

		buffer->upload(offset, bufferLights.data(), numLights);
	}

}; // engi namespace