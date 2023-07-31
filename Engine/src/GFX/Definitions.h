#pragma once

#include <cstdint>
#include <string>

namespace engi::gfx
{

	enum GpuUsage
	{
		USAGE_UNKNOWN,
		DEFAULT,
		IMMUTABLE,
		DYNAMIC,
		STAGING,
	};

	enum GpuBinding
	{
		BINDING_UNKNOWN = 0,
		VERTEX_BUFFER = 1,
		INDEX_BUFFER = 2,
		CONSTANT_BUFFER = 4,
		SHADER_RESOURCE = 8,
		RENDER_TARGET = 16,
		DEPTH_STENCIL = 32,
		UNORDERED_ACCESS = 64,
	};

	enum CpuAccess
	{
		ACCESS_UNUSED = 0,
		WRITE = 1,
		READ = 2,
	};

	enum GpuShaderType
	{
		UNKNOWN_SHADER = 0,
		VERTEX_SHADER = 1,
		PIXEL_SHADER = 2,
		GEOMETRY_SHADER = 4,
		HULL_SHADER = 8,
		DOMAIN_SHADER = 16,
		COMPUTE_SHADER = 32,
	};

	enum GpuMisc
	{
		MISC_NONE = 0,
		MISC_GENERATE_MIPS = 1,
		MISC_TEXTURECUBE = 2,
		MISC_STRUCTURED_BUFFER = 4,
		MISC_BYTEADDRESS_BUFFER = 8,
		MISC_INDIRECT_ARGS = 16,
	};

	struct GpuBufferDesc
	{
		GpuUsage usage = GpuUsage::DEFAULT;
		uint32_t bytes = 0;
		uint32_t pipelineFlags = GpuBinding::BINDING_UNKNOWN;
		uint32_t cpuFlags = CpuAccess::ACCESS_UNUSED;
		uint32_t byteStride = 0; // This field is only used if the buffer is structured. Otherwise the field is ignored
		uint32_t otherFlags = GpuMisc::MISC_NONE;
	};

	struct GpuShaderDesc
	{
		GpuShaderType type;
		std::string filepath;
		std::string entrypoint;
		uint32_t compilerFlags = 0; // currently not used
	};

	struct GpuShaderBuffer
	{
		const void* bytecode;
		size_t size;
	};

	enum GpuTextureType
	{
		TEXTURE_UNKNOWN,
		TEXTURE1D,
		TEXTURE2D,
		TEXTURE3D,
	};

	enum GpuFormat
	{
		FORMAT_UNKNOWN,
		RGBA32F,
		RGBA32U,
		RGB32F,
		RGB32U,
		RG32F,
		RG32U,
		R32F,
		R32U,
		R32T, // Typeless
		RGBA16F,
		RGBA16U,
		RGBA16SN, // signed-normalized
		RG16F,
		RG16U,
		R16F,
		R16U,
		RGBA8TYPELESS,
		RGBA8UN,
		RGBA8UNSRGB,
		RGBA8U,
		RG8UN,
		RG8U,
		DEPTH_STENCIL_FORMAT, // used for DSV only
		R24G8T, // for depthmap RTV
		R24UNX8T, // for depthmap SRV
		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC3_TYPELESS,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_UNORM, // 1-component block compression
		BC5_UNORM,
		BC6_UF16,
		BC7_TYPELESS,
		BC7_UNORM,
		BC7_UNORM_SRGB,
	};

	struct GpuTextureDesc
	{
		GpuTextureType type;
		uint32_t width;
		uint32_t height; // ignored if type == Texture1D
		uint32_t depth; // ignored if type == Texture1D or Texture2D
		uint32_t miplevels;
		uint32_t arraySize; // ignored if type == Texture3D
		GpuFormat format;
		GpuUsage usage;
		uint32_t pipelineFlags; // use GpuBinding enum
		uint32_t cpuFlags; // use CpuAccess enum
		uint32_t otherFlags; // use GpuMisc
	};

	struct GpuSwapchainDesc
	{
		void* windowHandle; // HWND for D3D11
		uint32_t width = 1; // for explicit width specification
		uint32_t height = 1; // for explicit height specification
		uint32_t backbuffers = 2;
		GpuFormat format = GpuFormat::RGBA8UN;
	};

	class IGpuTexture;
	struct GpuRenderTargetDesc
	{
		IGpuTexture* rtv = nullptr;
		uint32_t mipSlice = 0;
		uint32_t arraySlice = 0;
		bool useClearcolor = true;
		float clearcolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	class IGpuDescriptor;
	struct GpuRenderPassDesc
	{
		GpuRenderTargetDesc rtvs[8]{};
		IGpuDescriptor* uavs[8]{};
		IGpuTexture* depthStencilBuffer = nullptr;
		uint32_t depthMip = 0; // mipslice of depthbuffer
		uint32_t depthSlice = 0; // arrayslice of depthbuffer
		bool useClearDepth = true;
		bool useClearStencil = true;
		float clearDepth = 0.0f;
		uint32_t clearStencil = 0;
	};

