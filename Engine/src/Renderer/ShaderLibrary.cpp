#include "Renderer/ShaderLibrary.h"

#include "Core/Logger.h"
#include "Core/FileSystem.h"
#include "Renderer/ShaderCache.h"

namespace engi
{

	ShaderLibrary::ShaderLibrary(gfx::IGpuDevice* device)
		: m_shaderFolder(FileSystem::getInstance().getShaderPath())
		, m_device(device)
		, m_shaderCache(device)
	{
	}

	ShaderProgram* ShaderLibrary::createProgram(const std::string& shadername, bool geometryStage, bool tesselationStage, bool pixelStage) noexcept
	{
		ShaderProgram* program = getProgram(shadername);

		// TODO: This will fail the creation of shader IF the createComputeProgram was called earlier on the same shadername
		if (program) 
			return program;

		std::string filepath = (m_shaderFolder / shadername).string();
		program = new ShaderProgram(shadername, m_device, &m_compiler, &m_shaderCache);
		if (!program->init(filepath, geometryStage, tesselationStage, pixelStage))
		{
			ENGI_LOG_WARN("Failed to init a shader program {}", shadername);
			delete program;
			return nullptr;
		}

		m_shaders[shadername] = makeUnique<ShaderProgram>(program);
		return program;
	}

	ShaderProgram* ShaderLibrary::createComputeProgram(const std::string& shadername) noexcept
	{
		ShaderProgram* program = getProgram(shadername);
		if (program && program->hasComputeStage())
			return program;

		if (program)
		{
			if (program->hasComputeStage())
				return program;

			program->initCompute(program->getPath());
			return program;
		}

		std::string filepath = (m_shaderFolder / shadername).string();
		program = new ShaderProgram(shadername, m_device, &m_compiler, &m_shaderCache);
		if (!program->initCompute(filepath))
		{
			ENGI_LOG_WARN("Failed to init a shader program {}", shadername);
			delete program;
			return nullptr;
		}

		m_shaders[shadername] = makeUnique<ShaderProgram>(program);
		return program;
	}

	ShaderProgram* ShaderLibrary::getProgram(const std::string& shadername) noexcept
	{
		auto it = m_shaders.find(shadername);
		return it == m_shaders.end() ? nullptr : it->second.get();
	}

}; // engi namespace