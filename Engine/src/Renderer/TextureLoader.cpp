#include "Renderer/TextureLoader.h"

#include <filesystem>
#include <DDSTextureLoader11.h>
#include <DirectXTex.h>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUTexture.h"
#include "GFX/GPUDescriptor.h"
#include "GFX/DX11/D3D11_Device.h"
#include "GFX/DX11/D3D11_Texture.h"
#include "GFX/DX11/D3D11_Descriptor.h"
#include "GFX/DX11/D3D11_Utility.h"

// TODO: Remove this header
#include <iostream>

namespace engi
{

	using namespace gfx;

	TextureLoader::TextureLoader(gfx::IGpuDevice* device, TextureLibrary* textureLibrary)
		: m_device(device)
		, m_textureLibrary(textureLibrary)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	static void D3D11FillTextureDescAsTexture1D(ID3D11Resource* resource, GpuTextureDesc* desc)
	{
		ID3D11Texture1D* texture = (ID3D11Texture1D*)resource;
		D3D11_TEXTURE1D_DESC d3dDesc;
		texture->GetDesc(&d3dDesc);

		desc->type = GpuTextureType::TEXTURE1D;
		desc->width = d3dDesc.Width;
		desc->miplevels = d3dDesc.MipLevels;
		desc->arraySize = d3dDesc.ArraySize;
		desc->format = detail::d3dToGpuFormat(d3dDesc.Format);
		desc->usage = GpuUsage::IMMUTABLE; // we always use immutable
		desc->pipelineFlags = GpuBinding::SHADER_RESOURCE;
		desc->cpuFlags = CpuAccess::ACCESS_UNUSED;
		desc->otherFlags = detail::d3dToGpuMiscFlags(d3dDesc.MiscFlags);
	}

	static void D3D11FillTextureDescAsTexture2D(ID3D11Resource* resource, GpuTextureDesc* desc)
	{
		ID3D11Texture2D* texture = (ID3D11Texture2D*)resource;
		D3D11_TEXTURE2D_DESC d3dDesc;
		texture->GetDesc(&d3dDesc);

		desc->type = GpuTextureType::TEXTURE2D;
		desc->width = d3dDesc.Width;
		desc->height = d3dDesc.Height;
		desc->miplevels = d3dDesc.MipLevels;
		desc->arraySize = d3dDesc.ArraySize;
		desc->format = detail::d3dToGpuFormat(d3dDesc.Format);
		desc->usage = GpuUsage::IMMUTABLE; // we always use immutable
		desc->pipelineFlags = GpuBinding::SHADER_RESOURCE;
		desc->cpuFlags = CpuAccess::ACCESS_UNUSED;
		desc->otherFlags = detail::d3dToGpuMiscFlags(d3dDesc.MiscFlags);
	}

	static void D3D11FillTextureDescAsTexture3D(ID3D11Resource* resource, GpuTextureDesc* desc)
	{
		ID3D11Texture3D* texture = (ID3D11Texture3D*)resource;
		D3D11_TEXTURE3D_DESC d3dDesc;
		texture->GetDesc(&d3dDesc);

		desc->type = GpuTextureType::TEXTURE3D;
		desc->width = d3dDesc.Width;
		desc->height = d3dDesc.Height;
		desc->depth = d3dDesc.Width;
		desc->miplevels = d3dDesc.MipLevels;
		desc->format = detail::d3dToGpuFormat(d3dDesc.Format);
		desc->usage = GpuUsage::IMMUTABLE; // we always use immutable
		desc->pipelineFlags = GpuBinding::SHADER_RESOURCE;
		desc->cpuFlags = CpuAccess::ACCESS_UNUSED;
		desc->otherFlags = detail::d3dToGpuMiscFlags(d3dDesc.MiscFlags);
	}

