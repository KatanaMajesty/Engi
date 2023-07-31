#include "Renderer/Material.h"

#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUInputLayout.h"
#include "GFX/GPUPipelineState.h"
#include "Renderer/ShaderProgram.h"

// TODO: Remove this header
#include <iostream>

namespace engi
{

	Material::Material(gfx::IGpuDevice* device, const std::string& name)
		: m_device(device)
		, m_name(name)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	Material::~Material()
	{
	}

	void Material::setShader(ShaderProgram* shaderProgram) noexcept
	{
		m_shaderProgram = shaderProgram;
	}

	bool Material::init() noexcept
	{
		if (!m_shaderProgram)
			return false;

		m_desc.vs = m_shaderProgram->getVS();
		m_desc.ps = m_shaderProgram->getPS();
		m_desc.hs = m_shaderProgram->getHS();
		m_desc.ds = m_shaderProgram->getDS();
		m_desc.gs = m_shaderProgram->getGS();
		m_desc.cs = m_shaderProgram->getCS();
		gfx::IGpuPipelineState* pso = m_device->createPipelineState(m_name + "::PSO", m_desc);
		if (!pso)
		{
			std::cout << "Failed to create PSO for " << m_name << " material\n";
			return false;
		}

		m_pso = gfx::makeGpuHandle(pso, m_device->getResourceAllocator());
		return true;
	}

	void Material::bind() noexcept
	{
		m_device->setInputLayout(m_shaderProgram->getAttributeLayout());
		m_device->setPipelineState(m_pso.get());
	}

}; // engi namespace