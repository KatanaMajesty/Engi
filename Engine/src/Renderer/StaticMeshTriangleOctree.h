#pragma once

#include <vector>
#include <array>
#include "Math/Math.h"
#include "Utility/Memory.h"
#include "Renderer/StaticMesh.h"

// leave them as defines, for different builds
#define ENGI_TRIANGLEOCTREE_PREFERRED_TRIANGLES 32
#define ENGI_TRIANGLEOCTREE_STRETCHING_RATIO 1.05f

namespace engi
{

	struct StaticMeshTriangleOctreeTraits
	{
		static inline constexpr uint32_t octantTriangles() { return static_cast<uint32_t>(ENGI_TRIANGLEOCTREE_PREFERRED_TRIANGLES); }
		static inline constexpr float stretchingRatio() { return static_cast<float>(ENGI_TRIANGLEOCTREE_STRETCHING_RATIO); }
	};

	class StaticMeshTriangleOctree
	{
	public:
		using IntersectionType = StaticMeshIntersection;
		using TriangleIndexContainerType = std::vector<uint32_t>;
		using OctreeChildrenHandle = SharedHandle<std::array<StaticMeshTriangleOctree, 8>>;

		StaticMeshTriangleOctree() = default;
		StaticMeshTriangleOctree(const StaticMeshTriangleOctree&) = delete;
		StaticMeshTriangleOctree& operator=(const StaticMeshTriangleOctree&) = delete;
		StaticMeshTriangleOctree(StaticMeshTriangleOctree&&) = default;
		StaticMeshTriangleOctree& operator=(StaticMeshTriangleOctree&&) = default;
		~StaticMeshTriangleOctree() = default;

		bool initialize(const StaticMesh* mesh) noexcept;
		IntersectionType intersect(const math::Ray& ray) const noexcept;

	private:
		void initialize(const StaticMesh* mesh, const math::AABB& parentBox, const math::Vec3& parentCenter, int32_t octetIndex);
		bool addTriangle(uint32_t triIndex, const math::Vec3& p0, const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& center);
		bool intersectInternal(const math::Ray& meshRay, IntersectionType& meshIntersection) const;

		const StaticMesh* m_mesh = nullptr;
		TriangleIndexContainerType m_triangleIndices;
		OctreeChildrenHandle m_children = nullptr;
		math::AABB m_box;
		math::AABB m_initialBox;
	};

}; // engi namespace