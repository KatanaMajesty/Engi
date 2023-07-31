#pragma once

#include <string>
#include <vector>
#include <array>
#include "Math/Math.h"
#include "GFX/Definitions.h"

namespace engi
{

	enum StaticMeshFlags
	{
		STATIC_MESH_FLAGS_NONE = 0,
		STATIC_MESH_FLAGS_NORMALS = 1,
		STATIC_MESH_FLAGS_TEX_COORDS = 2,
		STATIC_MESH_FLAGS_TANGENTS = 4,
	};

	struct StaticMeshVertex
	{
		static std::array<gfx::GpuInputAttributeDesc, 5> getInputAttributes(uint32_t inputSlot);

		math::Vec3 position;
		math::Vec3 normal;
		math::Vec2 textureCoords;
		math::Vec3 tangent;
		math::Vec3 bitangent;
	};

	struct StaticMeshTriangle
	{
		using IndexType = uint32_t;

		std::array<IndexType, 3> indices;
	};

	class StaticMesh
	{
	public:
		using VertexContainerType = std::vector<StaticMeshVertex>;
		using TriangleContainerType = std::vector<StaticMeshTriangle>;

		StaticMesh() = default;
		StaticMesh(StaticMesh&&) = default;
		StaticMesh& operator=(StaticMesh&&) = default;
		StaticMesh(uint32_t numVertices, uint32_t numTriangles, const math::AABB& aabb, uint8_t flags, const std::string& name = "Unnamed static mesh");
		~StaticMesh() = default;

		bool addVertex(const StaticMeshVertex& vertex) noexcept;
		bool addTriangle(const StaticMeshTriangle& triangle) noexcept;
		constexpr bool isTwoSided() const noexcept { return m_twoSided; }
		constexpr void setTwoSided(bool val) noexcept { m_twoSided = val; }
		constexpr const std::string& getName() const noexcept { return m_name; }
		constexpr bool isEmpty() const noexcept { return m_numVertices == 0 && m_numTriangles == 0 && m_vertices.empty(); }
		constexpr bool isVertexFull() const noexcept { return m_numVertices == static_cast<uint32_t>(m_vertices.size()); }
		constexpr bool isTriangleFull() const noexcept { return m_numTriangles == static_cast<uint32_t>(m_triangles.size()); }
		constexpr VertexContainerType& getVertices() noexcept { return m_vertices; };
		constexpr TriangleContainerType& getTriangles() noexcept { return m_triangles; }
		constexpr const VertexContainerType& getVertices() const noexcept { return m_vertices; };
		constexpr const TriangleContainerType& getTriangles() const noexcept { return m_triangles; }
		constexpr uint32_t getNumVertices() const noexcept { return m_numVertices; }
		constexpr uint32_t getNumTriangles() const noexcept { return m_numTriangles; }
		constexpr const math::AABB& getAABB() const noexcept { return m_aabb; }
		constexpr bool hasNormals() const noexcept { return (m_flags & STATIC_MESH_FLAGS_NORMALS) != 0; }
		constexpr bool hasTexCoords() const noexcept { return (m_flags & STATIC_MESH_FLAGS_TEX_COORDS) != 0; }
		constexpr bool hasTangents() const noexcept { return (m_flags & STATIC_MESH_FLAGS_TANGENTS) != 0; }
		math::Mat4x4& getMeshToModel() { return m_meshToModel; } // TODO: This is probably temporary
		math::Mat4x4& getModelToMesh() { return m_modelToMesh; } // TODO: This is probably temporary
		const math::Mat4x4& getMeshToModel() const { return m_meshToModel; } // TODO: This is probably temporary
		const math::Mat4x4& getModelToMesh() const { return m_modelToMesh; } // TODO: This is probably temporary

	private:
		std::string m_name;
		uint8_t m_flags = 0;
		uint32_t m_numVertices = 0;
		uint32_t m_numTriangles = 0;
		VertexContainerType m_vertices;
		TriangleContainerType m_triangles;
		math::Mat4x4 m_meshToModel = math::Mat4x4(); // TODO: This is probably temporary
		math::Mat4x4 m_modelToMesh = math::Mat4x4(); // TODO: This is probably temporary
		math::AABB m_aabb;
		bool m_twoSided = false;
	};

	struct StaticMeshIntersection
	{
		float t = math::Numeric::infinity();
		math::Vec3 hitpos;

		inline constexpr void reset() noexcept { t = math::Numeric::infinity(); }
		inline constexpr bool isValid() const noexcept { return math::Numeric::isFinite(t); }
		inline constexpr bool operator<(const StaticMeshIntersection& other) const noexcept { return t < other.t; }
		inline constexpr bool operator>(const StaticMeshIntersection& other) const noexcept { return t > other.t; }
	};

}; // engi namespace