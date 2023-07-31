#pragma once

#include "GFX/DX11/D3D11_API.h"
#include "GFX/Definitions.h"

namespace engi::gfx::detail
{

	D3D11_USAGE d3d11Usage(GpuUsage usage);

	UINT d3d11BindFlags(uint32_t pipelineFlags);

	UINT d3d11CpuAccessFlags(uint32_t cpuFlags);

	UINT d3d11MiscFlags(uint32_t otherFlags);

	uint32_t d3dToGpuMiscFlags(UINT miscFlags);

	DXGI_FORMAT d3d11Format(GpuFormat format);

	GpuFormat d3dToGpuFormat(DXGI_FORMAT format);

	D3D11_COMPARISON_FUNC d3d11ComparisonFunc(GpuComparisonFunc func);

	D3D11_DEPTH_STENCILOP_DESC d3d11StencilDesc(const GpuStencilDesc& desc);

	D3D11_PRIMITIVE_TOPOLOGY d3d11PrimitiveTopology(GpuPrimitive primitive);

	D3D11_FILTER d3d11Filter(GpuSamplerFiltering filtering);

	D3D11_BLEND d3d11Blend(GpuBlendFactor factor);

	D3D11_BLEND_OP d3d11BlendOp(GpuBlendOp op);

	D3D11_CULL_MODE d3d11Culling(GpuCullingMode mode);

}; // engi::gfx::detail namespace