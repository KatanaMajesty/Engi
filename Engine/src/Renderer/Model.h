#pragma once

#include <vector>
#include "Utility/Memory.h"
#include "Renderer/StaticMeshTriangleOctree.h"
#include "Renderer/StaticMesh.h"

namespace engi
{

	namespace gfx { class IGpuDevice; }
	class ImmutableBuffer;
	class IndexBuffer;

	struct MeshRange
	{
		uint32_t vboOffset;
		uint32_t iboOffset;
		uint32_t numVertices;
		uint32_t numIndices;
	};

	struct StaticMeshEntry
	{
		StaticMeshEntry(StaticMeshEntry&&) = default;
		StaticMeshEntry& operator=(StaticMeshEntry&&) = default;
		StaticMeshEntry(StaticMesh&& mesh, const MeshRange& range);
		~StaticMeshEntry() = default;

		bool initialize() noexcept;
		inline constexpr bool isValid() const noexcept { return !mesh.isEmpty() && mesh.isVertexFull() && mesh.isTriangleFull(); }

		StaticMesh mesh;
		StaticMeshTriangleOctree bvh;
		MeshRange range;
	};

	// Model is a container of immutable meshes with gpu objects needed to render it
	class Model
	{
	public:

		Model(const std::string& name, gfx::IGpuDevice* device, uint32_t staticMeshEntries);
		Model(Model&&) = default;
		Model& operator=(Model&&) = default;
		~Model();

		bool addStaticMeshEntry(StaticMeshEntry&& meshEntry) noexcept;
		bool initialize() noexcept;
		inline const auto& getStaticMeshEntries() const noexcept { return m_staticMeshes; };
		inline auto& getStaticMeshEntries() noexcept { return m_staticMeshes; }; // TODO: Somehow get rid of this
		inline ImmutableBuffer* getVBO() noexcept { return m_vbo.get(); }
		inline IndexBuffer* getIBO() noexcept { return m_ibo.get(); }
		inline uint32_t getNumStaticMeshes() const noexcept { return static_cast<uint32_t>(m_staticMeshes.size()); }
		inline constexpr const std::string& getName() const noexcept { return m_name; }
		inline constexpr const std::string& getPath() const noexcept { return m_filepath; }
		inline constexpr void setPath(const std::string& filepath) noexcept { m_filepath = filepath; }
		inline constexpr bool hasPath() const noexcept { return !m_filepath.empty(); }

	private:
		std::string m_name;
		std::string m_filepath;
		gfx::IGpuDevice* m_device;
		UniqueHandle<ImmutableBuffer> m_vbo = nullptr;
		UniqueHandle<IndexBuffer> m_ibo = nullptr;
		uint32_t m_staticEntriesCapacity;
		std::vector<StaticMeshEntry> m_staticMeshes;
	};

}; // engi namespace