#include "GFX/DX11/D3D11_Swapchain.h"

#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"
#include "GFX/DX11/D3D11_Texture.h"

namespace engi::gfx
{

	D3D11Swapchain::D3D11Swapchain(const std::string& name, D3D11Device* device, const GpuSwapchainDesc& desc)
	{
		m_name = name;
		m_device = device;
		m_desc = desc;
	}

	D3D11Swapchain::~D3D11Swapchain()
	{
		m_device->getResourceAllocator()->destroyResource((IGpuResource*&)m_backbuffer);
	}

	void* D3D11Swapchain::getHandle()
	{
		return m_handle.Get();
	}

	void D3D11Swapchain::present()
	{
		m_handle->Present(m_hasVsync, 0);
	}

	bool D3D11Swapchain::resize(uint32_t width, uint32_t height)
	{
		if (m_desc.width == width && m_desc.height == height)
		{
			return false;
		}

		m_desc.width = width;
		m_desc.height = height;

		m_device->destroy((IGpuResource*&)m_backbuffer);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
		m_handle->GetDesc1(&swapchainDesc);
		HRESULT hr = m_handle->ResizeBuffers(swapchainDesc.BufferCount, width, height, swapchainDesc.Format, swapchainDesc.Flags);
		
		if (FAILED(hr))
		{
			return false;
		}

		return createBackbuffer();
	}

	IGpuTexture* D3D11Swapchain::getBackbuffer()
	{
		return m_backbuffer;
	}

	bool D3D11Swapchain::initialize()
	{
		ENGI_ASSERT(m_desc.backbuffers == 2 && "We currently only support double buffering");
		UINT swapchainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
		swapchainDesc.Width = m_desc.width;
		swapchainDesc.Height = m_desc.height;
		swapchainDesc.Format = detail::d3d11Format(m_desc.format);
		swapchainDesc.Stereo = FALSE;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = m_desc.backbuffers;
		swapchainDesc.Scaling = DXGI_SCALING_NONE;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = swapchainFlags;

		D3D11Device* device = (D3D11Device*)m_device;
		HRESULT hr = device->getDXGIFactory()->CreateSwapChainForHwnd(
			device->getHandle(), 
			(HWND)m_desc.windowHandle, 
			&swapchainDesc, 
			nullptr, 
			nullptr, 
			m_handle.ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// TODO: Maybe somehow incapsulate this code in D3D11_Resource?
#if !defined(_NDEBUG)
		const std::string& name = this->getName();
		if (m_handle)
			m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif

		return createBackbuffer();
	}

	bool D3D11Swapchain::createBackbuffer()
	{
		ENGI_ASSERT(m_handle && "Handle cannot be nullptr");

		if (m_desc.width == 0 || m_desc.height == 0)
		{
			return false;
		}

		ID3D11Resource* d3dBackbuffer;
		HRESULT hr = m_handle->GetBuffer(0, IID_ID3D11Texture2D, (void**)&d3dBackbuffer);
		if (FAILED(hr))
		{
			return false;
		}

		GpuTextureDesc backbufferDesc;
		backbufferDesc.type = TEXTURE2D;
		backbufferDesc.width = m_desc.width;
		backbufferDesc.height = m_desc.height;
		backbufferDesc.miplevels = 1;
		backbufferDesc.arraySize = 1;
		backbufferDesc.format = m_desc.format;
		backbufferDesc.usage = GpuUsage::DEFAULT;
		backbufferDesc.pipelineFlags = GpuBinding::RENDER_TARGET;
		backbufferDesc.cpuFlags = CpuAccess::ACCESS_UNUSED;
		backbufferDesc.otherFlags = 0;

		std::string name = m_name + "::backbuffer";

		D3D11Device* device = (D3D11Device*)m_device;
		m_backbuffer = m_device->createTexture(name, backbufferDesc, nullptr);
		ENGI_ASSERT(m_backbuffer && "Failed to create backbuffer");
		((D3D11Texture*)m_backbuffer)->m_handle.Attach(d3dBackbuffer);
		return true;
	}

}; // engi::gfx namespace