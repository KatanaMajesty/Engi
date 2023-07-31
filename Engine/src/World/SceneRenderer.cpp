#include "SceneRenderer.h"

#include "Utility/Timer.h"
#include "Core/CommonDefinitions.h"
#include "Core/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/ConstantBuffer.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/PostProcessor.h"
#include "Renderer/ReflectionCapture.h"
#include "Renderer/Skybox.h"
#include "Renderer/TextureLibrary.h"

namespace engi
{

	struct alignas(16) SceneConstant
	{
		float time;
		float timestep;
		float dissolutionTime;
		uint32_t numDirLights;
		uint32_t numSpotLights;
		uint32_t numPointLights;
		uint32_t useIBL;
		uint32_t useShadowmapping;
	};

	struct alignas(16) ViewConstant
	{
		ViewConstant() = default;

		ViewConstant(const Camera& camera)
			: proj(camera.getProj())
			, projInv(camera.getProjInv())
		{
			views[0] = camera.getView();
			viewsInv[0] = camera.getViewInv();
		}

		math::Mat4x4 views[6];
		math::Mat4x4 viewsInv[6];
		math::Mat4x4 proj;
		math::Mat4x4 projInv;
	};

	bool SceneRenderer::init(Renderer* renderer) noexcept
	{
		ENGI_ASSERT(renderer);
		m_renderer = renderer;

		m_particleSystem = makeUnique<ParticleSystem>(new ParticleSystem(renderer));
		if (!m_particleSystem->init())
		{
			ENGI_LOG_ERROR("Failed to initialize particle system");
			return false;
		}

		m_lightManager = makeUnique<LightManager>(new LightManager(renderer));
		if (!m_lightManager->init())
		{
			ENGI_LOG_ERROR("Failed to initialize Light Manager!");
			return false;
		}

		m_instanceTable = makeUnique<InstanceTable>(new InstanceTable());
		m_cameraInstanceID = m_instanceTable->addInstanceData(InstanceData());

		m_meshManager = makeUnique<MeshManager>(new MeshManager(m_renderer, this->getInstanceTable()));
		if (!m_meshManager->init())
		{
			ENGI_LOG_ERROR("Failed to initialize Mesh Manager!");
			return false;
		}

		m_decalManager = makeUnique<DecalManager>(new DecalManager(m_renderer));
		if (!m_decalManager->init(this->getInstanceTable()))
		{
			ENGI_LOG_ERROR("Failed to init Decal Manager");
			return false;
		}

		m_instanceRegistry = makeUnique<ModelInstanceRegistry>(new ModelInstanceRegistry(this));

		// TODO: Remove this somewhere?
		MaterialRegistry* materialRegistry = m_renderer->getMaterialRegistry();
		ShaderLibrary* shaderLibrary = m_renderer->getShaderLibrary();
		ShaderProgram* shader = nullptr;
		shader = shaderLibrary->createProgram("NormalVis.hlsl", true, false);
		shader->setAttributeLayout(MeshManager::getInputAttributes(0, 1));
		m_normalVisMaterial = materialRegistry->registerMaterial("ENGI_NormalVis");
		m_normalVisMaterial->setShader(shader);
		m_normalVisMaterial->init();

		shader = shaderLibrary->createProgram("Depthmap_Texture2D.hlsl", false, false, false);
		shader->setAttributeLayout(MeshManager::getInputAttributes(0, 1));
		m_depthmap2DMaterial = materialRegistry->registerMaterial("ENGI_Depthmap2D");
		m_depthmap2DMaterial->setShader(shader);
		m_depthmap2DMaterial->getRasterizerState().depthBias = -4;
		m_depthmap2DMaterial->getRasterizerState().depthBiasClamp = 0.0f;
		m_depthmap2DMaterial->getRasterizerState().depthSlopeBiasScale = -4.0f;
		m_depthmap2DMaterial->init();

		shader = shaderLibrary->createProgram("Depthmap_TextureCube.hlsl", true, false, false);
		shader->setAttributeLayout(MeshManager::getInputAttributes(0, 1));
		m_depthmapCubeMaterial = materialRegistry->registerMaterial("ENGI_DepthmapCube");
		m_depthmapCubeMaterial->setShader(shader);
		m_depthmapCubeMaterial->getRasterizerState().depthBias = -64;
		m_depthmapCubeMaterial->getRasterizerState().depthBiasClamp = 0.0f;
		m_depthmapCubeMaterial->getRasterizerState().depthSlopeBiasScale = -4.0f;
		m_depthmapCubeMaterial->init();

		shader = shaderLibrary->createProgram("PBR.hlsl", false, false);
		shader->setAttributeLayout({});
		m_deferredPBRMaterial = materialRegistry->registerMaterial("ENGI_DeferredPBR");
		m_deferredPBRMaterial->setShader(shader);
		m_deferredPBRMaterial->getDepthStencilState().depthEnabled = false;
		m_deferredPBRMaterial->getDepthStencilState().stencilEnabled = true;
		m_deferredPBRMaterial->getDepthStencilState().stencilRef = 1;

		// because fullscreen pass is backface only
		m_deferredPBRMaterial->getDepthStencilState().backFace.comparator = gfx::GpuComparisonFunc::COMP_EQUAL;
		m_deferredPBRMaterial->getRasterizerState().culling = gfx::CULLING_NONE;
		m_deferredPBRMaterial->init();

		shader = shaderLibrary->createProgram("Emissive.hlsl", false, false);
		shader->setAttributeLayout({});
		m_deferredEmissiveMaterial = materialRegistry->registerMaterial("ENGI_DeferredEmissive");
		m_deferredEmissiveMaterial->setShader(shader);
		m_deferredEmissiveMaterial->getDepthStencilState().depthEnabled = false;
		m_deferredEmissiveMaterial->getDepthStencilState().stencilEnabled = true;
		m_deferredEmissiveMaterial->getDepthStencilState().stencilRef = 2;
		
		// because fullscreen pass is backface only
		m_deferredEmissiveMaterial->getDepthStencilState().backFace.comparator = gfx::GpuComparisonFunc::COMP_EQUAL;
		m_deferredEmissiveMaterial->getRasterizerState().culling = gfx::CULLING_NONE;
		m_deferredEmissiveMaterial->init();

		// Thats how it works with this API...
		shader = shaderLibrary->createProgram("Incineration_Particles.hlsl", false, false);
		shader->initCompute(shader->getPath());
		m_incinerationParticlesMaterial = materialRegistry->registerMaterial("ENGI_IncinerationParticles");
		m_incinerationParticlesMaterial->setShader(shader);
		m_incinerationParticlesMaterial->getDepthStencilState().depthEnabled = true;
		m_incinerationParticlesMaterial->getDepthStencilState().depthWritable = false;
		m_incinerationParticlesMaterial->getRasterizerState().culling = gfx::CULLING_NONE;
		m_incinerationParticlesMaterial->init();

		m_sceneConstantBuffer = makeUnique<ConstantBuffer>(m_renderer->createConstantBuffer("PerFrame_CB", sizeof(SceneConstant)));
		m_viewConstantBuffer = makeUnique<ConstantBuffer>(m_renderer->createConstantBuffer("PerView_CB", sizeof(ViewConstant)));
		if (!m_sceneConstantBuffer || !m_viewConstantBuffer)
		{
			ENGI_LOG_ERROR("Failed to create constant buffers");
			return false;
		}

		// We do not initialize it over here. It will be initialized when the texture is set
		m_reflectionCapture = makeUnique<ReflectionCapture>(new ReflectionCapture(m_renderer));

		ShaderProgram* skyboxShader = m_renderer->getShaderLibrary()->createProgram("Skybox.hlsl", false, false);
		m_skybox = makeUnique<Skybox>(m_renderer->createSkybox("Scene_Skybox", skyboxShader, m_reflectionCapture.get()));
		if (!m_skybox)
		{
			ENGI_LOG_ERROR("Failed to create a scene's skybox");
			return false;
		}

		this->initSamplers();

		return true;
	}

