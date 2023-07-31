#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/GPUPipelineState.h"

namespace engi::gfx
{

	class D3D11Device;

	class D3D11PipelineState : public IGpuPipelineState
	{
	public:
		D3D11PipelineState(const std::string& name, D3D11Device* device, const GpuPipelineStateDesc& desc);
		virtual ~D3D11PipelineState() = default;

		virtual void* getHandle() override; // TODO: Currently this shouldn't be used for D3D11PipelineState

		bool initialize();
		inline D3D11_PRIMITIVE_TOPOLOGY getPrimitiveTopology() { return m_primitiveTopology; }
		inline ID3D11DepthStencilState* getDepthStencilState() { return m_depthStencilState.Get(); }
		inline ID3D11RasterizerState* getRasterizerState() { return m_rasterizerState.Get(); }
		inline ID3D11BlendState* getBlendState() { return m_blendState.Get(); }
		ID3D11VertexShader* getVertexShader();
		ID3D11HullShader* getHullShader();
		ID3D11DomainShader* getDomainShader();
		ID3D11GeometryShader* getGeometryShader();
		ID3D11PixelShader* getPixelShader();
		ID3D11ComputeShader* getComputeShader();

	private:
		D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
		ComPtr<ID3D11DepthStencilState> m_depthStencilState = nullptr;
		ComPtr<ID3D11RasterizerState> m_rasterizerState = nullptr;
		ComPtr<ID3D11BlendState> m_blendState = nullptr;
	}; // D3D11PipelineState class
	
}; // engi::gfx namespace