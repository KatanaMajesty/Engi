#include "Renderer/ShaderCache.h"

#include "Core/CommonDefinitions.h"
#include "Core/FileSystem.h"
#include "GFX/GPUDevice.h"

namespace engi
{

	ShaderCache::ShaderCache(gfx::IGpuDevice* device)
		: m_device(device)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	void ShaderCache::addShader(gfx::IGpuShader* shader) noexcept
	{
		m_compiledShaders[shader->getDesc()] = gfx::makeGpuHandle(shader, m_device->getResourceAllocator());
	}

	gfx::IGpuShader* ShaderCache::getShader(const gfx::GpuShaderDesc& desc) noexcept
	{
		auto it = m_compiledShaders.find(desc);
		return it == m_compiledShaders.end() ? nullptr : it->second.get();
	}

	void ShaderCache::removeShader(gfx::IGpuShader* shader) noexcept
	{
		[[maybe_unused]] size_t erased = m_compiledShaders.erase(shader->getDesc());
		ENGI_ASSERT(erased > 0);
	}

}; // engi namespace