	/*void SceneRenderer::resize(uint32_t width, uint32_t height) noexcept
	{
		ENGI_LOG_INFO("Initializing render passes on resize with ({}, {})", width, height);
		this->initRenderPasses(width, height);
	}*/

	void SceneRenderer::update(float timestep) noexcept
	{
		ENGI_ASSERT(m_dissolutionTexture && "Set a dissolution texture, please");
		ENGI_ASSERT(m_cameraManager && "We do not update without a camera");
		
		m_frameTimestep = timestep;

		if (m_instanceTable)
		{
			for (InstanceData& data : m_instanceTable->getAllInstanceData())
			{
				data.time += timestep;
				data.timestep = timestep;
			}

			// Mesh manager is using a bit deprecated API with buffer update stuff. Will be changed sometime in future
			if (m_meshManager)
				m_meshManager->requestBufferUpdate();

			if (m_decalManager)
				m_decalManager->update();

			if (m_cameraManager && m_cameraInstanceID != uint32_t(-1))
			{
				const Camera& camera = m_cameraManager->getCamera();
				InstanceData& cameraData = m_instanceTable->getInstanceData(m_cameraInstanceID);
				cameraData.modelToWorld = camera.getView();
				cameraData.worldToModel = camera.getViewInv();
			}
		}

		if (m_particleSystem)
			m_particleSystem->update(timestep, m_cameraManager->getCamera().getPosition());

		if (m_lightManager)
		{
			// Set depth anchors and apply them to the GPU before rendering all the stuff
			for (DirectionalLight& light : m_lightManager->getAllDirLights())
				light.setDepthAnchor(m_cameraManager->getCamera());

			m_lightManager->update();
		}
	}

