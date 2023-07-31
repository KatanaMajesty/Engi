#include "Renderer/ReflectionCapture.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/Renderer.h"
#include "Renderer/TextureLoader.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/Material.h"
#include "Renderer/ConstantBuffer.h"
#include "Renderer/Sampler.h"
#include "Renderer/MaterialRegistry.h"

namespace engi
{

	ReflectionCapture::ReflectionCapture(Renderer* renderer)
		: m_renderer(renderer)
	{
		ENGI_ASSERT(renderer && "Renderer cannot be nullptr");
	}

	bool ReflectionCapture::init(TextureCube* texture) noexcept
	{
		std::filesystem::path filepath = texture->getName();
		std::string filenameDiffuse = "IBL_Diffuse_" + filepath.filename().string();
		std::string filepathDiffuse = (m_iblPath / filenameDiffuse).string();
		m_diffuseIBL = loadDiffuseIBL(filepathDiffuse, texture);

		std::string filenameSpecular = "IBL_Specular_" + filepath.filename().string();
		std::string filepathSpecular = (m_iblPath / filenameSpecular).string();
		m_specularIBL = loadSpecularIBL(filepathSpecular, texture);

		std::string filenameLUT = "IBL_LUT.dds";
		std::string filepathLUT = (m_iblPath / filenameLUT).string();
		m_LUT = loadLUT(filepathLUT);

		if (!isInitialized())
		{
			ENGI_LOG_ERROR("Failed to initialize IBL");
			return false;
		}

		return true;
	}

	TextureCube* ReflectionCapture::loadDiffuseIBL(const std::string& filepath, TextureCube* skybox) noexcept
	{
		TextureLoader* textureLoader = m_renderer->getTextureLoader();
		TextureCube* diffuseIBL = textureLoader->loadTextureCube(filepath, true);
		if (!diffuseIBL)
		{
			diffuseIBL = generateDiffuseIBL(filepath, skybox);
		}
		return diffuseIBL;
	}

	TextureCube* ReflectionCapture::generateDiffuseIBL(const std::string& filepath, TextureCube* skybox) noexcept
	{
		using namespace gfx;
		ENGI_LOG_INFO("(ReflectionCapture): Generating Diffuse IBL texture at filepath {}", filepath);

		TextureLibrary* textureLibrary = m_renderer->getTextureLibrary();
		TextureCube* texture = textureLibrary->createTextureCube(filepath);
		if (!texture)
		{
			ENGI_LOG_ERROR("Failed to create a texture cube for diffuse ibl");
			return nullptr;
		}
		
		if (!texture->init(8, 8, 1, 1, RGBA16F) || !texture->initCubeShaderView())
		{
			ENGI_LOG_CRITICAL("Failed to init diffuse IBL texture");
		}

		ShaderLibrary* shaderLibrary = m_renderer->getShaderLibrary();
		ShaderProgram* program = shaderLibrary->createProgram("IBL_Diffuse_Precompute.hlsl", false, false);
		if (!program)
		{
			ENGI_LOG_ERROR("Failed to create IBL Diffuse generation shader");
			return nullptr;
		}

		RenderPass precomputeRenderPass;
		precomputeRenderPass.setViewport(texture->getWidth(), texture->getHeight());
		
		// 6 is amount of faces in cube...
		for (uint32_t slice = 0; slice < 6; ++slice)
			precomputeRenderPass.setRenderTarget(slice, texture->getHandle(), 0, slice, true);

		m_renderer->beginRenderPass(precomputeRenderPass);
		{
			if (!m_diffuseMat)
			{
				MaterialRegistry* materialRegistry = m_renderer->getMaterialRegistry();
				m_diffuseMat = materialRegistry->registerMaterial("IBL_Diffuse_Mat");
				m_diffuseMat->setShader(program);
				m_diffuseMat->getDepthStencilState().depthWritable = false;
				m_diffuseMat->getRasterizerState().culling = CULLING_NONE;
				if (!m_diffuseMat->init())
				{
					ENGI_LOG_ERROR("Failed to create diffuse IBL material");
					return nullptr;
				}
			}

			Sampler* linearSampler = m_renderer->getLinearSampler();
			ENGI_ASSERT(linearSampler && "No sampler was provided");

			skybox->bindCubeShaderView(15, PIXEL_SHADER);
			m_diffuseMat->bind();
			linearSampler->bind(1);
			m_renderer->draw(3, 0);
		}
		m_renderer->endRenderPass();

		TextureLoader* textureLoader = m_renderer->getTextureLoader();
		if (!textureLoader->saveToFile(texture, filepath, false, COMPRESSION_BC6_UNSIGNED))
		{
			ENGI_LOG_ERROR("Failed to generate diffuseIBL. Couldn't save the texture");
			delete texture;
			return nullptr;
		}

		return texture;
	}

