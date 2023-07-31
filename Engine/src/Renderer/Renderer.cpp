#include "Renderer/Renderer.h"

#include "Core/CommonDefinitions.h"
#include "Core/Logger.h"
#include "GFX/GPU.h"
#include "GFX/GPUShader.h"
#include "GFX/Definitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/ImGui.h"
#include "GFX/GPUSwapchain.h"
#include "GFX/GPUTexture.h"
#include "GFX/GPUSampler.h"
#include "GFX/GPUDescriptor.h"
#include "Renderer/ConstantBuffer.h"
#include "Renderer/Buffer.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/Material.h"
#include "Renderer/ModelRegistry.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureCube.h"
#include "Renderer/PostProcessor.h"
#include "Renderer/TextureLibrary.h"
#include "Renderer/TextureLoader.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Skybox.h"

namespace engi
{
	
	// TODO: Move this
	using namespace gfx;

	Renderer::Renderer()
	{
		ENGI_LOG_TRACE("[RENDERER] Size of renderer is {} bytes", sizeof(*this));
	}

	Renderer::~Renderer()
	{
		delete[] m_debugAABBRenderData.DrawData;
		if (m_imguiContext)
			m_imguiContext->deinitialize();
	}

	bool Renderer::init(void* handle, uint32_t width, uint32_t height) noexcept
	{
		ENGI_ASSERT(handle && "Window handle is nullptr");

		m_width = width;
		m_height = height;

		if (!this->createDevice())
			return false;

		// Initialize imgui context
		m_imguiContext = gfx::createImGuiContext(handle, m_device.get());
		ENGI_ASSERT(m_imguiContext && "Failed to create ImGui context");

		// Firstly we want to initialize Shader manager, materials, samplers and texture manager
		bool firstStage = initRegistries()
			&& initSamplers()
			&& initPostProcessor();
		if (!firstStage)
			return false;

		// Secondly we want to initialize swapchain and depthstencil state
		bool secondStage = createSwapchain(handle, width, height)
			&& initHDRRenderTarget()
			&& initDepthStencil();
		if (!secondStage)
			return false;

		ENGI_LOG_INFO("Initting gbuffers");
		if (!this->initGBuffers())
		{
			ENGI_LOG_WARN("Failed to init gbuffers");
			return false;
		}

		ENGI_LOG_INFO("Renderer was successfully initialized");
		this->initRenderData();

		return true;
	}

	void Renderer::resize(uint32_t width, uint32_t height) noexcept
	{
		ENGI_LOG_TRACE("Renderer::resize is fired with resolution of ({}, {})", width, height);

		if (width == 0 || height == 0)
		{
			ENGI_LOG_WARN("Renderer::resize was fired with invalid resolution of (0, 0). Skipping the resize event without handling");
			return;
		}

		if (width == m_width && height == m_height)
		{
			ENGI_LOG_INFO("Renderer::resize was fired with the same width and height of ({}, {}). Skipping the resize", m_width, m_height);
			return;
		}

		// resize swapchain first as it is considered the most prioritised operation
		m_swapchain->resize(width, height);
		m_width = width;
		m_height = height;

		if (!this->initDepthStencil())
		{
			ENGI_LOG_WARN("Failed to reinitialize depth-stencil texture on resize");
			ENGI_ASSERT(false);
			return;
		}

		if (!this->initHDRRenderTarget())
		{
			ENGI_LOG_WARN("Failed to reinitialize HDR render target on resize");
			ENGI_ASSERT(false);
			return;
		}

		if (!this->initGBuffers())
		{
			ENGI_LOG_WARN("Failed to reinitialize GBuffers on resize");
			ENGI_ASSERT(false);
			return;
		}
	}

	void Renderer::beginFrame()
	{
	}

	void Renderer::endFrame()
	{
		m_swapchain->present();
	}

