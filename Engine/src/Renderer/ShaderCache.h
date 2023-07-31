#pragma once

#include <unordered_map>
#include "GFX/Definitions.h"
#include "GFX/GPUShader.h"
#include "GFX/GPUResourceAllocator.h"

namespace engi
{

	namespace detail
	{

		struct ShaderDescEqual
		{
			bool operator()(const gfx::GpuShaderDesc& lhs, const gfx::GpuShaderDesc& rhs) const
			{
				return (lhs.filepath == rhs.filepath)
					&& (lhs.entrypoint == rhs.entrypoint)
					&& (lhs.type == rhs.type);
			}
		};

		struct ShaderDescHash
		{
			size_t operator()(const gfx::GpuShaderDesc& desc) const noexcept
			{
				size_t hashtype = std::hash<uint32_t>{}(desc.type);
				size_t hashpath = std::hash<std::string>{}(desc.filepath);
				size_t hashentry = std::hash<std::string>{}(desc.entrypoint);

				// from boost::hash_combine
				size_t hash = hashtype + 0x9e3779b9 + (hashpath << 6) + (hashentry >> 2);
				return hash;
			}
		};

	}; // detail namespace

	class gfx::IGpuDevice;

	// TODO: As we use one shader file per many shader types it is more efficient to use a shader code cache,
	// so that we don't read a shader file multiple times by using D3DReadFromFile

	class ShaderCache
	{
	public:
		using Hasher = detail::ShaderDescHash;
		using EqComparator = detail::ShaderDescEqual;
		using ShaderCacheContainer = std::unordered_map<gfx::GpuShaderDesc, gfx::GpuHandle<gfx::IGpuShader>, Hasher, EqComparator>;

		ShaderCache(gfx::IGpuDevice* device);
		ShaderCache(const ShaderCache&) = delete;
		ShaderCache& operator=(const ShaderCache&) = delete;
		~ShaderCache() = default;

		void addShader(gfx::IGpuShader* shader) noexcept;
		void removeShader(gfx::IGpuShader* shader) noexcept;
		gfx::IGpuShader* getShader(const gfx::GpuShaderDesc& desc) noexcept;

	private:
		gfx::IGpuDevice* m_device;
		ShaderCacheContainer m_compiledShaders;
	}; // ShaderManager class

}; // engi namespace