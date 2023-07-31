#pragma once

#include "Math/Math.h"
#include "Utility/Memory.h"
#include "GFX/Definitions.h"
#include "GFX/GPUResourceAllocator.h"

// TODO: WIP DEBUG RENDERER
#include "IndexBuffer.h"
#include "Buffer.h"
#include "ConstantBuffer.h"
#include "DynamicBuffer.h"
#include "Sampler.h"
#include "RenderPass.h"

namespace engi
{

	namespace gfx
	{
		class IGpuDevice;
		class IImGuiContext;
		class IGpuSwapchain;
		class IGpuTexture;
	}

	class Material;
	class MaterialRegistry;
	class ModelRegistry;
	class ModelLoader;
	class ShaderLibrary;
	class Texture2D;
	class TextureCube;
	class TextureLibrary;
	class TextureLoader;
	class PostProcessor;
	class Skybox;
	class ShaderProgram;
	class ReflectionCapture;

	class Renderer
	{
	public:
		Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		~Renderer();

		bool init(void* handle, uint32_t width, uint32_t height) noexcept;
		void resize(uint32_t width, uint32_t height) noexcept;

		void beginFrame();
		void endFrame();
		
		// UI Frame is assumed to be a different render pass with different states due to how ImGui determines its render target
		void beginUIFrame();
		void endUIFrame();

		void beginRenderPass(const RenderPass& renderPass) noexcept;
		void endRenderPass() noexcept;

		gfx::IGpuTexture* getBackbuffer() noexcept;
		inline constexpr uint32_t getBackbufferWidth() const noexcept { return m_width; }
		inline constexpr uint32_t getBackbufferHeight() const noexcept { return m_height; }

		inline MaterialRegistry* getMaterialRegistry() noexcept { return m_materialRegistry.get(); }
		inline ModelRegistry* getModelRegistry() noexcept { return m_modelRegistry.get(); }
		inline ShaderLibrary* getShaderLibrary() noexcept { return m_shaderLibrary.get(); }
		inline TextureLibrary* getTextureLibrary() noexcept { return m_textureLibrary.get(); }
		
		inline ModelLoader* getModelLoader() noexcept { return m_modelLoader.get(); }
		inline TextureLoader* getTextureLoader() noexcept { return m_textureLoader.get(); }
		
		// TODO: WIP moving resource creation responsibility to renderer
		Skybox* createSkybox(const std::string& name, ShaderProgram* shader, ReflectionCapture* reflectionCapture = nullptr) noexcept;
		IndexBuffer* createIndexBuffer(const std::string& name, const uint32_t* indices, uint32_t numIndices) noexcept;
		Sampler* createSampler(const std::string& name, const gfx::GpuSamplerDesc& desc) noexcept;
		
		Buffer* createResourceBuffer(const std::string& name) noexcept;
		ConstantBuffer* createConstantBuffer(const std::string& name, uint32_t size) noexcept;
		DynamicBuffer* createDynamicBuffer(const std::string& name, const void* data, uint32_t numVertices, uint32_t vertexSize) noexcept;

		inline PostProcessor* getPostProcessor() noexcept { return m_postProcessor.get(); }
		inline bool isImGuiInitialized() const noexcept { return m_imguiContext != nullptr; }

		inline Sampler* getNearestSampler() noexcept { return m_samplerNearest.get(); }
		inline Sampler* getLinearSampler() noexcept { return m_samplerLinear.get(); }
		inline Sampler* getLinearSamplerWithClamping() noexcept { return m_samplerLinearClamp.get(); }
		inline Sampler* getTrilinearSampler() noexcept { return m_samplerTrilinear.get(); }
		inline Sampler* getAnisotropicSampler() noexcept { return m_samplerAnisotropic.get(); }
		inline Sampler* getShadowSampler() noexcept { return m_samplerShadow.get(); }

		void updateDepthStencilCopyResource() noexcept;
		inline constexpr Texture2D* getDepthStencilTexture() noexcept { return m_depthStencilTexture; }
		inline constexpr Texture2D* getDepthStencilCopyResource() noexcept { return m_depthStencilResource; }
		inline constexpr Texture2D* getHDRTargetTexture() noexcept { return m_hdrTexture; }

