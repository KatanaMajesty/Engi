#include "Renderer/Skybox.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUDescriptor.h"
#include "Renderer/Material.h"
#include "Renderer/TextureCube.h"

namespace engi
{

	Skybox::Skybox(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
		
	}

	bool Skybox::init(ShaderProgram* shaderProgram, ReflectionCapture* reflectionCapture) noexcept
	{
		m_reflectionCapture = reflectionCapture;

		ENGI_ASSERT(shaderProgram && "Shader program cannot be nullptr");
		m_skyboxMat.reset(new Material(m_device, m_name + "::Skybox_mat"));
		m_skyboxMat->setShader(shaderProgram);
		m_skyboxMat->getDepthStencilState().depthEnabled = true;
		m_skyboxMat->getDepthStencilState().depthWritable = false;
		m_skyboxMat->getRasterizerState().ccwFront = true;
		return m_skyboxMat->init();
	}

	void Skybox::bindIBL() noexcept
	{
		using namespace gfx;
		m_reflectionCapture->getDiffuseIBL()->bindCubeShaderView(5, PIXEL_SHADER);
		m_reflectionCapture->getSpecularIBL()->bindCubeShaderView(6, PIXEL_SHADER);
		m_reflectionCapture->getLUT()->bindShaderView(7, PIXEL_SHADER);
	}

	void Skybox::setTexture(TextureCube* texture) noexcept
	{
		ENGI_ASSERT(texture);
		m_texture = texture;

		if (!m_reflectionCapture)
		{
			ENGI_LOG_INFO("No reflection capture was set for the Skybox {}. No IBL will be applied", m_name);
			return;
		}

		if (!m_reflectionCapture->init(texture))
		{
			ENGI_LOG_ERROR("Failed to initialize reflection capture for {}", texture->getName());
			return;
		}
	}

	void Skybox::render() noexcept
	{
		ENGI_ASSERT(m_skyboxMat && "Material cannot be nullptr");

		using namespace gfx;

		m_skyboxMat->bind();
		m_texture->bindCubeShaderView(0, PIXEL_SHADER);
		m_device->draw(3, 0);
	}

}; // engi namespace