#pragma once

#include "GFX/GPUDevice.h"
#include "GFX/GpuResourceAllocator.h"
#include "GFX/DX11/D3D11_API.h"

namespace engi::gfx
{

	class D3D11Device : public IGpuDevice
	{
	public:
		D3D11Device() = default;
		D3D11Device(const D3D11Device&) = default;
		D3D11Device& operator=(const D3D11Device&) = default;
		virtual ~D3D11Device();

		// Requests
		virtual IGpuSwapchain* createSwapchain(const std::string& name, const GpuSwapchainDesc& desc) override;
		virtual IGpuBuffer* createBuffer(const std::string& name, const GpuBufferDesc& desc, const void* initialData) override;
		virtual IGpuShader* createShader(const std::string& name, const GpuShaderDesc& desc, void* bytecode) override;
		virtual IGpuTexture* createTexture(const std::string& name, const GpuTextureDesc& desc, const void* initialData) override;
		virtual IGpuPipelineState* createPipelineState(const std::string& name, const GpuPipelineStateDesc& desc) override;
		virtual IGpuSampler* createSampler(const std::string& name, const GpuSamplerDesc& desc) override;
		virtual IGpuInputLayout* createInputLayout(const std::string& name, const GpuInputAttributeDesc* attributes, uint32_t numAttributes, const GpuShaderBuffer& shaderBuffer) override;
		virtual IGpuDescriptor* createSRV(const std::string& name, const GpuSrvDesc& desc, IGpuResource* resource) override;
		virtual IGpuDescriptor* createUAV(const std::string& name, const GpuUavDesc& desc, IGpuResource* resource) override;
		virtual void destroy(IGpuResource*& resource) override;

		// Commands
		virtual void beginRenderPass(const GpuRenderPassDesc& desc) override;
		virtual void endRenderPass() override;
		virtual void draw(uint32_t numVertices, uint32_t vertexOffset) override;
		virtual void drawIndexed(uint32_t numIndices, uint32_t indexOffset, uint32_t vertexOffset) override;
		virtual void drawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t vertexOffset, uint32_t instanceOffset) override;
		virtual void drawIndexedInstanced(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset) override;
		virtual void drawIndexedInstancedIndirect(IGpuBuffer* buffer, uint32_t byteOffset) override;
		virtual void dispatch(uint32_t threadGroupsX, uint32_t threadGroupsY, uint32_t threadGroupsZ) override;
		virtual void dispatchIndirect(IGpuBuffer* buffer, uint32_t byteOffset) override;

		virtual void setPipelineState(IGpuPipelineState* state) override;
		virtual void setInputLayout(IGpuInputLayout* inputLayout) override;
		virtual void setVertexBuffer(IGpuBuffer* buffer, uint32_t slot, uint32_t stride, uint32_t offset) override;
		virtual void setIndexBuffer(IGpuBuffer* buffer, uint32_t offset) override;
		virtual void setConstantBuffer(IGpuBuffer* buffer, uint32_t slot, uint32_t shaderTypes) override;
		virtual void setSRV(const IGpuDescriptor* descriptor, uint32_t slot, uint32_t shaderTypes) override;
		virtual void setComputeUAV(const IGpuDescriptor* descriptor, uint32_t slot) override;
		virtual void setSampler(const IGpuSampler* sampler, uint32_t slot, uint32_t shaderTypes) override;
		virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void copyTexture(IGpuTexture* dst, uint32_t dstMipslice, uint32_t dstArrayslice, uint32_t dstX, uint32_t dstY, uint32_t dstZ,
			IGpuTexture* src, uint32_t srcMipslice, uint32_t srcArrayslice, uint32_t srcX, uint32_t srcY, uint32_t srcZ,
			uint32_t width, uint32_t height, uint32_t depth) override;
		virtual void copyBuffer(IGpuBuffer* src, IGpuBuffer* dst, uint32_t dstOffset) override;
		virtual void updateBuffer(IGpuBuffer* buffer, uint32_t bufferOffset, const void* data, uint32_t byteSize) override;
		virtual void mapBuffer(IGpuBuffer* buffer, void** mapping) override;
		virtual void unmapBuffer(IGpuBuffer* buffer) override;

		virtual GpuResourceAllocator* getResourceAllocator() override { return &m_resourceAllocator; }

		bool initialize();
		void createDXGIFactory();
		void createD3D11Device();
		void createD3D11Debug();

		IDXGIFactory5* getDXGIFactory() { return m_dxgiFactory.Get(); }
		ID3D11Device5* getHandle() { return m_d3dDevice.Get(); }
		ID3D11DeviceContext4* getContext() { return m_d3dContext.Get(); }

	private:
		IDXGIAdapter* m_d3dAdapter;
		ComPtr<IDXGIFactory5> m_dxgiFactory;
		ComPtr<ID3D11Device5> m_d3dDevice;
		ComPtr<ID3D11DeviceContext4> m_d3dContext;
		ComPtr<ID3D11Debug> m_d3dDebug;

		GpuResourceAllocator m_resourceAllocator;
		GpuRenderPassDesc m_currentRenderPassDesc;
	}; // D3D11Device class

}; // namespace engi::gfx