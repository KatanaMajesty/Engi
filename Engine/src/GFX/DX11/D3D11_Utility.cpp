#include "GFX/DX11/D3D11_Utility.h"

#include <array>
#include <unordered_map>
#include "Core/CommonDefinitions.h"

namespace engi::gfx::detail
{

    D3D11_USAGE d3d11Usage(GpuUsage usage)
    {
        switch (usage)
        {
        case DEFAULT: return D3D11_USAGE_DEFAULT; break;
        case IMMUTABLE: return D3D11_USAGE_IMMUTABLE; break;
        case DYNAMIC: return D3D11_USAGE_DYNAMIC; break;
        case STAGING: return D3D11_USAGE_STAGING; break;
        default: ENGI_ASSERT(false);
        }
    }

    UINT d3d11BindFlags(uint32_t pipelineFlags)
    {
        UINT outFlags = USAGE_UNKNOWN;
        if ((pipelineFlags & VERTEX_BUFFER) != 0)
            outFlags |= D3D11_BIND_VERTEX_BUFFER;
        if ((pipelineFlags & INDEX_BUFFER) != 0)
            outFlags |= D3D11_BIND_INDEX_BUFFER;
        if ((pipelineFlags & CONSTANT_BUFFER) != 0)
            outFlags |= D3D11_BIND_CONSTANT_BUFFER;
        if ((pipelineFlags & SHADER_RESOURCE) != 0)
            outFlags |= D3D11_BIND_SHADER_RESOURCE;
        if ((pipelineFlags & RENDER_TARGET) != 0)
            outFlags |= D3D11_BIND_RENDER_TARGET;
        if ((pipelineFlags & DEPTH_STENCIL) != 0)
            outFlags |= D3D11_BIND_DEPTH_STENCIL;
        if ((pipelineFlags & UNORDERED_ACCESS) != 0)
            outFlags |= D3D11_BIND_UNORDERED_ACCESS;
        return outFlags;
    }

    UINT d3d11CpuAccessFlags(uint32_t cpuFlags)
    {
        UINT outFlags = ACCESS_UNUSED;
        if ((cpuFlags & WRITE) != 0)
            outFlags |= D3D11_CPU_ACCESS_WRITE;
        if ((cpuFlags & READ) != 0)
            outFlags |= D3D11_CPU_ACCESS_READ;
        return outFlags;
    }

    UINT d3d11MiscFlags(uint32_t otherFlags)
    {
        UINT outFlags = 0;
        if ((otherFlags & MISC_GENERATE_MIPS))
            outFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        if ((otherFlags & MISC_TEXTURECUBE))
            outFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
        if ((otherFlags & MISC_STRUCTURED_BUFFER))
            outFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        if ((otherFlags & MISC_INDIRECT_ARGS))
            outFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
        return outFlags;
    }

    uint32_t d3dToGpuMiscFlags(UINT miscFlags)
    {
        uint32_t outFlags = 0;
        if ((miscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS))
            outFlags |= MISC_GENERATE_MIPS;
        if ((miscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE))
            outFlags |= MISC_TEXTURECUBE;
        if ((miscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED))
            outFlags |= MISC_STRUCTURED_BUFFER;
        if ((miscFlags & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS))
            outFlags |= MISC_INDIRECT_ARGS;
        return outFlags;
    }

    static std::array g_formatRemap =
    {
        DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R32_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS,
        DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SNORM,
        DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT,
        DXGI_FORMAT::DXGI_FORMAT_R16_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS,
        DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_R8G8_UINT,
        DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS,
        DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT::DXGI_FORMAT_BC3_TYPELESS,
        DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB,
        DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16,
        DXGI_FORMAT::DXGI_FORMAT_BC7_TYPELESS,
        DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM,
        DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB,
    };

    DXGI_FORMAT d3d11Format(GpuFormat format)
    {
        if ((size_t)format > g_formatRemap.size())
            return DXGI_FORMAT_UNKNOWN;

        return g_formatRemap[format];
    }