	TextureCube* ReflectionCapture::loadSpecularIBL(const std::string& filepath, TextureCube* skybox) noexcept
	{
		TextureLoader* textureLoader = m_renderer->getTextureLoader();
		TextureCube* specularIBL = textureLoader->loadTextureCube(filepath, true);
		if (!specularIBL)
		{
			specularIBL = generateSpecularIBL(filepath, skybox);
		}
		return specularIBL;
	}

	struct alignas(16) SpecularPrecomputeConstant
	{
		uint32_t cubeSize;
		uint32_t mipSlice;
		uint32_t mipLevels;
	};

	TextureCube* ReflectionCapture::generateSpecularIBL(const std::string& filepath, TextureCube* skybox) noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(skybox->getNumMips() == skybox->getMaxMips());
		ENGI_LOG_INFO("(ReflectionCapture): Generating Specular IBL texture at filepath {}", filepath);

		TextureLoader* textureLoader = m_renderer->getTextureLoader();
		TextureCube* texture = textureLoader->getLibrary()->createTextureCube(filepath);
		if (!texture)
		{
			ENGI_LOG_ERROR("Failed to create a texture cube for specular ibl");
			return nullptr;
		}

		// 0 mip levels will generate 11 of those
		if (!texture->init(1024, 1024, 0, 1, RGBA16F) || !texture->initCubeShaderView())
		{
			ENGI_LOG_CRITICAL("Failed to init specular IBL texture");
		}

		ShaderLibrary* shaderLibrary = m_renderer->getShaderLibrary();
		ShaderProgram* program = shaderLibrary->createProgram("IBL_Specular_Precompute.hlsl", false, false);
		if (!program)
		{
			ENGI_LOG_ERROR("Failed to create IBL Specular generation shader");
			return nullptr;
		}

		if (!m_specularMat)
		{
			MaterialRegistry* materialRegistry = m_renderer->getMaterialRegistry();
			m_specularMat = materialRegistry->registerMaterial("IBL_Specular_Mat");
			m_specularMat->setShader(program);
			m_specularMat->getDepthStencilState().depthWritable = false;
			m_specularMat->getRasterizerState().culling = CULLING_NONE;
			if (!m_specularMat->init())
			{
				ENGI_LOG_ERROR("Failed to create specular IBL material");
				return nullptr;
			}
		}

		if (!m_specularIrradianceCB)
		{
			m_specularIrradianceCB.reset(m_renderer->createConstantBuffer("IBL_Specular_CB::Slot_8", sizeof(SpecularPrecomputeConstant)));
			ENGI_ASSERT(m_specularIrradianceCB);
		}

		Sampler* linearSampler = m_renderer->getLinearSampler();
		ENGI_ASSERT(linearSampler && "No sampler was provided");

		// Set states, that wont change
		linearSampler->bind(1);
		m_specularMat->bind();
		m_renderer->bindShaderResourceCube(skybox, 15, PIXEL_SHADER);