	Texture2D* TextureLoader::loadTextureAtlas(const std::string& filepath, uint32_t numWidthTextures, uint32_t numHeightTextures, bool requestSrv) noexcept
	{
		// We firstly load the texture as simple texture2D, which we will then process separately
		Texture2D* texture = this->loadTexture2D(filepath, requestSrv);
		if (!texture)
		{
			ENGI_LOG_WARN("Failed to create TextureAtlas. Could not load Texture2D");
			return nullptr;
		}

		ENGI_ASSERT(texture->getHandle() && "Unexpected behavior of an Engine. Not handled");
		const auto& textureDesc = texture->getHandle()->getDesc();
		uint32_t srcWidth = texture->getWidth();
		uint32_t srcHeight = texture->getHeight();
		uint32_t chunkWidth = srcWidth / numWidthTextures; ENGI_ASSERT(srcWidth % numWidthTextures == 0 && chunkWidth > 0 && "Provided parameters do not fit the texture");
		uint32_t chunkHeight = srcHeight / numHeightTextures; ENGI_ASSERT(srcHeight % numHeightTextures == 0 && chunkHeight > 0 && "Provided parameters do not fit the texture");
		uint32_t numTextures = numWidthTextures * numHeightTextures;
		uint32_t numMips = texture->getNumMips();

		// TODO: We temporarily create a texture using new keyword. It is preferred to use TextureRegistry, but due to limitation of the method, we cannot do that
		// as the Texture2D* texture is already registered under that filepath. Thus we create a texture manually and then will replace the one in the Registry
		Texture2D* atlas = new Texture2D(filepath, m_device);

		if (!atlas->init(chunkWidth, chunkHeight, 1, numTextures, textureDesc.format, textureDesc.pipelineFlags, gfx::GpuUsage::DEFAULT, textureDesc.cpuFlags))
		{
			ENGI_LOG_WARN("Failed to initialize texture atlas. Could not initialize texture atlas");
			delete atlas;
			return nullptr;
		}

		for (uint32_t x = 0; x < numWidthTextures; ++x)
		{
			for (uint32_t y = 0; y < numHeightTextures; ++y)
			{
				// Remark! We only copy first mip level, as it is easier
				Texture2DCopyState copyState;
				copyState.dstMipslice = 0;
				copyState.dstArrayslice = y * numWidthTextures + x;
				copyState.dstX = 0;
				copyState.dstY = 0;
				copyState.srcMipslice = 0;
				copyState.srcArrayslice = 0; // We only copy from the first array slice
				copyState.srcX = x * chunkWidth;
				copyState.srcY = y * chunkHeight;
				copyState.width = chunkWidth;
				copyState.height = chunkHeight;
				atlas->copyFrom(texture, copyState);
			}
		}

		// TODO: Remove the Texture2D that was used to create TextureAtlas from the TextureRegistry, as it is unnecessary and obsolete
		// m_textureRegistry->removeTexture2D(filepath);

		if (requestSrv)
		{
			// If not - do something
			atlas->initShaderView();
		}

		bool R = m_textureLibrary->removeTexture2D(filepath); ENGI_ASSERT(R);
		m_textureLibrary->addTexture2D(filepath, atlas);
		
		ENGI_LOG_TRACE("Created a Texture2DAtlas of {}", filepath);
		return atlas;
	}

	Texture2D* TextureLoader::loadTexture2D(const std::string& filepath, bool requestSrv) noexcept
	{
		IGpuTexture* gpuTexture = loadGPUTextureFromDDS(filepath);
		if (!gpuTexture)
			return nullptr;

		Texture2D* texture = m_textureLibrary->createTexture2D(filepath);
		if (!texture->init(gpuTexture))
		{
			ENGI_LOG_WARN("Failed to load texture 2d {}", filepath);
			return nullptr;
		}

		if (requestSrv)
			texture->initShaderView();

		return texture;
	}

	TextureCube* TextureLoader::loadTextureCube(const std::string& filepath, bool requestSrv) noexcept
	{
		IGpuTexture* gpuTexture = loadGPUTextureFromDDS(filepath);
		if (!gpuTexture)
			return nullptr;

		TextureCube* texture = m_textureLibrary->createTextureCube(filepath);
		if (!texture->init(gpuTexture))
		{
			ENGI_LOG_WARN("Failed to load texture cube {}", filepath);
			return nullptr;
		}

		if (requestSrv)
		{
			texture->initCubeShaderView();
			texture->initArrayShaderView();
		}

		return texture;
	}