    static std::unordered_map<DXGI_FORMAT, GpuFormat> g_dxgiFormatRemap =
    {
        { DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,             GpuFormat::FORMAT_UNKNOWN },
        { DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  GpuFormat::RGBA32F },
        { DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT,   GpuFormat::RGBA32U },
        { DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     GpuFormat::RGB32F },
        { DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT,      GpuFormat::RGB32U },
        { DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,        GpuFormat::RG32F },
        { DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT,         GpuFormat::RG32U },
        { DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,           GpuFormat::R32F },
        { DXGI_FORMAT::DXGI_FORMAT_R32_UINT,            GpuFormat::R32U },
        { DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS,        GpuFormat::R32T },
        { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,  GpuFormat::RGBA16F },
        { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UINT,   GpuFormat::RGBA16U },
        { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SNORM,  GpuFormat::RGBA16SN },
        { DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT,        GpuFormat::RG16F },
        { DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT,         GpuFormat::RG16U },
        { DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT,           GpuFormat::R16F },
        { DXGI_FORMAT::DXGI_FORMAT_R16_UINT,            GpuFormat::R16U },
        { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS,   GpuFormat::RGBA8TYPELESS },
        { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,      GpuFormat::RGBA8UN },
        { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, GpuFormat::RGBA8UNSRGB },
        { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT,       GpuFormat::RGBA8U },
        { DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT,   GpuFormat::DEPTH_STENCIL_FORMAT },
        { DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS,      GpuFormat::R24G8T },
        { DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS,   GpuFormat::R24UNX8T },
        { DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM,          GpuFormat::RG8UN },
        { DXGI_FORMAT::DXGI_FORMAT_R8G8_UINT,           GpuFormat::RG8U },
        { DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM,           GpuFormat::BC1_UNORM },
        { DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB,      GpuFormat::BC1_UNORM_SRGB },
        { DXGI_FORMAT::DXGI_FORMAT_BC3_TYPELESS,        GpuFormat::BC3_TYPELESS },
        { DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM,           GpuFormat::BC3_UNORM },
        { DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB,      GpuFormat::BC3_UNORM_SRGB },
        { DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM,           GpuFormat::BC4_UNORM },
        { DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM,           GpuFormat::BC5_UNORM },
        { DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16,           GpuFormat::BC6_UF16 },
        { DXGI_FORMAT::DXGI_FORMAT_BC7_TYPELESS,        GpuFormat::BC7_TYPELESS },
        { DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM,           GpuFormat::BC7_UNORM },
        { DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB,      GpuFormat::BC7_UNORM_SRGB },
    };

    GpuFormat d3dToGpuFormat(DXGI_FORMAT format)
    {
        auto it = g_dxgiFormatRemap.find(format);
        return it == g_dxgiFormatRemap.end() ? GpuFormat::FORMAT_UNKNOWN : it->second;
    }

    D3D11_COMPARISON_FUNC d3d11ComparisonFunc(GpuComparisonFunc func)
    {
        switch (func)
        {
        case COMP_NEVER: return D3D11_COMPARISON_NEVER; break;
        case COMP_LESS: return D3D11_COMPARISON_LESS; break;
        case COMP_EQUAL: return D3D11_COMPARISON_EQUAL; break;
        case COMP_LESS_EQUAL: return D3D11_COMPARISON_LESS_EQUAL; break;
        case COMP_GREATER: return D3D11_COMPARISON_GREATER; break;
        case COMP_NOT_EQUAL: return D3D11_COMPARISON_NOT_EQUAL; break;
        case COMP_GREATER_EQUAL: return D3D11_COMPARISON_GREATER_EQUAL; break;
        case COMP_ALWAYS: return D3D11_COMPARISON_ALWAYS; break;
        default: ENGI_ASSERT(false);
        }
    }

    D3D11_STENCIL_OP toD3DStencilOperation(GpuStencilOperation op)
    {
        switch (op)
        {
        case STENCIL_ZERO: return D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
        case STENCIL_KEEP: return D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
        case STENCIL_REPLACE: return D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
        default: ENGI_ASSERT(false);
        }
    }

    D3D11_DEPTH_STENCILOP_DESC d3d11StencilDesc(const GpuStencilDesc& desc)
    {
        D3D11_DEPTH_STENCILOP_DESC d3dDesc;
        d3dDesc.StencilFailOp = toD3DStencilOperation(desc.failOperation);
        d3dDesc.StencilDepthFailOp = toD3DStencilOperation(desc.depthFailOperation);
        d3dDesc.StencilPassOp = toD3DStencilOperation(desc.passOperation);
        d3dDesc.StencilFunc = d3d11ComparisonFunc(desc.comparator);
        return d3dDesc;
    }