		uint32_t cubeSize = texture->getWidth();
		uint32_t numMips = texture->getNumMips();
		ENGI_ASSERT(numMips == texture->getMaxMips() && "Error while creating a specular IBL texture");
		for (uint32_t mip = 0; mip < numMips; ++mip)
		{
			RenderPass precomputeRenderPass;
			precomputeRenderPass.setViewport(cubeSize, cubeSize);

			for (uint32_t slice = 0; slice < 6; ++slice)
				precomputeRenderPass.setRenderTarget(slice, texture->getHandle(), mip, slice, true);

			m_renderer->beginRenderPass(precomputeRenderPass);
			{
				SpecularPrecomputeConstant specularConstant;
				specularConstant.cubeSize = cubeSize;
				specularConstant.mipSlice = mip;
				specularConstant.mipLevels = numMips;
				m_specularIrradianceCB->upload(0, &specularConstant, sizeof(SpecularPrecomputeConstant));
				m_specularIrradianceCB->bind(8, PIXEL_SHADER);

				m_renderer->draw(3, 0);
			}
			m_renderer->endRenderPass();
			cubeSize /= 2;
		}
		
		if (!textureLoader->saveToFile(texture, filepath, false, COMPRESSION_BC6_UNSIGNED))
		{
			ENGI_LOG_ERROR("Failed to generate specularIBL. Couldn't save the texture");
			delete texture;
			return nullptr;
		}

		return texture;
	}

	Texture2D* ReflectionCapture::loadLUT(const std::string& filepath) noexcept
	{
		TextureLoader* textureLoader = m_renderer->getTextureLoader();
		Texture2D* lut = textureLoader->loadTexture2D(filepath, true);
		if (!lut)
		{
			lut = generateLUT(filepath);
		}
		return lut;
	}

	Texture2D* ReflectionCapture::generateLUT(const std::string& filepath) noexcept
	{
		using namespace gfx;

		ENGI_LOG_INFO("(ReflectionCapture): Generating Look-Up texture at filepath {}", filepath);
		TextureLoader* textureLoader = m_renderer->getTextureLoader();
		Texture2D* lut = textureLoader->getLibrary()->createTexture2D(filepath);
		if (!lut)
		{
			ENGI_LOG_ERROR("Failed to create a lookup texture");
			return nullptr;
		}

		if (!lut->init(256, 256, 1, 1, RG16F) || !lut->initShaderView())
			ENGI_LOG_CRITICAL("Failed to init look-up texture");

		ShaderLibrary* shaderLibrary = m_renderer->getShaderLibrary();
		ShaderProgram* program = shaderLibrary->createProgram("IBL_LUT_Precompute.hlsl", false, false);
		if (!program)
		{
			ENGI_LOG_ERROR("Failed to create IBL LUT generation shader");
			return nullptr;
		}

		RenderPass precomputeRenderPass;
		precomputeRenderPass.setViewport(lut->getWidth(), lut->getHeight());
		precomputeRenderPass.setRenderTarget(0, lut->getHandle(), 0, 0, true);

		m_renderer->beginRenderPass(precomputeRenderPass);
		{
			if (!m_lutMat)
			{
				MaterialRegistry* materialRegistry = m_renderer->getMaterialRegistry();
				m_lutMat = materialRegistry->registerMaterial("IBL_Lut_Mat");
				m_lutMat->setShader(program);
				m_lutMat->getDepthStencilState().depthEnabled = false;
				m_lutMat->getRasterizerState().culling = CULLING_NONE;
				if (!m_lutMat->init())
				{
					ENGI_LOG_ERROR("Failed to create lut material");
					return nullptr;
				}
			}

			m_lutMat->bind();
			m_renderer->draw(3, 0);
		}
		m_renderer->endRenderPass();

		if (!textureLoader->saveToFile(lut, filepath, false, COMPRESSION_BC5_UNSIGNED))
		{
			ENGI_LOG_ERROR("Failed to generate IBL_Lut texture. Couldn't save the texture");
			delete lut;
			return nullptr;
		}

		return lut;
	}

}; // engi namespace