	void SceneRenderer::render(bool debugPass) noexcept
	{
		ENGI_ASSERT(m_cameraManager && "We do not render without a camera");

		this->setSamplers();
		this->setSceneConstant();

		LightManager* lightManager = this->getLightManager();
		if (lightManager->isShadowmappingEnabled())
		{
			this->renderShadowsForDirectionalLight();
			this->renderShadowsForSpotLights();
			this->renderShadowsForPointLights();
		}

		// Main shading stage
		this->deferredPass();
		this->forwardPass();

		// Resolve stage
		this->performResolve();

		// Debug normal visualization stage
		if (debugPass)
		{
			this->performDebugStage();
		}
	}

	void SceneRenderer::setDissolutionTexture(Texture2D* texture) noexcept
	{
		ENGI_ASSERT(texture);
		ENGI_LOG_TRACE("Dissolution texture is set to {}", texture->getName());
		m_dissolutionTexture = texture;
	}

	bool SceneRenderer::initSamplers() noexcept
	{
		m_activeSampler = m_renderer->getLinearSampler();

		ENGI_LOG_INFO("SceneRenderer's samplers initialized successfully");
		return true;
	}

	void SceneRenderer::setSamplers() noexcept
	{
		if (!m_activeSampler)
		{
			Sampler* linearSampler = m_renderer->getLinearSampler();
			ENGI_ASSERT(linearSampler && "Cannot bind");
			linearSampler->bind(0);
		}
		else m_activeSampler->bind(0);

		m_renderer->getLinearSampler()->bind(1);
		m_renderer->getTrilinearSampler()->bind(2);
		m_renderer->getAnisotropicSampler()->bind(3);
		m_renderer->getShadowSampler()->bind(4);
		m_renderer->getNearestSampler()->bind(5); 
		m_renderer->getLinearSamplerWithClamping()->bind(6);
	}

	void SceneRenderer::setSceneConstant() noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(m_sceneConstantBuffer);

		LightManager* lightManager = this->getLightManager();
		SceneConstant sceneConstant;
		sceneConstant.time = Timer::getTime();
		sceneConstant.timestep = m_frameTimestep;
		sceneConstant.dissolutionTime = this->getDissolutionTime();
		sceneConstant.numDirLights = lightManager->getNumDirLights();
		sceneConstant.numSpotLights = lightManager->getNumSpotLights();
		sceneConstant.numPointLights = lightManager->getNumPointLights();
		sceneConstant.useIBL = m_reflectionCapture->shouldUseIBL();
		sceneConstant.useShadowmapping = lightManager->isShadowmappingEnabled();

