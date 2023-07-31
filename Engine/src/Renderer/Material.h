#pragma once

#include <array>
#include <string>
#include "Utility/Memory.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUResourceAllocator.h"
#include "GFX/Definitions.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IGpuPipelineState;
		class IGpuTexture;
	}
	class ShaderProgram;

	class Material
	{
	public:
		Material(gfx::IGpuDevice* device, const std::string& name = "Unknown material");
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;
		~Material();

		void setShader(ShaderProgram* shaderProgram) noexcept;
		bool init() noexcept;
		void bind() noexcept;
		ShaderProgram* getShader() noexcept { return m_shaderProgram; }
		const std::string& getName() const noexcept { return m_name; }
		gfx::GpuDepthStencilState& getDepthStencilState() noexcept { return m_desc.depthStencil; }
		gfx::GpuRasterizerState& getRasterizerState() noexcept { return m_desc.rasterizerState; }
		gfx::GpuBlendState& getBlendState(uint32_t rtv) noexcept { return m_desc.rtvBlendStates[rtv]; }
		gfx::GpuPipelineStateDesc& getDesc() noexcept { return m_desc; }
		void setPrimitiveType(gfx::GpuPrimitive type) noexcept { m_desc.primitiveTopology = type; }

	private:
		friend struct MaterialInstance;

		gfx::IGpuDevice* m_device;
		std::string m_name;
		ShaderProgram* m_shaderProgram = nullptr;
		gfx::GpuPipelineStateDesc m_desc{};
		gfx::GpuHandle<gfx::IGpuPipelineState> m_pso = nullptr;
	};

}; // engi namespace