		void updateNormalGBufferCopyResource() noexcept;
		inline constexpr Texture2D* getGBufferAlbedo() noexcept { return m_gbufferAlbedo; }
		inline constexpr Texture2D* getGBufferNormals() noexcept { return m_gbufferNormals; }
		inline constexpr Texture2D* getGBufferNormalsCopyResource() noexcept { return m_gbufferNormalsResource; }
		inline constexpr Texture2D* getGBufferEmission() noexcept { return m_gbufferEmission; }
		inline constexpr Texture2D* getGBufferRoughnessMetalness() noexcept { return m_gbufferRoughnessMetalness; }
		inline constexpr Texture2D* getGBufferObjectID() noexcept { return m_gbufferObjectID; }

		void bindShaderResource2D(std::nullptr_t resourceView, uint32_t slot, uint32_t shaderTypes) noexcept;
		void bindShaderResource2D(Texture2D* resourceView, uint32_t slot, uint32_t shaderTypes) noexcept;
		void bindShaderResource2D(TextureCube* resourceView, uint32_t slot, uint32_t shaderTypes) noexcept;
		void bindShaderResourceCube(TextureCube* resourceView, uint32_t slot, uint32_t shaderTypes) noexcept;

		// TODO: Temp, will be used to test the new renderer features before refactoring it
		void drawAABB(const math::AABB& aabb) noexcept;
		void beginDebugBatch();
		void endDebugBatch();

		void draw(uint32_t numVertices, uint32_t vertexOffset) noexcept;
		void drawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t vertexOffset, uint32_t instanceOffset) noexcept;
		void drawInstancedIndexed(uint32_t numIndicesPerVertex, uint32_t numInstances, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset) noexcept;
		void drawInstancedIndexedIndirect(Buffer* buffer, uint32_t numOffset) noexcept;

		void setComputeUav(Buffer* buffer, uint32_t slot) noexcept;
		void dispatch(uint32_t threadGroupsX, uint32_t threadGroupsY, uint32_t threadGroupsZ) noexcept;
		void dispatchIndirect(Buffer* buffer, uint32_t numOffset) noexcept;

	private:
		// WIP: New renderer render passes
		bool createDevice();
		bool createSwapchain(void* handle, uint32_t width, uint32_t height);
		bool initHDRRenderTarget();
		bool initDepthStencil();
		bool initRegistries();
		bool initSamplers();
		bool initPostProcessor();

		UniqueHandle<gfx::IGpuDevice> m_device = nullptr;
		UniqueHandle<gfx::IImGuiContext> m_imguiContext = nullptr;
		UniqueHandle<TextureLibrary> m_textureLibrary = nullptr;
		UniqueHandle<TextureLoader> m_textureLoader = nullptr;
		UniqueHandle<ModelRegistry> m_modelRegistry = nullptr;
		UniqueHandle<MaterialRegistry> m_materialRegistry = nullptr;
		UniqueHandle<ModelLoader> m_modelLoader = nullptr;
		UniqueHandle<ShaderLibrary> m_shaderLibrary = nullptr;
		uint32_t m_width;
		uint32_t m_height;

		gfx::GpuHandle<gfx::IGpuSwapchain> m_swapchain = nullptr;
		Texture2D* m_depthStencilTexture = nullptr;
		Texture2D* m_depthStencilResource = nullptr;
		Texture2D* m_hdrTexture = nullptr;

		// TODO: Wip Deferred rendering stuff
		bool initGBuffers() noexcept;

		Texture2D* m_gbufferAlbedo = nullptr;
		Texture2D* m_gbufferNormals = nullptr;
		Texture2D* m_gbufferNormalsResource = nullptr;
		Texture2D* m_gbufferRoughnessMetalness = nullptr;
		Texture2D* m_gbufferEmission = nullptr;
		Texture2D* m_gbufferObjectID = nullptr;

		UniqueHandle<PostProcessor> m_postProcessor = nullptr;

		UniqueHandle<Sampler> m_samplerNearest;
		UniqueHandle<Sampler> m_samplerLinear;
		UniqueHandle<Sampler> m_samplerTrilinear;
		UniqueHandle<Sampler> m_samplerAnisotropic;
		UniqueHandle<Sampler> m_samplerShadow;
		UniqueHandle<Sampler> m_samplerLinearClamp;

		// TODO: DEBUG MATERIAL! WILL BE REMOVED SOON PROB
		void initRenderData();
		struct AABBRenderData
		{
			math::AABB* DrawData = nullptr;
			uint32_t DrawCount = 0;

			SharedHandle<Material> material = nullptr;
			UniqueHandle<DynamicBuffer> vbo = nullptr;
			UniqueHandle<IndexBuffer> ibo = nullptr;
			uint32_t countPerDrawcall = 16;
		} m_debugAABBRenderData;

	}; // class Renderer

}; // engi namespace