		m_sceneConstantBuffer->upload(0, &sceneConstant, sizeof(SceneConstant));
		m_sceneConstantBuffer->bind(0, VERTEX_SHADER | HULL_SHADER | DOMAIN_SHADER | GEOMETRY_SHADER | PIXEL_SHADER | COMPUTE_SHADER);
	}

	void SceneRenderer::setViewConstant(const ViewConstant& viewConstant) noexcept
	{
		using namespace gfx;

		ENGI_ASSERT(m_viewConstantBuffer);
		m_viewConstantBuffer->upload(0, &viewConstant, sizeof(ViewConstant));
		m_viewConstantBuffer->bind(3, VERTEX_SHADER | GEOMETRY_SHADER | PIXEL_SHADER | COMPUTE_SHADER);
	}

	void SceneRenderer::renderShadowsForDirectionalLight() noexcept
	{
		LightManager* lm = this->getLightManager();
		Texture2D* dirDepthmapArray = lm->getDirectionalDepthmapArray();
		for (DirectionalLight& light : lm->getAllDirLights())
		{
			ViewConstant view;
			view.views[0] = light.getView();
			view.viewsInv[0] = light.getView().inverse();
			view.proj = light.getProjection();
			this->setViewConstant(view);

			uint32_t width = dirDepthmapArray->getWidth();
			uint32_t height = dirDepthmapArray->getHeight();

			RenderPass renderPassDepth2D;
			renderPassDepth2D.setDepthStencilBuffer2D(dirDepthmapArray, 0, light.getDepthmapArrayslice(), true, true);
			renderPassDepth2D.setViewport(width, height);

			m_renderer->beginRenderPass(renderPassDepth2D);
			{
				m_meshManager->renderUsingMaterial(m_depthmap2DMaterial);
			}
			m_renderer->endRenderPass();
		}
	}

	void SceneRenderer::renderShadowsForPointLights() noexcept
	{
		LightManager* lm = this->getLightManager();
		TextureCube* pointDepthmapArray = lm->getPointDepthmapArray();
		for (PointLight& light : lm->getAllPointLights())
		{
			ViewConstant view;
			view.proj = light.getProjection();
			view.projInv = view.proj.inverse();
			for (uint32_t i = 0; i < 6; ++i)
			{
				// we dont need inverse matrices here
				view.views[i] = light.getView(i);
			}
			this->setViewConstant(view);
			
			uint32_t width = pointDepthmapArray->getWidth();
			uint32_t height = pointDepthmapArray->getHeight();

			RenderPass renderPassDepthCube;
			renderPassDepthCube.setDepthStencilBufferCube(pointDepthmapArray, 0, light.getDepthmapArrayslice(), true, true);
			renderPassDepthCube.setViewport(width, height);

			m_renderer->beginRenderPass(renderPassDepthCube);
			{
				m_meshManager->renderUsingMaterial(m_depthmapCubeMaterial);
			}
			m_renderer->endRenderPass();
		}
	}

	void SceneRenderer::renderShadowsForSpotLights() noexcept
	{
		LightManager* lm = this->getLightManager();
		Texture2D* spotDepthmapArray = lm->getSpotDepthmapArray();
		for (SpotLight& light : lm->getAllSpotLights())
		{
			ViewConstant view;
			view.views[0] = light.getView();
			view.proj = light.getProjection();
			this->setViewConstant(view);

			uint32_t width = spotDepthmapArray->getWidth();
			uint32_t height = spotDepthmapArray->getHeight();

			RenderPass renderPassDepth2D;
			renderPassDepth2D.setDepthStencilBuffer2D(spotDepthmapArray, 0, light.getDepthmapArrayslice(), true, true);
			renderPassDepth2D.setViewport(width, height);

			m_renderer->beginRenderPass(renderPassDepth2D);
			{
				m_meshManager->renderUsingMaterial(m_depthmap2DMaterial);
			}
			m_renderer->endRenderPass();
		}
	}

	void SceneRenderer::deferredPass() noexcept
	{
		using namespace gfx;

		const Camera& camera = m_cameraManager->getCamera();

		// Set camera stuff and draw
		this->setViewConstant(ViewConstant(camera));

		Texture2D* depthStencilBuffer = m_renderer->getDepthStencilTexture();
		uint32_t width = m_renderer->getBackbufferWidth();
		uint32_t height = m_renderer->getBackbufferHeight();

		ParticleSystem* particleSystem = this->getParticleSystem();

		// Render into GBuffers
		RenderPass opaqueDeferredRenderPass;
		opaqueDeferredRenderPass.setDepthStencilBuffer2D(depthStencilBuffer, 0, 0, true, true);
		opaqueDeferredRenderPass.setRenderTarget(0, m_renderer->getGBufferAlbedo()->getHandle(), 0, 0, true);
		opaqueDeferredRenderPass.setRenderTarget(1, m_renderer->getGBufferNormals()->getHandle(), 0, 0, true);
		opaqueDeferredRenderPass.setRenderTarget(2, m_renderer->getGBufferEmission()->getHandle(), 0, 0, true);
		opaqueDeferredRenderPass.setRenderTarget(3, m_renderer->getGBufferRoughnessMetalness()->getHandle(), 0, 0, true);
		opaqueDeferredRenderPass.setRenderTarget(4, m_renderer->getGBufferObjectID()->getHandle(), 0, 0, true);
		opaqueDeferredRenderPass.setUnorderedAccessView(0, particleSystem->getIncinerationRangeBuffer());
		opaqueDeferredRenderPass.setUnorderedAccessView(1, particleSystem->getIncinerationStructuredBuffer());
		opaqueDeferredRenderPass.setViewport(width, height);
		m_renderer->beginRenderPass(opaqueDeferredRenderPass);
		{
			// Todo: We might want a specific order of rendering materials
			m_renderer->bindShaderResource2D(this->getDissolutionTexture(), 20, PIXEL_SHADER);
			m_meshManager->render();
		}
		m_renderer->endRenderPass();
		m_renderer->updateDepthStencilCopyResource();
		m_renderer->updateNormalGBufferCopyResource();

		RenderPass decalDeferredRenderPass;
		decalDeferredRenderPass.setDepthStencilBuffer2D(depthStencilBuffer, 0, 0, false, false);
		decalDeferredRenderPass.setRenderTarget(0, m_renderer->getGBufferAlbedo()->getHandle(), 0, 0, false);
		decalDeferredRenderPass.setRenderTarget(1, m_renderer->getGBufferNormals()->getHandle(), 0, 0, false);
		decalDeferredRenderPass.setRenderTarget(2, m_renderer->getGBufferEmission()->getHandle(), 0, 0, false);
		decalDeferredRenderPass.setRenderTarget(3, m_renderer->getGBufferRoughnessMetalness()->getHandle(), 0, 0, false);
		decalDeferredRenderPass.setViewport(width, height);
		m_renderer->beginRenderPass(decalDeferredRenderPass);
		{
			Texture2D* depthResource = m_renderer->getDepthStencilCopyResource();
			Texture2D* normalsResource = m_renderer->getGBufferNormalsCopyResource();
			m_decalManager->render(m_renderer->getGBufferObjectID(), normalsResource, depthResource);
		}
		m_renderer->endRenderPass();

		m_renderer->beginRenderPass(RenderPass());
		{
			Texture2D* depthResource = m_renderer->getDepthStencilCopyResource();
			Texture2D* normalsResource = m_renderer->getGBufferNormalsCopyResource();
			m_renderer->bindShaderResource2D(depthResource, 26, COMPUTE_SHADER);
			m_renderer->bindShaderResource2D(normalsResource, 22, COMPUTE_SHADER);
			m_renderer->bindShaderResource2D(m_renderer->getGBufferObjectID(), 25, COMPUTE_SHADER);

			SharedHandle<Material> material = m_renderer->getMaterialRegistry()->getMaterial(MATERIAL_BRDF_PBR_INCINERATION);
			material->bind();
			m_renderer->setComputeUav(particleSystem->getIncinerationRangeBuffer(), 0);
			m_renderer->setComputeUav(particleSystem->getIncinerationStructuredBuffer(), 1);
			m_renderer->dispatchIndirect(particleSystem->getIncinerationRangeBuffer(), 8);

			m_incinerationParticlesMaterial->bind();
			m_renderer->setComputeUav(particleSystem->getIncinerationRangeBuffer(), 0);
			m_renderer->setComputeUav(particleSystem->getIncinerationStructuredBuffer(), 1);
			m_renderer->dispatch(1, 1, 1);

			m_renderer->bindShaderResource2D(nullptr, 25, COMPUTE_SHADER);
			m_renderer->setComputeUav(nullptr, 0);
			m_renderer->setComputeUav(nullptr, 1);
		}
		m_renderer->endRenderPass();

		Texture2D* HDRTexture = m_renderer->getHDRTargetTexture();
		LightManager* lightManager = this->getLightManager();

		// Resolve deferred GBuffers to HDR Buffer
		RenderPass gbufferToHDRRenderPass;
		gbufferToHDRRenderPass.setDepthStencilBuffer2D(depthStencilBuffer, 0, 0, false, false);
		gbufferToHDRRenderPass.setRenderTarget(0, HDRTexture->getHandle(), 0, 0, true);
		gbufferToHDRRenderPass.setViewport(HDRTexture->getWidth(), HDRTexture->getHeight());
		m_renderer->beginRenderPass(gbufferToHDRRenderPass);
		{
			ENGI_ASSERT(m_skybox); // We do not want to render without skybox for now
			m_skybox->bindIBL();
			
			// Do full-screen pass
			lightManager->getDirLightBuffer()->bind(8, VERTEX_SHADER | PIXEL_SHADER);
			lightManager->getSpotLightBuffer()->bind(9, VERTEX_SHADER | PIXEL_SHADER);
			lightManager->getPointLightBuffer()->bind(10, VERTEX_SHADER | PIXEL_SHADER);
			m_renderer->bindShaderResource2D(lightManager->getDirectionalDepthmapArray(), 12, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(lightManager->getSpotDepthmapArray(), 13, PIXEL_SHADER);
			m_renderer->bindShaderResourceCube(lightManager->getPointDepthmapArray(), 14, PIXEL_SHADER);
			
			m_renderer->bindShaderResource2D(m_renderer->getGBufferAlbedo(), 21, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(m_renderer->getGBufferNormals(), 22, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(m_renderer->getGBufferEmission(), 23, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(m_renderer->getGBufferRoughnessMetalness(), 24, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(m_renderer->getGBufferObjectID(), 25, PIXEL_SHADER);
			
			Texture2D* depthBufferResource = m_renderer->getDepthStencilCopyResource();
			m_renderer->bindShaderResource2D(depthBufferResource, 26, PIXEL_SHADER);
			
			// TODO: Rewrite cookie management
			// Now only first spotlight's cookie is used, others are ignored
			auto& spotlights = m_lightManager->getAllSpotLights();
			if (spotlights.size() > 0 && spotlights.begin()->getCookie())
			{
				spotlights.begin()->getCookie()->bindShaderView(11, PIXEL_SHADER);
			}
			
			m_deferredEmissiveMaterial->bind();
			m_renderer->draw(3, 0);
			
			m_deferredPBRMaterial->bind();
			m_renderer->draw(3, 0);

			m_skybox->render();

			particleSystem->getIncinerationRangeBuffer()->bind(0, VERTEX_SHADER);
			particleSystem->getIncinerationStructuredBuffer()->bind(1, VERTEX_SHADER);
			m_incinerationParticlesMaterial->bind();
			m_renderer->drawInstancedIndexedIndirect(particleSystem->getIncinerationRangeBuffer(), 3);
			m_renderer->bindShaderResource2D(nullptr, 0, VERTEX_SHADER);
			m_renderer->bindShaderResource2D(nullptr, 1, VERTEX_SHADER);

			m_renderer->bindShaderResource2D(nullptr, 21, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(nullptr, 22, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(nullptr, 23, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(nullptr, 24, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(nullptr, 25, PIXEL_SHADER);
			
			m_renderer->bindShaderResource2D(nullptr, 12, PIXEL_SHADER);
			m_renderer->bindShaderResource2D(nullptr, 13, PIXEL_SHADER);
			m_renderer->bindShaderResourceCube(nullptr, 14, PIXEL_SHADER);
		}
		m_renderer->endRenderPass();

	}

	void SceneRenderer::forwardPass() noexcept
	{
		Texture2D* HDRTexture = m_renderer->getHDRTargetTexture();
		Texture2D* depthStencilBuffer = m_renderer->getDepthStencilTexture();

		RenderPass generalForwardRenderPass;
		generalForwardRenderPass.setDepthStencilBuffer2D(depthStencilBuffer, 0, 0, false, false);
		generalForwardRenderPass.setRenderTarget(0, HDRTexture->getHandle(), 0, 0, false);
		generalForwardRenderPass.setViewport(HDRTexture->getWidth(), HDRTexture->getHeight());
		m_renderer->beginRenderPass(generalForwardRenderPass);
		{
			Texture2D* depthResource = m_renderer->getDepthStencilCopyResource();
			m_particleSystem->render(depthResource);
		}
		m_renderer->endRenderPass();
	}

	void SceneRenderer::performResolve() noexcept
	{
		const CameraPhysicalProperties& cameraPhysProps = m_cameraManager->getPhysicalCameraProperties();
		PostProcessor* postProcessor = m_renderer->getPostProcessor();
		postProcessor->setTonemapper(TonemapperType::ACES);

		// Perform either resolve-FXAA or standard resolve
		if (this->isFXAAEnabled() && postProcessor->isFXAAInitted())
		{
			// Reinitialize a texture, if resize occured. Needed lazily because we do not want PostProcessor to have its own event propagation
			// Thus, we use this if-branch instead of onResize callback
			uint32_t width = m_renderer->getBackbufferWidth();
			uint32_t height = m_renderer->getBackbufferHeight();
			if (!m_fxaaAttachment || m_fxaaAttachment->getWidth() != width || m_fxaaAttachment->getHeight() != height)
			{
				TextureLibrary* textureLibrary = m_renderer->getTextureLibrary();
				m_fxaaAttachment = textureLibrary->createTexture2D("PostProcessor_FXAA_Attachment");
				if (!m_fxaaAttachment->init(width, height, 1, 1, gfx::GpuFormat::RGBA8UN) || !m_fxaaAttachment->initShaderView())
				{
					ENGI_LOG_CRITICAL("Failed to create FXAA attachment. FXAA will be disabled");
					m_useFXAA = false;
					return;
				}
			}
			postProcessor->resolve(m_fxaaAttachment->getHandle(), cameraPhysProps.exposureValue, cameraPhysProps.gammaCorrection);
			postProcessor->applyFXAA(m_fxaaAttachment, m_fxaaSpecification);
		}
		else postProcessor->resolve(m_renderer->getBackbuffer(), cameraPhysProps.exposureValue, cameraPhysProps.gammaCorrection);
	}

	void SceneRenderer::performDebugStage() noexcept
	{
		const Camera& camera = m_cameraManager->getCamera();

		// Set camera stuff and draw
		ViewConstant view;
		view.views[0] = camera.getView();
		view.viewsInv[0] = camera.getViewInv();
		view.proj = camera.getProj();
		view.projInv = camera.getProjInv();
		this->setViewConstant(view);

		Texture2D* depthStencilBuffer = m_renderer->getDepthStencilTexture();
		gfx::IGpuTexture* backbuffer = m_renderer->getBackbuffer();
		uint32_t width = m_renderer->getBackbufferWidth();
		uint32_t height = m_renderer->getBackbufferHeight();

		RenderPass debugForwardRenderPass;
		debugForwardRenderPass.setDepthStencilBuffer2D(depthStencilBuffer, 0, 0, false, false);
		debugForwardRenderPass.setRenderTarget(0, backbuffer, 0, 0, false);
		debugForwardRenderPass.setViewport(width, height);
		m_renderer->beginRenderPass(debugForwardRenderPass);
		{
			m_meshManager->renderUsingMaterial(m_normalVisMaterial);
		}
		m_renderer->endRenderPass();
	}

}; // engi namespace