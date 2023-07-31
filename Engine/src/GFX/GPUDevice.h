#pragma once

#include "GFX/Definitions.h"

namespace engi::gfx
{

	class IGpuResource;
	class GpuResourceAllocator;
	class IGpuBuffer;
	class IGpuInputLayout;
	class IGpuPipelineState;
	class IGpuSampler;
	class IGpuShader;
	class IGpuSwapchain;
	class IGpuTexture;
	class IGpuTextureLoader;
	class IGpuDescriptor;

	// Currently we are only using immediate rendering, thus all states and commands are executed through the IGpuDevice
	// In future we would prefer to use own IGpuCommandlist implementation
	class IGpuDevice
	{
	public:
		IGpuDevice() = default;
		IGpuDevice(const IGpuDevice&) = default;
		IGpuDevice& operator=(const IGpuDevice&) = default;
		virtual ~IGpuDevice() = default;

		virtual IGpuSwapchain* createSwapchain(const std::string& name, const GpuSwapchainDesc& desc) = 0;
		virtual IGpuBuffer* createBuffer(const std::string& name, const GpuBufferDesc& desc, const void* initialData) = 0;
		virtual IGpuShader* createShader(const std::string& name, const GpuShaderDesc& desc, void* bytecode) = 0;
		virtual IGpuTexture* createTexture(const std::string& name, const GpuTextureDesc& desc, const void* initialData) = 0;
		virtual IGpuPipelineState* createPipelineState(const std::string& name, const GpuPipelineStateDesc& desc) = 0;
		virtual IGpuSampler* createSampler(const std::string& name, const GpuSamplerDesc& desc) = 0;
		virtual IGpuInputLayout* createInputLayout(const std::string& name, const GpuInputAttributeDesc* attributes, uint32_t numAttributes, const GpuShaderBuffer& shaderBuffer) = 0;
		virtual IGpuDescriptor* createSRV(const std::string& name, const GpuSrvDesc& desc, IGpuResource* resource) = 0;
		virtual IGpuDescriptor* createUAV(const std::string& name, const GpuUavDesc& desc, IGpuResource* resource) = 0;
		virtual void destroy(IGpuResource*& resource) = 0;

		virtual void beginRenderPass(const GpuRenderPassDesc& desc) = 0;
		virtual void endRenderPass() = 0;
		virtual void draw(uint32_t numVertices, uint32_t vertexOffset) = 0;
		virtual void drawIndexed(uint32_t numIndices, uint32_t indexOffset, uint32_t vertexOffset) = 0;
		virtual void drawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t vertexOffset, uint32_t instanceOffset) = 0;
		virtual void drawIndexedInstanced(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset) = 0;
		virtual void drawIndexedInstancedIndirect(IGpuBuffer* buffer, uint32_t byteOffset) = 0;
		virtual void dispatch(uint32_t threadGroupsX, uint32_t threadGroupsY, uint32_t threadGroupsZ) = 0;
		virtual void dispatchIndirect(IGpuBuffer* buffer, uint32_t byteOffset) = 0;

		virtual void setPipelineState(IGpuPipelineState* state) = 0;
		virtual void setInputLayout(IGpuInputLayout* inputLayout) = 0;
		virtual void setVertexBuffer(IGpuBuffer* buffer, uint32_t slot, uint32_t stride, uint32_t offset) = 0;
		virtual void setIndexBuffer(IGpuBuffer* buffer, uint32_t offset) = 0;
		virtual void setConstantBuffer(IGpuBuffer* buffer, uint32_t slot, uint32_t shaderTypes) = 0;
		virtual void setSRV(const IGpuDescriptor* descriptor, uint32_t slot, uint32_t shaderTypes) = 0;
		virtual void setComputeUAV(const IGpuDescriptor* descriptor, uint32_t slot) = 0;
		virtual void setSampler(const IGpuSampler* sampler, uint32_t slot, uint32_t shaderTypes) = 0;
		virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void copyTexture(IGpuTexture* dst, uint32_t dstMipslice, uint32_t dstArrayslice, uint32_t dstX, uint32_t dstY, uint32_t dstZ, 
			IGpuTexture* src, uint32_t srcMipslice, uint32_t srcArrayslice, uint32_t srcX, uint32_t srcY, uint32_t srcZ, 
			uint32_t width, uint32_t height, uint32_t depth) = 0;
		virtual void copyBuffer(IGpuBuffer* src, IGpuBuffer* dst, uint32_t dstOffset) = 0;
		virtual void updateBuffer(IGpuBuffer* buffer, uint32_t bufferOffset, const void* data, uint32_t byteSize) = 0;
		virtual void mapBuffer(IGpuBuffer* buffer, void** mapping) = 0;
		virtual void unmapBuffer(IGpuBuffer* buffer) = 0;

		// TODO: Make non-virtual
		virtual GpuResourceAllocator* getResourceAllocator() = 0;
	}; // IGpuDevice class

}; // engi::gfx namespace