	enum GpuComparisonFunc
	{
		COMP_NEVER,
		COMP_LESS,
		COMP_EQUAL,
		COMP_LESS_EQUAL,
		COMP_GREATER,
		COMP_NOT_EQUAL,
		COMP_GREATER_EQUAL,
		COMP_ALWAYS,
	};

	enum GpuStencilOperation
	{
		STENCIL_ZERO,
		STENCIL_KEEP,
		STENCIL_REPLACE,
	};

	struct GpuStencilDesc
	{
		GpuStencilOperation failOperation = GpuStencilOperation::STENCIL_KEEP;
		GpuStencilOperation depthFailOperation = GpuStencilOperation::STENCIL_KEEP;
		GpuStencilOperation passOperation = GpuStencilOperation::STENCIL_KEEP;
		GpuComparisonFunc comparator = GpuComparisonFunc::COMP_ALWAYS;
	};

	struct GpuDepthStencilState
	{
		bool depthEnabled = true;
		bool depthWritable = true;
		GpuComparisonFunc depthFunc = GpuComparisonFunc::COMP_GREATER_EQUAL; // for reversed-depth

		bool stencilEnabled = false;
		uint8_t stencilReadMask = 0xff;
		uint8_t stencilWriteMask = 0xff;
		GpuStencilDesc frontFace;
		GpuStencilDesc backFace;

		uint8_t stencilRef = 0;
	};

	enum GpuCullingMode
	{
		CULLING_NONE,
		CULLING_FRONT,
		CULLING_BACK,
	};

	struct GpuRasterizerState
	{
		bool wireframe = false;
		GpuCullingMode culling = CULLING_BACK;
		bool ccwFront = false;
		bool depthClip = true;
		int32_t depthBias = 0;
		float depthBiasClamp = 0.0f;
		float depthSlopeBiasScale = 0.0f;
		// Scissors are skipped
		// Multisampling and line sampling stages are skipped
	};

	enum GpuBlendFactor
	{
		BLENDFACTOR_ZERO,
		BLENDFACTOR_ONE,
		BLENDFACTOR_SRC_COLOR,
		BLENDFACTOR_ONE_SUB_SRC_COLOR,
		BLENDFACTOR_SRC_ALPHA,
		BLENDFACTOR_ONE_SUB_SRC_ALPHA,
		BLENDFACTOR_DEST_ALPHA,
		BLENDFACTOR_ONE_SUB_DEST_ALPHA,
		BLENDFACTOR_DEST_COLOR,
		BLENDFACTOR_ONE_SUB_DEST_COLOR,
	};

	enum GpuBlendOp
	{
		BLENDOP_ADD,
		BLENDOP_SUB,
		BLENDOP_REV_SUB,
		BLENDOP_MIN,
		BLENDOP_MAX,
	};

	// Normal blending by default
	struct GpuBlendState
	{
		bool blendEnabled = false;
		GpuBlendFactor srcColorFactor = GpuBlendFactor::BLENDFACTOR_SRC_ALPHA;
		GpuBlendFactor destColorFactor = GpuBlendFactor::BLENDFACTOR_ONE_SUB_SRC_ALPHA;
		GpuBlendOp colorOperator = GpuBlendOp::BLENDOP_ADD;
		GpuBlendFactor srcAlphaFactor = GpuBlendFactor::BLENDFACTOR_ONE;
		GpuBlendFactor destAlphaFactor = GpuBlendFactor::BLENDFACTOR_ONE_SUB_SRC_ALPHA;
		GpuBlendOp alphaOPerator = GpuBlendOp::BLENDOP_ADD;
	};

	enum GpuPrimitive
	{
		PRIMITIVE_UNDEFINED,
		POINTLIST,
		LINELIST,
		LINESTRIP,
		TRIANGLELIST,
		TRIANGLESTRIP,
		LINELIST_ADJ,
		LINESTRIP_ADJ,
		TRIANGLELIST_ADJ,
		TRIANGLESTRIP_ADJ,
		CONTROLPOINT_PATCHLIST2,
		CONTROLPOINT_PATCHLIST3,
		CONTROLPOINT_PATCHLIST4,
	};

