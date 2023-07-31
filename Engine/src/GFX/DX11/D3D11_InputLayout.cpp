#include "GFX/DX11/D3D11_InputLayout.h"

#include "Core/CommonDefinitions.h"
#include "GFX/DX11/D3D11_Utility.h"
#include "GFX/DX11/D3D11_Device.h"

namespace engi::gfx
{

	D3D11InputLayout::D3D11InputLayout(const std::string& name, D3D11Device* device, const GpuInputAttributeDesc* attributes, uint32_t numAttributes)
	{
		m_name = name;
		m_device = device;

		ENGI_ASSERT(attributes && numAttributes > 0 && "Invalid attribute data passed to D3D11InputLayout");
		m_attributes = std::vector<GpuInputAttributeDesc>(attributes, attributes + numAttributes);
	}

	void* D3D11InputLayout::getHandle()
	{
		return m_handle.Get();
	}

	bool D3D11InputLayout::initialize(const GpuShaderBuffer& shaderBuffer)
	{
		uint32_t numAttributes = static_cast<uint32_t>(m_attributes.size());
		std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescs;
		elementDescs.resize(numAttributes);
		for (uint32_t i = 0; i < numAttributes; ++i)
		{
			const GpuInputAttributeDesc& desc = m_attributes[i];
			D3D11_INPUT_ELEMENT_DESC& d3dDesc = elementDescs[i];
			d3dDesc.SemanticName = desc.semanticName.c_str();
			d3dDesc.SemanticIndex = desc.semanticIndex;
			d3dDesc.Format = detail::d3d11Format(desc.format);
			d3dDesc.InputSlot = desc.inputSlot;
			d3dDesc.AlignedByteOffset = desc.byteOffset;
			d3dDesc.InputSlotClass = desc.perVertex ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
			d3dDesc.InstanceDataStepRate = desc.perVertex ? 0 : 1;
		}

		D3D11Device* device = (D3D11Device*)m_device;
		HRESULT hr = device->getHandle()->CreateInputLayout(
			elementDescs.data(),
			(UINT)elementDescs.size(),
			shaderBuffer.bytecode, 
			shaderBuffer.size, 
			m_handle.ReleaseAndGetAddressOf());

		// TODO: Maybe somehow incapsulate this code in D3D11_Resource?
#if !defined(_NDEBUG)
		const std::string& name = this->getName();
		if (m_handle)
			m_handle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
#endif

		return SUCCEEDED(hr);
	}

}; // engi::gfx namespace