	void Renderer::beginUIFrame()
	{
		ENGI_ASSERT(this->isImGuiInitialized());
		
		RenderPass uiRenderPass;
		uiRenderPass.setDepthStencilBuffer2D(this->getDepthStencilTexture(), 0, 0, true, true);
		uiRenderPass.setRenderTarget(0, this->getBackbuffer(), 0, 0, false);
		uiRenderPass.setViewport(m_width, m_height);

		this->beginRenderPass(uiRenderPass);
		m_imguiContext->beginFrame(); // It is important to start ImGui frame AFTER the 0 slot RTV is set
	}

	void Renderer::endUIFrame()
	{
		ENGI_ASSERT(this->isImGuiInitialized());

		m_imguiContext->endFrame();
		this->endRenderPass();
	}

	void Renderer::beginRenderPass(const RenderPass& renderPass) noexcept
	{
		m_device->setViewport(0, 0, renderPass.viewportWidth, renderPass.viewportHeight);
		m_device->beginRenderPass(renderPass.desc);
	}

	void Renderer::endRenderPass() noexcept
	{
		m_device->endRenderPass();
	}

	gfx::IGpuTexture* Renderer::getBackbuffer() noexcept
	{
		ENGI_ASSERT(m_swapchain);
		return m_swapchain->getBackbuffer();
	}

	Skybox* Renderer::createSkybox(const std::string& name, ShaderProgram* shader, ReflectionCapture* reflectionCapture) noexcept
	{
		ENGI_ASSERT(shader);

		Skybox* skybox = new Skybox(name, m_device.get());
		if (!skybox->init(shader, reflectionCapture))
		{
			ENGI_LOG_WARN("Failed to create skybox {}", name);
			delete skybox;
			return nullptr;
		}
		return skybox;
	}

	Buffer* Renderer::createResourceBuffer(const std::string& name) noexcept
	{
		Buffer* buffer = new Buffer(name, m_device.get());
		return buffer;
	}

	ConstantBuffer* Renderer::createConstantBuffer(const std::string& name, uint32_t size) noexcept
	{
		ENGI_ASSERT(size > 0);

		ConstantBuffer* buffer = new ConstantBuffer(name, m_device.get());
		if (!buffer->initialize(size))
		{
			ENGI_LOG_WARN("Failed to create constant buffer {}", name);
			delete buffer;
			return nullptr;
		}
		return buffer;
	}

	Sampler* Renderer::createSampler(const std::string& name, const gfx::GpuSamplerDesc& desc) noexcept
	{
		Sampler* sampler = new Sampler(name, m_device.get());
		if (!sampler->init(desc))
		{
			ENGI_LOG_WARN("Failed to create a sampler {}", name);
			delete sampler;
			return nullptr;
		}
		return sampler;
	}