	class IGpuShader;
	struct GpuPipelineStateDesc
	{
		bool alphaToCoverage = false;
		bool independentBlend = false;
		GpuBlendState rtvBlendStates[8]{};
		GpuDepthStencilState depthStencil{};
		GpuRasterizerState rasterizerState{};
		GpuPrimitive primitiveTopology = GpuPrimitive::TRIANGLELIST;
		IGpuShader* vs = nullptr; // not nullable is CS is null
		IGpuShader* hs = nullptr; // nullable
		IGpuShader* ds = nullptr; // nullable
		IGpuShader* gs = nullptr; // nullable
		IGpuShader* ps = nullptr; // nullable
		IGpuShader* cs = nullptr; // not nullable if VS is null
	};

	enum GpuSamplerFiltering
	{
		FILTER_UNKNOWN,
		FILTER_MIN_MAG_MIP_POINT,
		FILTER_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MIN_MAG_MIP_LINEAR,
		FILTER_ANISOTROPIC,
		FILTER_SHADOWS,
	};

	enum GpuSamplerAddressing
	{
		ADDRESS_WRAP,
		ADDRESS_MIRROR,
		ADDRESS_CLAMP,
		ADDRESS_BORDER,
	};

	struct GpuSamplerDesc
	{
		GpuSamplerFiltering	filtering = GpuSamplerFiltering::FILTER_UNKNOWN;
		GpuComparisonFunc comparator = GpuComparisonFunc::COMP_GREATER;
		GpuSamplerAddressing addressing = GpuSamplerAddressing::ADDRESS_WRAP;
		float border[4];
		// float mipBias = 0 // unused
		// uint32_t maxAnisotropy = 8 // obsolete
		// uint32_t minLod = 0; // unused
		// uint32_t maxLod = max; // unused
	};

	struct GpuInputAttributeDesc
	{
		GpuInputAttributeDesc() = default;
		GpuInputAttributeDesc(const std::string& semanticName, uint32_t semanticIndex, GpuFormat format, uint32_t inputSlot, bool perVertex, uint32_t byteOffset)
			: semanticName(semanticName)
			, semanticIndex(semanticIndex)
			, format(format)
			, inputSlot(inputSlot)
			, perVertex(perVertex)
			, byteOffset(byteOffset)
		{
		}

		std::string semanticName;
		uint32_t semanticIndex;
		GpuFormat format;
		uint32_t inputSlot;
		bool perVertex;
		uint32_t byteOffset; // packing is asserted
	};

	enum GpuResourceType
	{
		RESOURCE_UNKNOWN,
		RESOURCE_BUFFER,
		RESOURCE_TEXTURE2D,
		RESOURCE_TEXTURE2D_ARRAY,
		RESOURCE_TEXTURECUBE,
		RESOURCE_TEXTURECUBE_ARRAY,
	};

	// This is replaced with enum GpuResourceType
	//enum GpuSRVType
	//{
	//	SRV_UNKNOWN,
	//	SRV_STRUCTURED_BUFFER,
	//	SRV_TEXTURE2D,
	//	SRV_TEXTURE2D_ARRAY,
	//	SRV_TEXTURECUBE,
	//	SRV_TEXTURECUBE_ARRAY,
	//	SRV_AUTO,
	//};

	struct GpuSrvDesc
	{
		GpuSrvDesc() : format(GpuFormat::FORMAT_UNKNOWN), type(GpuResourceType::RESOURCE_UNKNOWN), texture{} {};

		GpuFormat format;

		// You may specify resource as RESOURCE_UNKNOWN in order to a view that accesses the entire resource
		GpuResourceType type;

		union
		{
			struct
			{
				uint32_t mipSlice = 0;
				uint32_t mipLevels = 0;
				uint32_t firstArraySlice = 0;
				uint32_t arraySize = 1;
			} texture;

			struct
			{
				uint32_t size = 0;
				uint32_t offset = 0;
			} buffer;
		};
	};

	struct GpuUavDesc
	{
		GpuUavDesc() : format(GpuFormat::FORMAT_UNKNOWN), type(GpuResourceType::RESOURCE_UNKNOWN), texture{} {};

		GpuFormat format;

		// You may specify resource as RESOURCE_UNKNOWN in order to a view that accesses the entire resource
		// Only RESOURCE_UNKNOWN, RESOURCE_BUFFER, RESOURCE_TEXTURE2D and RESOURCE_TEXTURE2D_ARRAY are applicable for UAV
		GpuResourceType type;

		union
		{
			struct
			{
				uint32_t mipSlice = 0;
				uint32_t firstArraySlice = 0;
				uint32_t arraySize = 1;
			} texture;

			struct
			{
				uint32_t firstElement = 0;
				uint32_t numElements = 0;
			} buffer;
		};
	};

}; // engi::gfx namespace