#include "Renderer/PostProcessor.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/GPUTexture.h"
#include "Renderer/Renderer.h"
#include "Renderer/ConstantBuffer.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/Material.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureLibrary.h"

namespace engi
{

	struct alignas(16) PostprocessorData
	{
		float ev100;
		float gamma;
	};

	struct alignas(16) GpuFXAASpecification
	{
		math::Vec4 dimensions; // (width, height, invWidth, invHeight)

		float qualitySubpix;
		float qualityEdgeThreshold;
		float qualityEdgeThresholdMin;
	};

	PostProcessor::PostProcessor(Renderer* renderer)
		: m_renderer(renderer)
	{
		ENGI_ASSERT(renderer && "Renderer cannot be nullptr");
	}

	bool PostProcessor::init() noexcept
	{
		ShaderLibrary* shaderLibrary = m_renderer->getShaderLibrary();
		ShaderProgram* shaderProgram = shaderLibrary->createProgram("PostProcessor.hlsl", false, false);
		if (!shaderProgram)
		{
			ENGI_LOG_ERROR("Failed to initialize PostProcessor. Failed to create post-processing shader");
			return false;
		}
		shaderProgram->setAttributeLayout({});

		MaterialRegistry* materialRegistry = m_renderer->getMaterialRegistry();
		m_tonemapMaterial = materialRegistry->registerMaterial("PostProcessor_Tonemapper_Mat");
		m_tonemapMaterial->setShader(shaderProgram);
		m_tonemapMaterial->getDepthStencilState().depthWritable = false;
		m_tonemapMaterial->getRasterizerState().ccwFront = true;
		if (!m_tonemapMaterial->init())
		{
			ENGI_LOG_ERROR("Failed to initialize PostProcessor's material");
			return false;
		}

		shaderProgram = shaderLibrary->createProgram("FXAA.hlsl", false, false);
		if (!shaderProgram)
		{
			ENGI_LOG_ERROR("Failed to create FXAA shader program. FXAA will be disabled");
			m_isFXAAInitted = false;
		}

		m_antiAliasingMaterial = materialRegistry->registerMaterial("PostProcessor_AA_Mat");
		m_antiAliasingMaterial->setShader(shaderProgram);
		m_antiAliasingMaterial->getDepthStencilState().depthEnabled = false;
		m_antiAliasingMaterial->getRasterizerState().ccwFront = true;
		if (!m_antiAliasingMaterial->init())
		{
			ENGI_LOG_ERROR("Failed to initialize Anti-Aliasing material. FXAA will be disabled");
			m_isFXAAInitted = false;
		}

		m_tonemapData = makeUnique<ConstantBuffer>(m_renderer->createConstantBuffer("PostProcessor::TonemappingData", sizeof(PostprocessorData)));
		if (!m_tonemapData)
		{
			ENGI_LOG_WARN("Failed to create data buffer for PostProcessor");
			return false;
		}

		m_antiAliasingSpec = makeUnique<ConstantBuffer>(m_renderer->createConstantBuffer("PostProcessor::AntiAliasingSpec", sizeof(GpuFXAASpecification)));
		if (!m_antiAliasingSpec)
		{
			ENGI_LOG_WARN("Failed to create FXAA specification for PostProcessor. FXAA will be disabled");
			m_isFXAAInitted = false;
		}

		return true;
	}

	void PostProcessor::resolve(Texture2D* target, float ev100, float gamma) noexcept
	{
		ENGI_ASSERT(target);
		this->resolve(target->getHandle(), ev100, gamma);
	}

	void PostProcessor::resolve(gfx::IGpuTexture* target, float ev100, float gamma) noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(target);

		Texture2D* HDRTexture = m_renderer->getHDRTargetTexture();
		ENGI_ASSERT(HDRTexture && "Renderer's HDR Texture to resolve was nullptr"); // Will probably never occur, won't remove for now though
		uint32_t width = HDRTexture->getWidth();
		uint32_t height = HDRTexture->getHeight();

		RenderPass postFXRenderPass;
		postFXRenderPass.setViewport(width, height);
		postFXRenderPass.setRenderTarget(0, target, 0, 0, true);
		m_renderer->beginRenderPass(postFXRenderPass);
		{
			PostprocessorData data;
			data.ev100 = ev100;
			data.gamma = gamma;
			m_tonemapData->upload(0, &data, sizeof(PostprocessorData));
			m_tonemapData->bind(0, PIXEL_SHADER);
			m_tonemapMaterial->bind();

			m_renderer->bindShaderResource2D(HDRTexture, 0, PIXEL_SHADER);
			m_renderer->draw(3, 0);
			m_renderer->bindShaderResource2D(nullptr, 0, PIXEL_SHADER);
		}
		m_renderer->endRenderPass();
	}

	void PostProcessor::applyFXAA(Texture2D* src, const FXAASpec& fxaaSpec) noexcept
	{
		using namespace gfx;

		if (!this->isFXAAInitted())
			return;

		ENGI_ASSERT(src);
		gfx::IGpuTexture* backbuffer = m_renderer->getBackbuffer();
		uint32_t width = m_renderer->getBackbufferWidth();
		uint32_t height = m_renderer->getBackbufferHeight();

		RenderPass AARenderPass;
		AARenderPass.setViewport(width, height);
		AARenderPass.setRenderTarget(0, backbuffer, 0, 0, true);
		m_renderer->beginRenderPass(AARenderPass);
		{
			GpuFXAASpecification gpuFxaaSpec;
			gpuFxaaSpec.dimensions = math::Vec4(static_cast<float>(width), static_cast<float>(height), 1.0f / width, 1.0f / height);
			gpuFxaaSpec.qualitySubpix = fxaaSpec.qualitySubpix;
			gpuFxaaSpec.qualityEdgeThreshold = fxaaSpec.qualityEdgeThreshold;
			gpuFxaaSpec.qualityEdgeThresholdMin = fxaaSpec.qualityEdgeThresholdMin;

			m_antiAliasingSpec->upload(0, &gpuFxaaSpec, sizeof(GpuFXAASpecification));
			m_antiAliasingSpec->bind(0, PIXEL_SHADER);
			m_antiAliasingMaterial->bind();
			m_renderer->bindShaderResource2D(src, 0, PIXEL_SHADER);
			m_renderer->draw(3, 0);
			m_renderer->bindShaderResource2D(nullptr, 0, PIXEL_SHADER);
		}
		m_renderer->endRenderPass();
	}

}; // engi namespace