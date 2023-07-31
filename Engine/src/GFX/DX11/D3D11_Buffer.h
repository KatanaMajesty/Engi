#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUBuffer.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11Buffer : public IGpuBuffer
	{
	public:
		D3D11Buffer(const std::string& name, D3D11Device* device, const GpuBufferDesc& desc);
		virtual ~D3D11Buffer() = default;

		bool initialize(const void* initialData);
		virtual void* getHandle() override;

	private:
		ComPtr<ID3D11Buffer> m_handle;
	}; // D3D11Buffer class

}; // engi::gfx namespace