    D3D11_PRIMITIVE_TOPOLOGY d3d11PrimitiveTopology(GpuPrimitive primitive)
    {
        switch (primitive)
        {
        case PRIMITIVE_UNDEFINED: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED; break;
        case POINTLIST: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break;
        case LINELIST: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break;
        case LINESTRIP: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
        case TRIANGLELIST: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
        case TRIANGLESTRIP: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
        case LINELIST_ADJ: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ; break;
        case LINESTRIP_ADJ: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ; break;
        case TRIANGLELIST_ADJ: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ; break;
        case TRIANGLESTRIP_ADJ: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ; break;
        case CONTROLPOINT_PATCHLIST2: return D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST; break;
        case CONTROLPOINT_PATCHLIST3: return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST; break;
        case CONTROLPOINT_PATCHLIST4: return D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST; break;
        default: ENGI_ASSERT(false);
        }
    }

    D3D11_FILTER d3d11Filter(GpuSamplerFiltering filtering)
    {
        switch (filtering)
        {
        case GpuSamplerFiltering::FILTER_MIN_MAG_MIP_POINT: return D3D11_FILTER_MIN_MAG_MIP_POINT; break;
        case GpuSamplerFiltering::FILTER_MIN_MAG_POINT_MIP_LINEAR: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; break;
        case GpuSamplerFiltering::FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
        case GpuSamplerFiltering::FILTER_MIN_POINT_MAG_MIP_LINEAR: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR; break;
        case GpuSamplerFiltering::FILTER_MIN_LINEAR_MAG_MIP_POINT: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; break;
        case GpuSamplerFiltering::FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; break;
        case GpuSamplerFiltering::FILTER_MIN_MAG_LINEAR_MIP_POINT: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
        case GpuSamplerFiltering::FILTER_MIN_MAG_MIP_LINEAR: return D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
        case GpuSamplerFiltering::FILTER_ANISOTROPIC: return D3D11_FILTER_ANISOTROPIC; break;
        case GpuSamplerFiltering::FILTER_SHADOWS: return D3D11_FILTER_COMPARISON_ANISOTROPIC;
        case GpuSamplerFiltering::FILTER_UNKNOWN:
        default: ENGI_ASSERT(false);
        }
    }

    D3D11_BLEND d3d11Blend(GpuBlendFactor factor)
    {
        switch (factor)
        {
        case GpuBlendFactor::BLENDFACTOR_ZERO: return D3D11_BLEND_ZERO;
        case GpuBlendFactor::BLENDFACTOR_ONE: return D3D11_BLEND_ONE;
        case GpuBlendFactor::BLENDFACTOR_SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
        case GpuBlendFactor::BLENDFACTOR_ONE_SUB_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
        case GpuBlendFactor::BLENDFACTOR_SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
        case GpuBlendFactor::BLENDFACTOR_ONE_SUB_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
        case GpuBlendFactor::BLENDFACTOR_DEST_ALPHA: return D3D11_BLEND_DEST_ALPHA;
        case GpuBlendFactor::BLENDFACTOR_ONE_SUB_DEST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
        case GpuBlendFactor::BLENDFACTOR_DEST_COLOR: return D3D11_BLEND_DEST_COLOR;
        case GpuBlendFactor::BLENDFACTOR_ONE_SUB_DEST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
        default: ENGI_ASSERT(false);
        }
    }

    D3D11_BLEND_OP d3d11BlendOp(GpuBlendOp op)
    {
        switch (op)
        {
        case GpuBlendOp::BLENDOP_ADD: return D3D11_BLEND_OP_ADD;
        case GpuBlendOp::BLENDOP_SUB: return D3D11_BLEND_OP_SUBTRACT;
        case GpuBlendOp::BLENDOP_REV_SUB: return D3D11_BLEND_OP_REV_SUBTRACT;
        case GpuBlendOp::BLENDOP_MIN: return D3D11_BLEND_OP_MIN;
        case GpuBlendOp::BLENDOP_MAX: return D3D11_BLEND_OP_MAX;
        default: ENGI_ASSERT(false);
        }
    }

    D3D11_CULL_MODE d3d11Culling(GpuCullingMode mode)
    {
        switch (mode)
        {
        case CULLING_NONE: return D3D11_CULL_NONE;
        case CULLING_FRONT: return D3D11_CULL_FRONT;
        case CULLING_BACK: return D3D11_CULL_BACK;
        default: ENGI_ASSERT(false);
        }
    }

}; // engi::gfx::detail namespace