	DynamicBuffer* Renderer::createDynamicBuffer(const std::string& name, const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept
	{
		DynamicBuffer* buffer = new DynamicBuffer(name, m_device.get());
		if (!buffer->init(data, numVertices, vertexSize))
		{
			ENGI_LOG_WARN("Failed to create dynamic buffer {}", name);
			delete buffer;
			return nullptr;
		}
		return buffer;
	}

	IndexBuffer* Renderer::createIndexBuffer(const std::string& name, const uint32_t* indices, uint32_t numIndices) noexcept
	{
		ENGI_ASSERT(indices && numIndices > 0);

		IndexBuffer* buffer = new IndexBuffer(name, m_device.get());
		if (!buffer->initialize(indices, numIndices))
		{
			ENGI_LOG_WARN("Failed to create index buffer {}", name);
			delete buffer;
			return nullptr;
		}
		return buffer;
	}

	void Renderer::updateDepthStencilCopyResource() noexcept
	{
		if (m_depthStencilTexture)
		{
			ENGI_ASSERT(m_depthStencilResource && "They both should be non-null here. Something went wrong during the creation stage");

			// REMARK: We do not count the width of particular mip slice. Texture2D::getWidth returns the width of mip = 0
			// REMARK: Default-constructed Texture2DCopyState is suitable specifically for copying the depth texture, as it copies the whole resource
			// in a most optimized way.
			m_depthStencilResource->copyFrom(m_depthStencilTexture, Texture2DCopyState());
		}
	}

	void Renderer::updateNormalGBufferCopyResource() noexcept
	{
		if (m_gbufferNormals)
		{
			ENGI_ASSERT(m_gbufferNormalsResource && "They both should be non-null here. Something went wrong during the creation stage");

			// REMARK: We do not count the width of particular mip slice. Texture2D::getWidth returns the width of mip = 0
			// REMARK: Default-constructed Texture2DCopyState is suitable specifically for copying the depth texture, as it copies the whole resource
			// in a most optimized way.
			m_gbufferNormalsResource->copyFrom(m_gbufferNormals, Texture2DCopyState());
		}
	}

	static void SetResourceViewToNull(gfx::IGpuDevice* device, uint32_t slot, uint32_t shaderTypes)
	{
		ENGI_ASSERT(device);
		device->setSRV(nullptr, slot, shaderTypes);
	}

	void Renderer::bindShaderResource2D(std::nullptr_t resourceView, uint32_t slot, uint32_t shaderTypes) noexcept
	{
		SetResourceViewToNull(m_device.get(), slot, shaderTypes);
	}

	void Renderer::bindShaderResource2D(Texture2D* resourceView, uint32_t slot, uint32_t shaderTypes) noexcept
	{
		if (!resourceView)
		{
			SetResourceViewToNull(m_device.get(), slot, shaderTypes);
			return;
		}

		resourceView->bindShaderView(slot, shaderTypes);
	}

	void Renderer::bindShaderResource2D(TextureCube* resourceView, uint32_t slot, uint32_t shaderTypes) noexcept
	{
		if (!resourceView)
		{
			SetResourceViewToNull(m_device.get(), slot, shaderTypes);
			return;
		}

		resourceView->bindArrayShaderView(slot, shaderTypes);
	}

	void Renderer::bindShaderResourceCube(TextureCube* resourceView, uint32_t slot, uint32_t shaderTypes) noexcept
	{
		if (!resourceView)
		{
			SetResourceViewToNull(m_device.get(), slot, shaderTypes);
			return;
		}

		resourceView->bindCubeShaderView(slot, shaderTypes);
	}

	void Renderer::drawAABB(const math::AABB& aabb) noexcept
	{
		if (m_debugAABBRenderData.DrawCount >= m_debugAABBRenderData.countPerDrawcall)
		{
			this->endDebugBatch();
			this->beginDebugBatch();
		}

		uint32_t count = m_debugAABBRenderData.DrawCount++;
		m_debugAABBRenderData.DrawData[count] = aabb;
	}

	void Renderer::beginDebugBatch()
	{
		m_debugAABBRenderData.DrawCount = 0;
	}

	void Renderer::endDebugBatch()
	{
		if (m_debugAABBRenderData.DrawCount == 0)
			return;

		GpuRenderTargetDesc ldrBackbuffer;
		ldrBackbuffer.rtv = m_swapchain->getBackbuffer();
		ldrBackbuffer.useClearcolor = false;

		GpuRenderPassDesc passDesc;
		passDesc.depthStencilBuffer = m_depthStencilTexture->getHandle();
		passDesc.clearDepth = 0.0f;
		passDesc.clearStencil = 0;
		passDesc.rtvs[0] = ldrBackbuffer;

		uint32_t w = m_swapchain->getDesc().width;
		uint32_t h = m_swapchain->getDesc().height;
		m_device->setViewport(0, 0, w, h);
		m_device->beginRenderPass(passDesc);
		{
			AABBRenderData& data = m_debugAABBRenderData;
			math::Vec3* mapping = reinterpret_cast<math::Vec3*>(data.vbo->map());
			uint32_t count = data.DrawCount;
			for (uint32_t i = 0; i < count; ++i)
			{
				const math::AABB& aabb = data.DrawData[i];
				mapping[i * 8 + 0] = aabb.min;
				mapping[i * 8 + 1] = math::Vec3(aabb.min.x, aabb.max.y, aabb.min.z);
				mapping[i * 8 + 2] = math::Vec3(aabb.max.x, aabb.max.y, aabb.min.z);
				mapping[i * 8 + 3] = math::Vec3(aabb.max.x, aabb.min.y, aabb.min.z);
				mapping[i * 8 + 4] = aabb.max;
				mapping[i * 8 + 5] = math::Vec3(aabb.max.x, aabb.min.y, aabb.max.z);
				mapping[i * 8 + 6] = math::Vec3(aabb.min.x, aabb.min.y, aabb.max.z);
				mapping[i * 8 + 7] = math::Vec3(aabb.min.x, aabb.max.y, aabb.max.z);
			}
			data.vbo->unmap();

			data.vbo->bind(0, 0);
			data.ibo->bind(0);
			data.material->bind();
			m_device->drawIndexed(48 * data.DrawCount, 0, 0);
		}
		m_device->endRenderPass();
	}

	void Renderer::draw(uint32_t numVertices, uint32_t vertexOffset) noexcept
	{
		m_device->draw(numVertices, vertexOffset);
	}

	void Renderer::drawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t vertexOffset, uint32_t instanceOffset) noexcept
	{
		m_device->drawInstanced(numVerticesPerInstance, numInstances, vertexOffset, instanceOffset);
	}

