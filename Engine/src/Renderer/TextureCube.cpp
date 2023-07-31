#include "Renderer/TextureCube.h"

#include "Math/Math.h"
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/Definitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUTexture.h"
#include "GFX/GPUDescriptor.h"

namespace engi
{

	TextureCube::TextureCube(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	TextureCube::~TextureCube()
	{
	}

	bool TextureCube::init(gfx::IGpuTexture* handle) noexcept
	{
		using namespace gfx;
		ENGI_ASSERT(handle && "Provided handle is nullptr");
		ENGI_ASSERT(handle->getDesc().arraySize == 6);
		if (handle->getDesc().type != TEXTURE2D)
			return false;

		if ((handle->getDesc().otherFlags & MISC_TEXTURECUBE) == 0)
			return false;

		m_handle = gfx::makeGpuHandle(handle, m_device->getResourceAllocator());
		return true;
	}

	bool TextureCube::init(uint32_t width, uint32_t height, uint32_t mips, uint32_t numCubes, gfx::GpuFormat format, uint32_t pipelineFlags, gfx::GpuUsage usage, uint32_t cpuFlags) noexcept
	{
		gfx::GpuTextureDesc desc;
		desc.type = gfx::TEXTURE2D;
		desc.width = width;
		desc.height = height;
		desc.miplevels = mips;
		desc.arraySize = 6 * numCubes; // each cubemap takes 6 Texture2Ds
		desc.format = format;
		desc.usage = usage;
		desc.pipelineFlags = pipelineFlags;
		desc.cpuFlags = cpuFlags;
		desc.otherFlags = gfx::GpuMisc::MISC_TEXTURECUBE;

		m_handle.reset();
		m_handle = gfx::makeGpuHandle(m_device->createTexture("TextureCube_" + getName(), desc, nullptr), m_device->getResourceAllocator());
		if (!m_handle)
			return false;

		return true;
	}

	bool TextureCube::initCubeShaderView(gfx::GpuFormat format) noexcept
	{
		uint32_t numCubes = getArraySize();
		bool isArray = (numCubes > 1);
		bool useHandleFormat = (format == gfx::GpuFormat::FORMAT_UNKNOWN);
		gfx::GpuSrvDesc srvDesc;
		srvDesc.type = isArray ? gfx::GpuResourceType::RESOURCE_TEXTURECUBE_ARRAY : gfx::GpuResourceType::RESOURCE_TEXTURECUBE;
		srvDesc.format = useHandleFormat ? m_handle->getDesc().format : format;
		srvDesc.texture.arraySize = numCubes * 6; // Use numCubes * 6 instead of desc.arraySize to avoid possible srv runtime problems with arraysize
		srvDesc.texture.firstArraySlice = 0;
		srvDesc.texture.mipLevels = m_handle->getDesc().miplevels;
		srvDesc.texture.mipSlice = 0;

		m_cubeSrv.reset();
		m_cubeSrv = gfx::makeGpuHandle(m_device->createSRV("TextureCube_CubeSrv::" + getName(), srvDesc, m_handle.get()), m_device->getResourceAllocator());
		if (!m_cubeSrv)
		{
			ENGI_LOG_WARN("Failed to init cube shader resource view of a TextureCube: {}", getName());
			return false;
		}

		return true;
	}

	bool TextureCube::initArrayShaderView(gfx::GpuFormat format) noexcept
	{
		// If the TextureCube is TEXTURE_CUBE_ARRAY, then we do not create the SRV for Texture2DArray
		uint32_t numCubes = getArraySize();
		if (numCubes > 1)
		{
			ENGI_LOG_WARN("Cannot create a Texture2DArray SRV for TextureCube {}, as it seems to be an array (numCubes = {})", getName(), numCubes);
			return false;
		}

		bool useHandleFormat = (format == gfx::GpuFormat::FORMAT_UNKNOWN);
		gfx::GpuSrvDesc srvDesc;
		srvDesc.type = gfx::GpuResourceType::RESOURCE_TEXTURE2D_ARRAY;
		srvDesc.format = useHandleFormat ? m_handle->getDesc().format : format;
		srvDesc.texture.arraySize = 6;
		srvDesc.texture.firstArraySlice = 0;
		srvDesc.texture.mipLevels = m_handle->getDesc().miplevels;
		srvDesc.texture.mipSlice = 0;

		m_arraySrv.reset();
		m_arraySrv = gfx::makeGpuHandle(m_device->createSRV("TextureCube_Array2DSrv::" + getName(), srvDesc, m_handle.get()), m_device->getResourceAllocator());
		if (!m_arraySrv)
		{
			ENGI_LOG_WARN("Failed to init array shader resource view of a TextureCube: {}", getName());
			return false;
		}

		return true;
	}

	uint32_t TextureCube::getNumMips() const noexcept
	{
		return m_handle->getDesc().miplevels;
	}

	uint32_t TextureCube::getMaxMips() const noexcept
	{
		math::Vec2 res(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
		return static_cast<uint32_t>(math::log2(res.getMin())) + 1;
	}

	uint32_t TextureCube::getWidth() const noexcept
	{
		return m_handle->getDesc().width;
	}

	uint32_t TextureCube::getHeight() const noexcept
	{
		return m_handle->getDesc().height;
	}

	uint32_t TextureCube::getArraySize() const noexcept
	{
		ENGI_ASSERT(m_handle->getDesc().arraySize % 6 == 0 && "TextureCube was not initialized correctly (arraysize is not multiples of 6)");
		return m_handle->getDesc().arraySize / 6;
	}

	void TextureCube::bindCubeShaderView(uint32_t slot, uint32_t shaderTypes) noexcept
	{
		ENGI_ASSERT(m_cubeSrv);
		m_device->setSRV(m_cubeSrv.get(), slot, shaderTypes);
	}

	void TextureCube::bindArrayShaderView(uint32_t slot, uint32_t shaderTypes) noexcept
	{
		ENGI_ASSERT(m_arraySrv);
		m_device->setSRV(m_arraySrv.get(), slot, shaderTypes);
	}

}; // engi namespace