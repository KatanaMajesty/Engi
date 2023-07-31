#pragma once

#include <vector>
#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUInputLayout.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11InputLayout : public IGpuInputLayout
	{
	public:
		D3D11InputLayout(const std::string& name, D3D11Device* device, const GpuInputAttributeDesc* attributes, uint32_t numAttributes);
		virtual ~D3D11InputLayout() = default;

		virtual void* getHandle() override;

		bool initialize(const GpuShaderBuffer& shaderBuffer);

	private:
		ComPtr<ID3D11InputLayout> m_handle;
	}; // D3D11InputLayout

}; // engi::gfx namespace