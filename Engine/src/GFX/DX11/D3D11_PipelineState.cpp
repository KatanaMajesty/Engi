#include "GFX/DX11/D3D11_PipelineState.h"

#include "Core/CommonDefinitions.h"
#include "GFX/GPUShader.h"
#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"

namespace engi::gfx
{

	D3D11PipelineState::D3D11PipelineState(const std::string& name, D3D11Device* device, const GpuPipelineStateDesc& desc)
	{
		m_name = name;
		m_device = device;
		m_desc = desc;
	}

	void* D3D11PipelineState::getHandle()
	{
#if 0	// Legacy stuff
		// TODO: In fact, pipeline state has nothing to return here in D3D11
		// return m_depthStencilState.Get();
#else
		return nullptr;
#endif
	}

	bool D3D11PipelineState::initialize()
	{
		ENGI_ASSERT(m_desc.vs || m_desc.cs && "Vertex shader (and compute shader) cannot be nullptr");

		D3D11Device* device = (D3D11Device*)m_device;
		ID3D11Device5* d3dDevice = device->getHandle();

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ENGI_ZEROMEM(&depthStencilDesc);
		
		// Depth Testing
		depthStencilDesc.DepthEnable = m_desc.depthStencil.depthEnabled;
		depthStencilDesc.DepthFunc = detail::d3d11ComparisonFunc(m_desc.depthStencil.depthFunc);
		depthStencilDesc.DepthWriteMask = m_desc.depthStencil.depthWritable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		
		// Stencil Testing
		depthStencilDesc.StencilEnable = m_desc.depthStencil.stencilEnabled;
		depthStencilDesc.StencilReadMask = m_desc.depthStencil.stencilReadMask;
		depthStencilDesc.StencilWriteMask = m_desc.depthStencil.stencilWriteMask;
		depthStencilDesc.BackFace = detail::d3d11StencilDesc(m_desc.depthStencil.backFace);
		depthStencilDesc.FrontFace = detail::d3d11StencilDesc(m_desc.depthStencil.frontFace);

		HRESULT hr = d3dDevice->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		const GpuRasterizerState& r = m_desc.rasterizerState;
		D3D11_RASTERIZER_DESC rasterizerDesc;
		rasterizerDesc.FillMode = r.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = detail::d3d11Culling(r.culling);
		rasterizerDesc.FrontCounterClockwise = r.ccwFront;
		rasterizerDesc.DepthClipEnable = r.depthClip;
		rasterizerDesc.DepthBias = r.depthBias;
		rasterizerDesc.DepthBiasClamp = r.depthBiasClamp;
		rasterizerDesc.SlopeScaledDepthBias = r.depthSlopeBiasScale;
		// We dont use that now
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;
		hr = d3dDevice->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		D3D11_BLEND_DESC blendDesc;
		ENGI_ZEROMEM(&blendDesc);
		blendDesc.AlphaToCoverageEnable = m_desc.alphaToCoverage;
		blendDesc.IndependentBlendEnable = m_desc.independentBlend;
		for (uint32_t i = 0; i < 8; ++i)
		{
			const GpuBlendState& srcBlendDesc = m_desc.rtvBlendStates[i];
			D3D11_RENDER_TARGET_BLEND_DESC& d3d11BlendDesc = blendDesc.RenderTarget[i];
			d3d11BlendDesc.BlendEnable = srcBlendDesc.blendEnabled;
			d3d11BlendDesc.SrcBlend = detail::d3d11Blend(srcBlendDesc.srcColorFactor);
			d3d11BlendDesc.DestBlend = detail::d3d11Blend(srcBlendDesc.destColorFactor);
			d3d11BlendDesc.BlendOp = detail::d3d11BlendOp(srcBlendDesc.colorOperator);
			d3d11BlendDesc.SrcBlendAlpha = detail::d3d11Blend(srcBlendDesc.srcAlphaFactor);
			d3d11BlendDesc.DestBlendAlpha = detail::d3d11Blend(srcBlendDesc.destAlphaFactor);
			d3d11BlendDesc.BlendOpAlpha = detail::d3d11BlendOp(srcBlendDesc.alphaOPerator);
			d3d11BlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		hr = d3dDevice->CreateBlendState(&blendDesc, m_blendState.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		m_primitiveTopology = detail::d3d11PrimitiveTopology(m_desc.primitiveTopology);

// #if 0
#if !defined(_NDEBUG)
		const std::string& name = this->getName();
		if (m_depthStencilState)
		{
			std::string depthStencilName = name + "_DepthStencilState";
			m_depthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)depthStencilName.size(), depthStencilName.c_str());
		}

		if (m_rasterizerState)
		{
			std::string rasterizerName = name + "_RasterizerState";
			m_rasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)rasterizerName.size(), rasterizerName.c_str());
		}

		if (m_blendState)
		{
			std::string blendName = name + "_BlendState";
			m_blendState->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)blendName.size(), blendName.c_str());
		}
#endif

		return true;
	}

	ID3D11VertexShader* D3D11PipelineState::getVertexShader()
	{
		return m_desc.vs ? (ID3D11VertexShader*)m_desc.vs->getHandle() : nullptr;
	}

	ID3D11HullShader* D3D11PipelineState::getHullShader()
	{
		return m_desc.hs ? (ID3D11HullShader*)m_desc.hs->getHandle() : nullptr;
	}

	ID3D11DomainShader* D3D11PipelineState::getDomainShader()
	{
		return m_desc.ds ? (ID3D11DomainShader*)m_desc.ds->getHandle() : nullptr;
	}

	ID3D11GeometryShader* D3D11PipelineState::getGeometryShader()
	{
		return m_desc.gs ? (ID3D11GeometryShader*)m_desc.gs->getHandle() : nullptr;
	}

	ID3D11PixelShader* D3D11PipelineState::getPixelShader()
	{
		return m_desc.ps ? (ID3D11PixelShader*)m_desc.ps->getHandle() : nullptr;
	}

	ID3D11ComputeShader* D3D11PipelineState::getComputeShader()
	{
		return m_desc.cs ? (ID3D11ComputeShader*)m_desc.cs->getHandle() : nullptr;
	}

}; // engi::gfx namespace