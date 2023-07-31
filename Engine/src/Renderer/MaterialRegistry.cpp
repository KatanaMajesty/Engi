#include "Renderer/MaterialRegistry.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/MeshManager.h"

namespace engi
{

	static std::string g_defaultMatNames[] = // TODO: Maybe move/change this?
	{
		"D_Hologram_mat",
		"D_Normals_mat",
		"D_Albedo_mat",
		"D_Emissive_mat",
		"D_BRDF_PBR_mat",
		"D_BRDF_PBR_NOCULL_mat",
		"D_BRDF_PBR_Dissolution_mat",
		"D_BRDF_PBR_Incineration_mat",
	};

	MaterialRegistry::MaterialRegistry(gfx::IGpuDevice* device, ShaderLibrary* shaderLibrary)
		: m_device(device)
		, m_shaderLibrary(shaderLibrary)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
		ENGI_ASSERT(shaderLibrary);
	}

	MaterialRegistry::~MaterialRegistry()
	{
		ENGI_LOG_INFO("Destroying material registry");
		bool successfullyDestroyed = true;
		for (auto& pair : m_materials)
		{
			const SharedHandle<Material>& material = pair.second;
			if (material.use_count() > 1)
			{
				successfullyDestroyed = false;
				ENGI_LOG_WARN("Failed to destroy {} material as it is used somewhere else. Skipping", material->getName());
			}
		}

		if (successfullyDestroyed)
			ENGI_LOG_INFO("Successfully destroyed material registry");
		else ENGI_LOG_ERROR("Failed to successfully destroy material registry");
	}

	static ShaderProgram* CreateShaderWithMeshAttributes(ShaderLibrary* library, const std::string& shadername, bool geometry, bool tesselation)
	{
		ENGI_ASSERT(library);
		ShaderProgram* shader = library->createProgram(shadername, geometry, tesselation, true);
		
		ENGI_ASSERT(shader);
		shader->setAttributeLayout(MeshManager::getInputAttributes(0, 1));

		return shader;
	}

	bool MaterialRegistry::init() noexcept
	{
		SharedHandle<Material> mat = registerMaterial(g_defaultMatNames[MATERIAL_HOLOGRAM]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 2;
		mat->getDepthStencilState().frontFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->setShader(CreateShaderWithMeshAttributes(m_shaderLibrary, "GBuffer_Hologram.hlsl", true, true));
		mat->setPrimitiveType(gfx::GpuPrimitive::CONTROLPOINT_PATCHLIST3);
		mat->init();

		mat = registerMaterial(g_defaultMatNames[MATERIAL_NORMALS]);
		mat->setShader(CreateShaderWithMeshAttributes(m_shaderLibrary, "NormalColor.hlsl", false, false));
		mat->setPrimitiveType(gfx::GpuPrimitive::CONTROLPOINT_PATCHLIST3);
		mat->init();

		mat = registerMaterial(g_defaultMatNames[MATERIAL_ALBEDO_COLOR]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 2;
		mat->setShader(CreateShaderWithMeshAttributes(m_shaderLibrary, "AlbedoColor.hlsl", false, false));
		mat->init();

		mat = registerMaterial(g_defaultMatNames[MATERIAL_EMISSIVE]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 2;
		mat->getDepthStencilState().frontFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->getDepthStencilState().backFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->setShader(CreateShaderWithMeshAttributes(m_shaderLibrary, "GBuffer_Emissive.hlsl", false, false));
		mat->init();

		ShaderProgram* PBRShader = CreateShaderWithMeshAttributes(m_shaderLibrary, "GBuffer_Opaque.hlsl", false, false);
		mat = registerMaterial(g_defaultMatNames[MATERIAL_BRDF_PBR]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 1;
		mat->getDepthStencilState().frontFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->setShader(PBRShader);
		mat->init();

		mat = registerMaterial(g_defaultMatNames[MATERIAL_BRDF_PBR_NO_CULLING]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 1;
		mat->getDepthStencilState().frontFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->getDepthStencilState().backFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->getRasterizerState().culling = gfx::CULLING_NONE;
		mat->setShader(PBRShader);
		mat->init();
		
		mat = registerMaterial(g_defaultMatNames[MATERIAL_BRDF_PBR_DISSOLUTION]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 1;
		mat->getDepthStencilState().frontFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->getRasterizerState().culling = gfx::CULLING_NONE;
		mat->setShader(CreateShaderWithMeshAttributes(m_shaderLibrary, "GBuffer_Dissolution.hlsl", false, false));
		mat->init();

		ShaderProgram* incinerationProgram = CreateShaderWithMeshAttributes(m_shaderLibrary, "GBuffer_Incineration.hlsl", false, false);
		if (!incinerationProgram->initCompute(incinerationProgram->getPath()))
		{
			ENGI_LOG_ERROR("Failed to init compute shader for incineration effect");
			return false;
		}

		mat = registerMaterial(g_defaultMatNames[MATERIAL_BRDF_PBR_INCINERATION]);
		mat->getDepthStencilState().stencilEnabled = true;
		mat->getDepthStencilState().stencilRef = 1;
		mat->getDepthStencilState().frontFace.passOperation = gfx::GpuStencilOperation::STENCIL_REPLACE;
		mat->getRasterizerState().culling = gfx::CULLING_NONE;
		mat->setShader(incinerationProgram);
		mat->init();

		return true;
	}

	SharedHandle<Material> MaterialRegistry::registerMaterial(const std::string& name) noexcept
	{
		SharedHandle<Material> mat = getMaterial(name);
		if (mat)
			return mat;

		mat = makeShared<Material>(new Material(m_device, name));
		m_materials[name] = mat;
		return mat;
	}

	SharedHandle<Material> MaterialRegistry::getMaterial(const std::string& name) noexcept
	{
		auto it = m_materials.find(name);
		return (it == m_materials.end()) ? nullptr : it->second;
	}

	SharedHandle<Material> MaterialRegistry::getMaterial(MaterialType type) noexcept
	{
		return getMaterial(g_defaultMatNames[type]);
	}

};