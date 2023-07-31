#pragma once

#include <string>
#include <span>
#include "Utility/Memory.h"
#include "GFX/Definitions.h"
#include "GFX/GPUResourceAllocator.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuShader;
		class IGpuInputLayout;
	}
	class ShaderCompiler;
	class ShaderCache;

	class ShaderProgram
	{
	public:
		ShaderProgram(const std::string& name, gfx::IGpuDevice* device, ShaderCompiler* compiler, ShaderCache* cache);
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;
		~ShaderProgram();

		bool init(const std::string& filepath, bool geometryStage, bool tesselationStage, bool pixelStage) noexcept;
		bool initCompute(const std::string& filepath) noexcept;

		void setAttributeLayout(std::span<const gfx::GpuInputAttributeDesc> attributes) noexcept;
		bool hasAttributeLayout() const noexcept { return m_inputLayout != nullptr; }
		
		constexpr gfx::IGpuShader* getVS() noexcept { return m_vs; }
		constexpr gfx::IGpuShader* getPS() noexcept { return m_ps; }
		constexpr gfx::IGpuShader* getGS() noexcept { return m_gs; }
		constexpr gfx::IGpuShader* getHS() noexcept { return m_hs; }
		constexpr gfx::IGpuShader* getDS() noexcept { return m_ds; }
		constexpr gfx::IGpuShader* getCS() noexcept { return m_cs; }
		bool recompileAll() noexcept;
		
		gfx::IGpuInputLayout* getAttributeLayout() noexcept { return m_inputLayout.get(); }
		const std::string& getName() const noexcept { return m_name; }
		const std::string& getPath() const noexcept { return m_filepath; }
		constexpr bool hasTesselationStage() const noexcept { return m_hs && m_ds; }
		constexpr bool hasGeometryStage() const noexcept { return m_gs; }
		constexpr bool hasPixelStage() const noexcept { return m_ps; }
		constexpr bool hasComputeStage() const noexcept { return m_cs; }

	private:
		gfx::IGpuShader* loadShader(const std::string& filepath, const std::string& entrypoint, gfx::GpuShaderType shaderType) noexcept;
		void recompile(gfx::IGpuShader* shader, void* bytecode) noexcept;
		void* getBytecode(const gfx::GpuShaderDesc& desc) noexcept;

		std::string m_name;
		gfx::IGpuDevice* m_device;
		ShaderCompiler* m_compiler;
		ShaderCache* m_cache;
		std::string m_filepath;
		gfx::IGpuShader* m_vs = nullptr;
		gfx::IGpuShader* m_ps = nullptr;
		gfx::IGpuShader* m_hs = nullptr;
		gfx::IGpuShader* m_ds = nullptr;
		gfx::IGpuShader* m_gs = nullptr;
		gfx::IGpuShader* m_cs = nullptr;
		gfx::GpuHandle<gfx::IGpuInputLayout> m_inputLayout = nullptr;
	};

}; // engi namespace