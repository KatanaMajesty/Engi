#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUShader.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11Shader : public IGpuShader
	{
	public:
		D3D11Shader(const std::string& name, D3D11Device* device, const GpuShaderDesc& desc);
		virtual ~D3D11Shader() = default;

		virtual bool initialize(void* bytecode) override;
		virtual GpuShaderBuffer getBytecode() override;
		virtual void* getHandle() override;
		void attachBytecode(void* bytecode);

	private:
		ComPtr<ID3D10Blob> m_shaderBlob = nullptr;
		ComPtr<ID3D11DeviceChild> m_handle = nullptr;
	}; // D3D11Shader class

}; // engi::gfx namespace