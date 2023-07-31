#include "Renderer/ShaderProgram.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "GFX/Definitions.h"
#include "GFX/GPUDevice.h"
#include "GFX/GPUInputLayout.h"
#include "Renderer/ShaderCompiler.h"
#include "Renderer/ShaderCache.h"

namespace engi
{

	ShaderProgram::ShaderProgram(const std::string& name, gfx::IGpuDevice* device, ShaderCompiler* compiler, ShaderCache* cache)
		: m_name(name)
		, m_device(device)
		, m_compiler(compiler)
		, m_cache(cache)
	{
		ENGI_ASSERT(compiler);
		ENGI_ASSERT(cache);
	}

	ShaderProgram::~ShaderProgram()
	{
	}

	bool ShaderProgram::init(const std::string& filepath, bool geometryStage, bool tesselationStage, bool pixelStage) noexcept
	{
		gfx::IGpuShader* vs = loadShader(filepath, "vs_main", gfx::GpuShaderType::VERTEX_SHADER);
		if (!vs)
		{
			ENGI_LOG_ERROR("Failed to parse vertex shader of a {} program", getName());
			return false;
		}
		m_vs = vs;

		if (pixelStage)
		{
			gfx::IGpuShader* ps = loadShader(filepath, "ps_main", gfx::GpuShaderType::PIXEL_SHADER);
			if (!ps)
			{
				ENGI_LOG_ERROR("Failed to parse pixel shader of a {} program", getName());
				return false;
			}
			m_ps = ps;
		}

		if (geometryStage)
		{
			gfx::IGpuShader* gs = loadShader(filepath, "gs_main", gfx::GpuShaderType::GEOMETRY_SHADER);
			if (!gs)
			{
				ENGI_LOG_ERROR("Failed to parse geometry shader of a {} program", getName());
				return false;
			}
			m_gs = gs;
		}
		else m_gs = nullptr;

		if (tesselationStage)
		{
			gfx::IGpuShader* ds = loadShader(filepath, "ds_main", gfx::GpuShaderType::DOMAIN_SHADER);
			gfx::IGpuShader* hs = loadShader(filepath, "hs_main", gfx::GpuShaderType::HULL_SHADER);
			if (!hs || !ds)
			{
				ENGI_LOG_ERROR("Failed to parse either hull of domain shader of a {} program", getName());
				return false;
			}
			m_hs = hs;
			m_ds = ds;
		}
		else
		{
			m_hs = nullptr;
			m_ds = nullptr;
		}

		m_filepath = filepath;
		return true;
	}

	bool ShaderProgram::initCompute(const std::string& filepath) noexcept
	{
		gfx::IGpuShader* cs = loadShader(filepath, "cs_main", gfx::GpuShaderType::COMPUTE_SHADER);
		if (!cs)
		{
			ENGI_LOG_WARN("Failed to parse compute shader of a {} program", getName());
			return false;
		}

		m_cs = cs;
		m_filepath = filepath;
		return true;
	}

	void ShaderProgram::setAttributeLayout(std::span<const gfx::GpuInputAttributeDesc> attributes) noexcept
	{
		// TODO: Add check if the shader program is compute program and does not require any attributes
		// if (compute) return
		if (attributes.empty())
			return;

		ENGI_ASSERT(this->getVS() && "The shader program should be initted beforehand");
		m_inputLayout = gfx::makeGpuHandle(
			m_device->createInputLayout(this->getName() + "_InputLayout", attributes.data(), static_cast<uint32_t>(attributes.size()), this->getVS()->getBytecode()),
			m_device->getResourceAllocator());

		if (!m_inputLayout)
		{
			ENGI_LOG_WARN("Failed to set attribute layout");
			return;
		}
	}

	bool ShaderProgram::recompileAll() noexcept
	{
		if (!m_vs && !m_cs)
		{
			ENGI_LOG_WARN("Cannot recompile the shader");
			return false;
		}

		void* vsBytecode = nullptr;
		if (m_vs)
		{
			vsBytecode = getBytecode(m_vs->getDesc());
		}

		if (!vsBytecode)
		{
			ENGI_LOG_ERROR("Failed to recompile the program");
			return false;
		}

		void* psBytecode = nullptr;
		void* gsBytecode = nullptr;
		void* hsBytecode = nullptr;
		void* dsBytecode = nullptr;
		void* csBytecode = getBytecode(m_cs->getDesc());

		if (hasPixelStage())
		{
			psBytecode = getBytecode(m_ps->getDesc());
			if (!psBytecode)
			{
				ENGI_LOG_ERROR("Failed to recompile the program");
				return false;
			}
		}

		if (hasGeometryStage())
		{
			gsBytecode = getBytecode(m_gs->getDesc());
			if (!gsBytecode)
			{
				ENGI_LOG_ERROR("Failed to recompile the program");
				return false;
			}
		}

		if (hasTesselationStage())
		{
			hsBytecode = getBytecode(m_hs->getDesc());
			dsBytecode = getBytecode(m_ds->getDesc());
			if (!hsBytecode || !dsBytecode)
			{
				ENGI_LOG_ERROR("Failed to recompile the program");
				return false;
			}
		}

		if (hasComputeStage())
		{
			csBytecode = getBytecode(m_cs->getDesc());
			if (!csBytecode)
			{
				ENGI_LOG_ERROR("Failed to recompile the program");
				return false;
			}
		}

		recompile(m_vs, vsBytecode);
		recompile(m_ps, psBytecode);
		recompile(m_hs, hsBytecode);
		recompile(m_ds, dsBytecode);
		recompile(m_gs, gsBytecode);
		recompile(m_cs, csBytecode);
		ENGI_LOG_INFO("Successfully recompiled the {} program", getName());
		return true;
	}

	gfx::IGpuShader* ShaderProgram::loadShader(const std::string& filepath, const std::string& entrypoint, gfx::GpuShaderType shaderType) noexcept
	{
		gfx::GpuShaderDesc desc;
		desc.filepath = filepath;
		desc.entrypoint = entrypoint;
		desc.type = shaderType;
		gfx::IGpuShader* shader = m_cache->getShader(desc);
		if (!shader)
		{
			void* bytecode = m_compiler->compileFromHLSLFile(desc);
			if (!bytecode)
				return nullptr;

			shader = m_device->createShader(m_name, desc, bytecode);
			if (!shader)
				return nullptr;

			m_cache->addShader(shader);
		}
		return shader;
	}

	void* ShaderProgram::getBytecode(const gfx::GpuShaderDesc& desc) noexcept
	{
		return m_compiler->compileFromHLSLFile(desc);
	}

	void ShaderProgram::recompile(gfx::IGpuShader* shader, void* bytecode) noexcept
	{
		if (!shader)
			return;
		
		shader->initialize(bytecode);
	}

}; // engi namespace