	void Renderer::drawInstancedIndexed(uint32_t numIndicesPerInstance, uint32_t numInstances, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset) noexcept
	{
		m_device->drawIndexedInstanced(numIndicesPerInstance, numInstances, indexOffset, vertexOffset, instanceOffset);
	}

	void Renderer::drawInstancedIndexedIndirect(Buffer* buffer, uint32_t numOffset) noexcept
	{
		ENGI_ASSERT(buffer);
		m_device->drawIndexedInstancedIndirect(buffer->getHandle(), numOffset * buffer->getBufferStride());
	}

	void Renderer::setComputeUav(Buffer* buffer, uint32_t slot) noexcept
	{
		m_device->setComputeUAV(buffer ? buffer->getUav() : nullptr, slot);
	}

	void Renderer::dispatch(uint32_t threadGroupsX, uint32_t threadGroupsY, uint32_t threadGroupsZ) noexcept
	{
		m_device->dispatch(threadGroupsX, threadGroupsY, threadGroupsZ);
	}

	void Renderer::dispatchIndirect(Buffer* buffer, uint32_t numOffset) noexcept
	{
		ENGI_ASSERT(buffer);
		uint32_t stride = buffer->getBufferStride();
		uint32_t byteOffset = numOffset * stride;
		m_device->dispatchIndirect(buffer->getHandle(), byteOffset);
	}

	bool Renderer::createDevice()
	{
		m_device = gfx::createDevice();
		ENGI_ASSERT(m_device && "Failed to create logical device");
		return true;
	}

	bool Renderer::createSwapchain(void* handle, uint32_t width, uint32_t height)
	{
		GpuSwapchainDesc swapchainDesc;
		swapchainDesc.windowHandle = handle;
		swapchainDesc.width = width;
		swapchainDesc.height = height;
		swapchainDesc.backbuffers = 2;
		swapchainDesc.format = GpuFormat::RGBA8UN;

		GpuResourceAllocator* gpuAllocator = m_device->getResourceAllocator();
		m_swapchain = makeGpuHandle(m_device->createSwapchain("Renderer::Swapchain", swapchainDesc), gpuAllocator);
		if (!m_swapchain)
		{
			ENGI_LOG_ERROR("Failed to create swapchain");
			return false;
		}

		return true;
	}

