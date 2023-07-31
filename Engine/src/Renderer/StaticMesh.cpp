#include "Renderer/StaticMesh.h"

namespace engi
{

	std::array<gfx::GpuInputAttributeDesc, 5> StaticMeshVertex::getInputAttributes(uint32_t inputSlot)
	{
		using namespace gfx;
		std::array<GpuInputAttributeDesc, 5> attributes =
		{
			GpuInputAttributeDesc("STATIC_MESH_POSITION", 0, GpuFormat::RGB32F, inputSlot, true, offsetof(StaticMeshVertex, position)),
			GpuInputAttributeDesc("STATIC_MESH_NORMAL", 0, GpuFormat::RGB32F, inputSlot, true, offsetof(StaticMeshVertex, normal)),
			GpuInputAttributeDesc("STATIC_MESH_TEXUV", 0, GpuFormat::RG32F, inputSlot, true, offsetof(StaticMeshVertex, textureCoords)),
			GpuInputAttributeDesc("STATIC_MESH_TANGENT", 0, GpuFormat::RGB32F, inputSlot, true, offsetof(StaticMeshVertex, tangent)),
			GpuInputAttributeDesc("STATIC_MESH_BITANGENT", 0, GpuFormat::RGB32F, inputSlot, true, offsetof(StaticMeshVertex, bitangent)),
		};
		return attributes;
	}

	StaticMesh::StaticMesh(uint32_t numVertices, uint32_t numTriangles, const math::AABB& aabb, uint8_t flags, const std::string& name)
		: m_name(name)
		, m_flags(flags)
		, m_numVertices(numVertices)
		, m_numTriangles(numTriangles)
		, m_aabb(aabb)
	{
		m_vertices.reserve(numVertices);
		m_triangles.reserve(numTriangles);
	}

	bool StaticMesh::addVertex(const StaticMeshVertex& vertex) noexcept
	{
		uint32_t vertexIndex = static_cast<uint32_t>(m_vertices.size());
		if (vertexIndex >= m_numVertices)
			return false;

		m_vertices.push_back(vertex);
		return true;
	}

	bool StaticMesh::addTriangle(const StaticMeshTriangle& triangle) noexcept
	{
		uint32_t triIndex = static_cast<uint32_t>(m_triangles.size());
		if (triIndex >= m_numTriangles)
			return false;

		m_triangles.push_back(triangle);
		return false;
	}

}; // engi namespace