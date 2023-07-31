#pragma once

#include <filesystem>
#include <unordered_map>
#include "Utility/Memory.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderCompiler.h"
#include "Renderer/ShaderCache.h"

namespace engi
{

	namespace gfx { class IGpuDevice; }

	class ShaderLibrary
	{
	public:
		ShaderLibrary(gfx::IGpuDevice* device);
		ShaderLibrary(const ShaderLibrary&) = delete;
		ShaderLibrary& operator=(const ShaderLibrary&) = delete;
		~ShaderLibrary() = default;

		ShaderProgram* createProgram(const std::string& shadername, bool geometryStage, bool tesselationStage, bool pixelStage = true) noexcept;
		ShaderProgram* createComputeProgram(const std::string& shadername) noexcept;
		ShaderProgram* getProgram(const std::string& shadername) noexcept;
		const auto& getAllShaders() const noexcept { return m_shaders; }
	
	private:
		std::filesystem::path m_shaderFolder;
		gfx::IGpuDevice* m_device;
		ShaderCache m_shaderCache;
		ShaderCompiler m_compiler;
		std::unordered_map<std::string, UniqueHandle<ShaderProgram>> m_shaders;
	};

}; // engi namespace