	bool Renderer::initHDRRenderTarget()
	{
		IGpuTexture* backbuffer = m_swapchain->getBackbuffer();
		const GpuTextureDesc& surfaceDesc = backbuffer->getDesc();
		GpuResourceAllocator* gpuAllocator = m_device->getResourceAllocator();

		// Params of Texture2D::Init are defaulted to SHADER_RESOURCE | RENDER_TARGET and ACCESS_UNUSED
		m_hdrTexture = m_textureLibrary->createTexture2D("Renderer_HDRTargetTexture");
		if (!m_hdrTexture->init(surfaceDesc.width, surfaceDesc.height, 1, 1, GpuFormat::RGBA16F))
		{
			ENGI_LOG_ERROR("Failed to initialize HDR render target");
			return false;
		}

		if (!m_hdrTexture->initShaderView())
		{
			ENGI_LOG_ERROR("Failed to initialize HDR render target's resource view (SRV)");
			return false;
		}

		return true;
	}

	bool Renderer::initDepthStencil()
	{
		IGpuTexture* backbuffer = m_swapchain->getBackbuffer();
		const GpuTextureDesc& surfaceDesc = backbuffer->getDesc();
		GpuResourceAllocator* gpuAllocator = m_device->getResourceAllocator();

		m_depthStencilTexture = m_textureLibrary->createTexture2D("Renderer_DepthStencilTexture");
		if (!m_depthStencilTexture->init(surfaceDesc.width, surfaceDesc.height, 1, 1, GpuFormat::DEPTH_STENCIL_FORMAT, GpuBinding::DEPTH_STENCIL, GpuUsage::DEFAULT))
		{
			ENGI_LOG_ERROR("Failed to init depth stencil buffer");
			return false;
		}

		m_depthStencilResource = m_textureLibrary->createTexture2D("Renderer_DepthStencilTexture_CopyAsResource");
		if (!m_depthStencilResource->init(surfaceDesc.width, surfaceDesc.height, 1, 1, GpuFormat::R24G8T, GpuBinding::SHADER_RESOURCE, GpuUsage::DEFAULT))
		{
			ENGI_LOG_ERROR("Failed to init depth stencil buffer copy");
			return false;
		}

		if (!m_depthStencilResource->initShaderView(GpuFormat::R24UNX8T))
		{
			ENGI_LOG_ERROR("Failed to init depth stencil buffer copies' resource view");
			return false;
		}

		return true;
	}

	bool Renderer::initRegistries()
	{
		gfx::IGpuDevice* device = m_device.get();
		m_shaderLibrary.reset(new ShaderLibrary(device));

		m_materialRegistry = makeUnique<MaterialRegistry>(new MaterialRegistry(device, m_shaderLibrary.get()));
		if (!m_materialRegistry->init())
			return false;

		m_modelRegistry = makeUnique<ModelRegistry>(new ModelRegistry(device));
		if (!m_modelRegistry->init())
			return false;

		m_textureLibrary.reset(new TextureLibrary(device));
		m_textureLoader.reset(new TextureLoader(device, m_textureLibrary.get()));
		m_modelLoader.reset(new ModelLoader(m_textureLoader.get(), m_modelRegistry.get(), m_materialRegistry.get()));

		return true;
	}

