#include "Renderer/Texture2D.h"

#include "Math/Math.h"
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUDescriptor.h"
#include "GFX/GPUTexture.h"

namespace engi
{

	Texture2D::Texture2D(const std::string& name, gfx::IGpuDevice* device)
		: m_name(name)
		, m_device(device)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	Texture2D::~Texture2D()
	{
	}

	bool Texture2D::init(gfx::IGpuTexture* handle) noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(handle && "Provided handle is nullptr");
		if (handle->getDesc().type != TEXTURE2D)
			return false;

		if ((handle->getDesc().otherFlags & MISC_TEXTURECUBE) != 0)
			return false;

		m_handle = gfx::makeGpuHandle(handle, m_device->getResourceAllocator());
		return true;
	}

	bool Texture2D::init(uint32_t width, uint32_t height, uint32_t mips, uint32_t arraySize, gfx::GpuFormat format, uint32_t pipelineFlags, gfx::GpuUsage usage, uint32_t cpuFlags) noexcept
	{
		gfx::GpuTextureDesc desc;
		desc.type = gfx::TEXTURE2D;
		desc.width = width;
		desc.height = height;
		desc.miplevels = mips;
		desc.arraySize = arraySize;
		desc.format = format;
		desc.usage = usage;
		desc.pipelineFlags = pipelineFlags;
		desc.cpuFlags = cpuFlags;
		desc.otherFlags = 0;

		m_handle.reset();
		m_handle = gfx::makeGpuHandle(m_device->createTexture("Texture2D_" + getName(), desc, nullptr), m_device->getResourceAllocator());
		if (!m_handle)
		{
			ENGI_LOG_WARN("Failed to init texture handle for a Texture2D: {}", getName());
			return false;
		}

		return true;
	}

	bool Texture2D::initShaderView(gfx::GpuFormat format) noexcept
	{
		using namespace gfx;

		bool isArray = (getArraySize() > 1);
		GpuSrvDesc srvDesc;
		srvDesc.type = isArray ? GpuResourceType::RESOURCE_TEXTURE2D_ARRAY : GpuResourceType::RESOURCE_TEXTURE2D;
		srvDesc.format = (format == GpuFormat::FORMAT_UNKNOWN) ? m_handle->getDesc().format : format;
		srvDesc.texture.arraySize = getArraySize();
		srvDesc.texture.firstArraySlice = 0;
		srvDesc.texture.mipLevels = getNumMips();
		srvDesc.texture.mipSlice = 0;

		m_srv.reset();
		m_srv = gfx::makeGpuHandle(m_device->createSRV("Texture2D_Srv::" + getName(), srvDesc, m_handle.get()), m_device->getResourceAllocator());
		if (!m_srv)
		{
			ENGI_LOG_WARN("Failed to init shader resource view of a Texture2D: {}", getName());
			return false;
		}

		return true;
	}

	void Texture2D::bindShaderView(uint32_t slot, uint32_t shaderTypes) noexcept
	{
		ENGI_ASSERT(m_srv);
		m_device->setSRV(m_srv.get(), slot, shaderTypes);
	}

	void Texture2D::copyFrom(Texture2D* srcTexture, const Texture2DCopyState& copyState) noexcept
	{
		ENGI_ASSERT(srcTexture && m_device == srcTexture->m_device);

		uint32_t depth = 1;
		if (copyState.width == 0 && copyState.height == 0)
		{
			// If both width and height are zeros, that means that we want to copy the whole texture, thus we would need to provide 0 in to the depth as well
			depth = 0;
		}

		m_device->copyTexture(m_handle.get(), 
			copyState.dstMipslice, 
			copyState.dstArrayslice, 
			copyState.dstX, copyState.dstY, 0,
			srcTexture->m_handle.get(), 
			copyState.srcMipslice, 
			copyState.srcArrayslice, 
			copyState.srcX, copyState.srcY, 0, 
			copyState.width, copyState.height, depth);
	}

	uint32_t Texture2D::getNumMips() const noexcept
	{
		ENGI_ASSERT(m_handle);
		return m_handle->getDesc().miplevels;
	}

	uint32_t Texture2D::getMaxMips() const noexcept
	{
		math::Vec2 res(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
		return static_cast<uint32_t>(math::log2(res.getMin())) + 1;
	}

	uint32_t Texture2D::getWidth() const noexcept
	{
		ENGI_ASSERT(m_handle);
		return m_handle->getDesc().width;
	}

	uint32_t Texture2D::getHeight() const noexcept
	{
		ENGI_ASSERT(m_handle);
		return m_handle->getDesc().height;
	}

	uint32_t Texture2D::getArraySize() const noexcept
	{
		ENGI_ASSERT(m_handle);
		return m_handle->getDesc().arraySize;
	}

}; // engi namespace