	bool TextureLoader::saveToFile(Texture2D* texture, const std::string& filepath, bool mips, CompressionFormat format) noexcept
	{
		ENGI_ASSERT(texture);
		IGpuTexture* gpuTexture = texture->getHandle();
		ENGI_ASSERT(gpuTexture);
		ENGI_ASSERT(gpuTexture->getDesc().type == GpuTextureType::TEXTURE2D);
		if (!saveGPUTextureToDDS(gpuTexture, filepath, mips, format))
			return false;

		return true;
	}

	bool TextureLoader::saveToFile(TextureCube* texture, const std::string& filepath, bool mips, CompressionFormat format) noexcept
	{
		ENGI_ASSERT(texture);
		IGpuTexture* gpuTexture = texture->getHandle();
		ENGI_ASSERT(gpuTexture);
		ENGI_ASSERT(gpuTexture->getDesc().type == GpuTextureType::TEXTURE2D);
		ENGI_ASSERT(gpuTexture->getDesc().arraySize == 6);
		if (!saveGPUTextureToDDS(gpuTexture, filepath, mips, format))
			return false;

		return true;
	}

	gfx::IGpuTexture* TextureLoader::loadGPUTextureFromDDS(const std::string& filepath) noexcept
	{
		D3D11Device* device = dynamic_cast<D3D11Device*>(m_device);
		if (!device)
		{
			std::cout << "TextureLoader cannot load from dds for the provided logical device (not D3D11)\n";
			return nullptr;
		}

		GpuResourceAllocator* gpuAllocator = device->getResourceAllocator();
		IGpuTexture* texture = gpuAllocator->createResource<D3D11Texture>(filepath, device, GpuTextureDesc{});

		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11Texture* gpuTexture = (D3D11Texture*)texture;
		std::wstring wfilepath(filepath.begin(), filepath.end());
		HRESULT hr = DirectX::CreateDDSTextureFromFileEx(
			device->getHandle(),
			wfilepath.c_str(), 0,
			D3D11_USAGE_IMMUTABLE,
			bindFlags,
			0,
			0,
			DirectX::DDS_LOADER_DEFAULT,
			gpuTexture->m_handle.GetAddressOf(),
			nullptr);
		if (FAILED(hr))
		{
			gpuAllocator->destroyResource((IGpuResource*&)gpuTexture);
			return nullptr;
		}

		D3D11_RESOURCE_DIMENSION dimension;
		ID3D11Resource* d3dTexture = (ID3D11Resource*)gpuTexture->getHandle();
		d3dTexture->GetType(&dimension);

		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D: D3D11FillTextureDescAsTexture1D(d3dTexture, &gpuTexture->m_desc); break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D: D3D11FillTextureDescAsTexture2D(d3dTexture, &gpuTexture->m_desc); break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D: D3D11FillTextureDescAsTexture3D(d3dTexture, &gpuTexture->m_desc); break;
		case D3D11_RESOURCE_DIMENSION_UNKNOWN:
		case D3D11_RESOURCE_DIMENSION_BUFFER:
		default:
			gpuTexture->m_desc.type = GpuTextureType::TEXTURE_UNKNOWN; break; // we dont do anything with unknown type
		}

		if (gpuTexture->m_desc.type == GpuTextureType::TEXTURE_UNKNOWN)
		{
			std::cout << "Failed to determine " << filepath << " texture type\n";
			gpuAllocator->destroyResource((IGpuResource*&)gpuTexture);
			return nullptr;
		}

		if (gpuTexture->m_desc.format == GpuFormat::FORMAT_UNKNOWN)
		{
			std::cout << "Format of the " << filepath << " is not supported by Engi\n";
			gpuAllocator->destroyResource((IGpuResource*&)gpuTexture);
			return nullptr;
		}

#if !defined(_NDEBUG)
		bool isTextureCube = (gpuTexture->m_desc.otherFlags & MISC_TEXTURECUBE) != 0;
		const std::string& name = (isTextureCube ? "TextureCube_" : "Texture2D_") + filepath;
		if (d3dTexture)
			d3dTexture->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif

		return texture;
	}

	// TODO: Make an array
	static DXGI_FORMAT toDXGIFormat(CompressionFormat format)
	{
		switch (format)
		{
		case COMPRESSION_BC1_LINEAR: return DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
		case COMPRESSION_BC1_SRGB: return DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB;
		case COMPRESSION_BC3_LINEAR: return DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM;
		case COMPRESSION_BC3_SRGB: return DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB;
		case COMPRESSION_BC4_UNSIGNED: return DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM;
		case COMPRESSION_BC4_SIGNED: return DXGI_FORMAT::DXGI_FORMAT_BC4_SNORM;
		case COMPRESSION_BC5_UNSIGNED: return DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
		case COMPRESSION_BC5_SIGNED: return DXGI_FORMAT::DXGI_FORMAT_BC5_SNORM;
		case COMPRESSION_BC6_UNSIGNED: return DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16;
		case COMPRESSION_BC6_SIGNED: return DXGI_FORMAT::DXGI_FORMAT_BC6H_SF16;
		case COMPRESSION_BC7_LINEAR: return DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM;
		case COMPRESSION_BC7_SRGB: return DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB;
		default: ENGI_ASSERT(false); return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		}
	}

	bool TextureLoader::saveGPUTextureToDDS(gfx::IGpuTexture* texture, const std::string& filepath, bool mips, CompressionFormat format) noexcept
	{
		D3D11Device* device = dynamic_cast<D3D11Device*>(m_device);
		if (!device)
		{
			std::cout << "TextureLoader cannot save to dds for the provided logical device (not D3D11)\n";
			return false;
		}

		DirectX::ScratchImage image;
		HRESULT hr = DirectX::CaptureTexture(device->getHandle(), device->getContext(), (ID3D11Resource*)texture->getHandle(), image);
		if (FAILED(hr))
		{
			std::cout << "Failed to capture a texture from " << texture->getName() << std::endl;
			return false;
		}

		const DirectX::ScratchImage* pImage = &image;
		DirectX::ScratchImage mipChain;
		if (mips)
		{
			hr = DirectX::GenerateMipMaps(*(pImage->GetImage(0, 0, 0)), DirectX::TEX_FILTER_DEFAULT, 0, mipChain);
			if (FAILED(hr))
			{
				std::cout << "failed to generate mip maps. Skipping...\n";
			}
			else pImage = &mipChain;
		}

		DirectX::ScratchImage compressed;
		if (format != COMPRESSION_NONE)
		{
			if (format >= COMPRESSION_BC6_UNSIGNED && format <= COMPRESSION_BC7_SRGB)
			{
				hr = DirectX::Compress(device->getHandle(),
					pImage->GetImages(),
					pImage->GetImageCount(),
					pImage->GetMetadata(),
					toDXGIFormat(format), DirectX::TEX_COMPRESS_PARALLEL, 1.0f, compressed);
			}
			else
			{
				hr = DirectX::Compress(pImage->GetImages(),
					pImage->GetImageCount(),
					pImage->GetMetadata(),
					toDXGIFormat(format), DirectX::TEX_COMPRESS_PARALLEL, 1.0f, compressed);
			}

			if (FAILED(hr))
			{
				ENGI_LOG_WARN("Failed to compress the texture. Apparently it is already compressed or the format mismatch. Skipping...");
			}
			else pImage = &compressed;
		}
		
		std::wstring wfilepath(filepath.begin(), filepath.end());
		hr = DirectX::SaveToDDSFile(pImage->GetImages(), pImage->GetImageCount(), pImage->GetMetadata(), DirectX::DDS_FLAGS_NONE, wfilepath.c_str());
		if (FAILED(hr))
		{
			std::cout << "Failed to save the texture to " << filepath << "\n	Apparently some folders may be missing. Returning...\n";
			return false;
		}

		return true;
	}

}; // engi namespace