	bool Renderer::initSamplers()
	{
		using namespace gfx;

		{
			GpuSamplerDesc desc;
			desc.filtering = GpuSamplerFiltering::FILTER_MIN_MAG_MIP_POINT;
			desc.addressing = GpuSamplerAddressing::ADDRESS_WRAP;
			m_samplerNearest = makeUnique<Sampler>(this->createSampler("Renderer::NearestSampler", desc));
			if (!m_samplerNearest)
			{
				ENGI_LOG_WARN("Failed to create nearest sampler");
				return false;
			}
		}
		
		{
			GpuSamplerDesc desc;
			desc.filtering = GpuSamplerFiltering::FILTER_MIN_MAG_LINEAR_MIP_POINT;
			desc.addressing = GpuSamplerAddressing::ADDRESS_WRAP;
			m_samplerLinear = makeUnique<Sampler>(this->createSampler("Renderer::LinearSampler", desc));
			if (!m_samplerLinear)
			{
				ENGI_LOG_WARN("Failed to create linear sampler");
				return false;
			}
		}

		{
			GpuSamplerDesc desc;
			desc.filtering = GpuSamplerFiltering::FILTER_MIN_MAG_MIP_LINEAR;
			desc.addressing = GpuSamplerAddressing::ADDRESS_WRAP;
			m_samplerTrilinear = makeUnique<Sampler>(this->createSampler("Renderer::TrilinearSampler", desc));
			if (!m_samplerTrilinear)
			{
				ENGI_LOG_WARN("Failed to create trilinear sampler");
				return false;
			}
		}

		{
			GpuSamplerDesc desc;
			desc.filtering = GpuSamplerFiltering::FILTER_ANISOTROPIC;
			desc.addressing = GpuSamplerAddressing::ADDRESS_WRAP;
			m_samplerAnisotropic = makeUnique<Sampler>(this->createSampler("Renderer::AnisotropicSampler", desc));
			if (!m_samplerAnisotropic)
			{
				ENGI_LOG_WARN("Failed to create anisotropic sampler");
				return false;
			}
		}

		{
			GpuSamplerDesc desc;
			desc.filtering = GpuSamplerFiltering::FILTER_SHADOWS;
			desc.comparator = GpuComparisonFunc::COMP_GREATER;
			desc.addressing = GpuSamplerAddressing::ADDRESS_BORDER;
			desc.border[0] = 0.0f;
			desc.border[1] = 0.0f;
			desc.border[2] = 0.0f;
			desc.border[3] = 1.0f;
			m_samplerShadow = makeUnique<Sampler>(this->createSampler("Renderer::ShadowSampler", desc));
			if (!m_samplerShadow)
			{
				ENGI_LOG_WARN("Failed to create shadow sampler");
				return false;
			}
		}

		{
			GpuSamplerDesc desc;
			desc.filtering = GpuSamplerFiltering::FILTER_MIN_MAG_LINEAR_MIP_POINT;
			desc.addressing = GpuSamplerAddressing::ADDRESS_CLAMP;
			m_samplerLinearClamp = makeUnique<Sampler>(this->createSampler("Renderer::LinearSampler_Clamp", desc));
			if (!m_samplerLinearClamp)
			{
				ENGI_LOG_WARN("Failed to create linear sampler");
				return false;
			}
		}

		return true;
	}

	bool Renderer::initPostProcessor()
	{
		m_postProcessor.reset(new PostProcessor(this));
		if (!m_postProcessor->init())
		{
			ENGI_LOG_WARN("Failed to init post processor");
			return false;
		}

		return true;
	}

	bool Renderer::initGBuffers() noexcept
	{
		using namespace gfx;

		m_gbufferAlbedo = m_textureLibrary->createTexture2D("Renderer_GBuffer_Albedo");
		if (!m_gbufferAlbedo->init(m_width, m_height, 1, 1, GpuFormat::RGBA8UN) || !m_gbufferAlbedo->initShaderView())
		{
			ENGI_LOG_WARN("Failed to init albedo-alpha gbuffer");
			return false;
		}

		m_gbufferNormals = m_textureLibrary->createTexture2D("Renderer_GBuffer_Normals");
		if (!m_gbufferNormals->init(m_width, m_height, 1, 1, GpuFormat::RGBA16SN) || !m_gbufferNormals->initShaderView())
		{
			ENGI_LOG_WARN("Failed to init normal gbuffer");
			return false;
		}

		m_gbufferNormalsResource = m_textureLibrary->createTexture2D("Renderer_GBuffer_Normals_CopyAsResource");
		if (!m_gbufferNormalsResource->init(m_width, m_height, 1, 1, GpuFormat::RGBA16SN) || !m_gbufferNormalsResource->initShaderView())
		{
			ENGI_LOG_WARN("Failed to init normal gbuffer copy resource");
			return false;
		}

		m_gbufferRoughnessMetalness = m_textureLibrary->createTexture2D("Renderer_GBuffer_RoughnessMetalness");
		if (!m_gbufferRoughnessMetalness->init(m_width, m_height, 1, 1, GpuFormat::RG8UN) || !m_gbufferRoughnessMetalness->initShaderView())
		{
			ENGI_LOG_WARN("Failed to init roughness-metalness gbuffer");
			return false;
		}

		m_gbufferEmission = m_textureLibrary->createTexture2D("Renderer_GBuffer_Emission");
		if (!m_gbufferEmission->init(m_width, m_height, 1, 1, GpuFormat::RGBA16F) || !m_gbufferEmission->initShaderView())
		{
			ENGI_LOG_WARN("Failed to init emission gbuffer");
			return false;
		}

		m_gbufferObjectID = m_textureLibrary->createTexture2D("Renderer_GBuffer_ObjectIDs");
		if (!m_gbufferObjectID->init(m_width, m_height, 1, 1, GpuFormat::R32U) || !m_gbufferObjectID->initShaderView())
		{
			ENGI_LOG_WARN("Failed to init Object ID gbuffer");
			return false;
		}

		return true;
	}

	void Renderer::initRenderData()
	{
		delete[] m_debugAABBRenderData.DrawData;
		m_debugAABBRenderData.DrawData = new math::AABB[m_debugAABBRenderData.countPerDrawcall];
		m_debugAABBRenderData.DrawCount = 0;

		ShaderProgram* shader = m_shaderLibrary->createProgram("DebugDrawLine.hlsl", false, false);
		shader->setAttributeLayout({ { gfx::GpuInputAttributeDesc("WORLD_POS", 0, gfx::GpuFormat::RGB32F, 0, true, 0) } });

		m_debugAABBRenderData.material = m_materialRegistry->registerMaterial("ENGI_DebugAABBMaterial");
		ENGI_ASSERT(m_debugAABBRenderData.material);
		m_debugAABBRenderData.material->setPrimitiveType(gfx::GpuPrimitive::LINELIST);
		m_debugAABBRenderData.material->getDepthStencilState().depthEnabled = false;
		m_debugAABBRenderData.material->setShader(shader);
		if (!m_debugAABBRenderData.material->init())
		{
			ENGI_LOG_ERROR("Failed to init debug draw line material");
			return;
		}

		uint32_t size = 48 * m_debugAABBRenderData.countPerDrawcall;
		uint32_t* indices = new uint32_t[size];
		uint32_t indexLayout[48] =
		{
			0, 1, 1, 2, 2, 3, 3, 0, // -Z
			6, 7, 7, 1, 1, 0, 0, 6, // -X
			5, 4, 4, 7, 7, 6, 6, 5, // +Z
			3, 2, 2, 4, 4, 5, 5, 3, // +X
			1, 7, 7, 4, 4, 2, 2, 1, // +Y
			0, 6, 6, 5, 5, 3, 3, 0, // -Y
		};
		ENGI_ASSERT(sizeof(indexLayout) / sizeof(uint32_t) == 48);

		for (uint32_t i = 0; i < m_debugAABBRenderData.countPerDrawcall; ++i)
		{
			for (uint32_t j = 0; j < 48; ++j)
			{
				indices[i * 48 + j] = i * 8 + indexLayout[j];
			}
		}

		m_debugAABBRenderData.ibo = makeUnique<IndexBuffer>(new IndexBuffer("DebugAABB_IBO", m_device.get()));
		m_debugAABBRenderData.ibo->initialize(indices, size);
		delete[] indices;

		m_debugAABBRenderData.vbo = makeUnique<DynamicBuffer>(new DynamicBuffer("DebugAABB_VBO", m_device.get()));
		m_debugAABBRenderData.vbo->init(nullptr, 8 * m_debugAABBRenderData.countPerDrawcall, sizeof(math::Vec3));
	}

}; // engi namespace