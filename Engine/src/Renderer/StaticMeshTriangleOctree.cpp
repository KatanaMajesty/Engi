#include "Renderer/StaticMeshTriangleOctree.h"

#include <algorithm>
#include "Core/CommonDefinitions.h"

namespace engi
{

	using namespace math;

	bool StaticMeshTriangleOctree::initialize(const StaticMesh* mesh) noexcept
	{
		if (!mesh || mesh->isEmpty() || !mesh->isTriangleFull() || !mesh->isVertexFull())
			return false;

		m_triangleIndices.clear();
		m_triangleIndices.shrink_to_fit();

		m_mesh = mesh;
		m_children.reset(); // be more verbose

		static constexpr Vec3 eps(Numeric::epsilon());
		const AABB& meshBox = mesh->getAABB();
		m_box = AABB(meshBox.min - eps, meshBox.max + eps);
		m_initialBox = m_box;

		// iterate though all triangles and fill m_triangles
		uint32_t numTriangles = mesh->getNumTriangles();

		const StaticMesh::VertexContainerType& vertices = mesh->getVertices();
		const StaticMesh::TriangleContainerType& triangles = mesh->getTriangles();
		for (uint32_t triIndex = 0; triIndex < numTriangles; ++triIndex)
		{
			const StaticMeshTriangle& meshTri = triangles[triIndex];
			const Vec3& p0 = vertices[meshTri.indices[0]].position;
			const Vec3& p1 = vertices[meshTri.indices[1]].position;
			const Vec3& p2 = vertices[meshTri.indices[2]].position;
			Vec3 center = (p0 + p1 + p2) / 3.0f;
			if (!addTriangle(triIndex, p0, p1, p2, center))
				return false;
		}

		return true;
	}

	bool StaticMeshTriangleOctree::addTriangle(uint32_t triIndex, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& center)
	{
		if (!m_initialBox.contains(center) ||
			!m_box.contains(p0) ||
			!m_box.contains(p1) ||
			!m_box.contains(p2))
		{
			// This means that meshes' AABB is initialized incorrectly
			// if either one of points isn't in the boxes, then no need for further calculations,
			// this means that we won't parse this triangle
			return false;
		}

		// check if no children were created
		if (!m_children)
		{
			// if we still can insert triangle indices in node, then we do it and return true
			if (m_triangleIndices.size() < StaticMeshTriangleOctreeTraits::octantTriangles())
			{
				// we insert an index and return true, because at this state 
				m_triangleIndices.push_back(triIndex);
				return true;
			}
			else
			{
				const AABB& parentBox = m_initialBox;
				Vec3 parentBoxCenter = (parentBox.min + parentBox.max) / 2.0f;

				// create 8 children for the current node
				m_children.reset(new std::array<StaticMeshTriangleOctree, 8>());
				for (uint32_t childIndex = 0; childIndex < 8; ++childIndex)
				{
					// initialize every child
					// we initialize them with different function, because they have a parent AABB
					m_children->at(childIndex).initialize(m_mesh, parentBox, parentBoxCenter, childIndex);
				}

				std::vector<uint32_t> newTriangleIndices;
				const StaticMesh::VertexContainerType& vertices = m_mesh->getVertices();
				const StaticMesh::TriangleContainerType& triangles = m_mesh->getTriangles();
				for (uint32_t index : m_triangleIndices)
				{
					const StaticMeshTriangle& otherTriangle = triangles[index];
					const Vec3& otherP0 = vertices[otherTriangle.indices[0]].position;
					const Vec3& otherP1 = vertices[otherTriangle.indices[1]].position;
					const Vec3& otherP2 = vertices[otherTriangle.indices[2]].position;
					Vec3 otherCenter = (otherP0 + otherP1 + otherP2) / 3.0f;

					uint32_t childIndex = 0;
					for (; childIndex < 8; ++childIndex)
					{
						if (m_children->at(childIndex).addTriangle(index, otherP0, otherP1, otherP2, otherCenter))
						{
							break;
						}
					}

					// if we didn't insert the triangle in any of the children
					// we insert it in the parent (current)
					if (childIndex == 8)
					{
						newTriangleIndices.push_back(index);
					}
				}

				// assign newTriangles to m_triangles
				m_triangleIndices = std::move(newTriangleIndices);
			}
		}

		uint32_t childIndex = 0;
		for (; childIndex < 8; ++childIndex)
		{
			if (m_children->at(childIndex).addTriangle(triIndex, p0, p1, p2, center))
			{
				break;
			}
		}

		if (childIndex == 8)
		{
			m_triangleIndices.push_back(triIndex);
		}

		return true;
	}

	void StaticMeshTriangleOctree::initialize(const StaticMesh* mesh, const AABB& parentBox, const Vec3& parentCenter, int32_t octetIndex)
	{
		m_mesh = mesh;
		m_children.reset(); // be more verbose

		if (octetIndex % 2 == 0)
		{
			m_initialBox.min.x = parentBox.min.x;
			m_initialBox.max.x = parentCenter.x;
		}
		else
		{
			m_initialBox.min.x = parentCenter.x;
			m_initialBox.max.x = parentBox.max.x;
		}

		if (octetIndex % 4 < 2)
		{
			m_initialBox.min.y = parentBox.min.y;
			m_initialBox.max.y = parentCenter.y;
		}
		else
		{
			m_initialBox.min.y = parentCenter.y;
			m_initialBox.max.y = parentBox.max.y;
		}

		if (octetIndex < 4)
		{
			m_initialBox.min.z = parentBox.min.z;
			m_initialBox.max.z = parentCenter.z;
		}
		else
		{
			m_initialBox.min.z = parentCenter.z;
			m_initialBox.max.z = parentBox.max.z;
		}

		m_box = m_initialBox;
		Vec3 elongation = m_box.size() * (StaticMeshTriangleOctreeTraits::stretchingRatio() - 1.0f);

		if (octetIndex % 2 == 0)
		{
			m_box.max.x += elongation.x;
		}
		else
		{
			m_box.min.x -= elongation.x;
		}

		if (octetIndex % 4 < 2)
		{
			m_box.max.y += elongation.y;
		}
		else
		{
			m_box.min.y -= elongation.y;
		}

		if (octetIndex < 4)
		{
			m_box.max.z += elongation.z;
		}
		else
		{
			m_box.min.z -= elongation.z;
		}
	}

	StaticMeshTriangleOctree::IntersectionType StaticMeshTriangleOctree::intersect(const Ray& meshRay) const noexcept
	{
		IntersectionType result;
		result.reset();

		if (!meshRay.intersects(m_box, nullptr))
		{
			return result;
		}

		// TODO: Add attributes (maybe_unused)
		intersectInternal(meshRay, result);
		return result;
	}

	struct OctantIntersection
	{
		uint32_t index;
		float t;

		inline constexpr void reset() { t = Numeric::infinity(); }
		inline constexpr bool isValid() const { return Numeric::isFinite(t); }

		inline constexpr bool operator<(const OctantIntersection& other) const { return t < other.t; }
		inline constexpr bool operator>(const OctantIntersection& other) const { return t > other.t; }
	}; // OctantIntersection struct

	bool StaticMeshTriangleOctree::intersectInternal(const Ray& meshRay, IntersectionType& meshIntersection) const
	{
		bool found = false;
		const StaticMesh::VertexContainerType& vertices = m_mesh->getVertices();
		const StaticMesh::TriangleContainerType& triangles = m_mesh->getTriangles();
		for (size_t triIndex : m_triangleIndices)
		{
			const StaticMeshTriangle& triangle = triangles[triIndex];
			const StaticMeshVertex& v0 = vertices[triangle.indices.at(0)];
			const StaticMeshVertex& v1 = vertices[triangle.indices.at(1)];
			const StaticMeshVertex& v2 = vertices[triangle.indices.at(2)];

			float triT = Numeric::infinity();
			// TODO: TEMP!
			if (meshRay.intersects(v0.position, v1.position, v2.position, &triT) && meshIntersection.t > triT)
			{
				meshIntersection.t = triT;
				meshIntersection.hitpos = meshRay.origin + (meshRay.direction * triT);
				//meshIntersection.normal = v0.normal; // remark! this normal is in mesh space
				found = true;
			}
		}

		// return if no children left
		if (!m_children)
		{
			return found;
		}

		std::array<OctantIntersection, 8> boxIntersections;
		for (uint32_t octantIndex = 0; octantIndex < 8; ++octantIndex)
		{
			OctantIntersection& childIntersection = boxIntersections[octantIndex];
			childIntersection.reset();
			childIntersection.index = octantIndex;

			StaticMeshTriangleOctree& child = m_children->at(octantIndex);
			if (child.m_box.contains(meshRay.origin))
			{
				childIntersection.t = 0.0f;
				continue;
			}

			// This will only change distance t if intersection occurs, otherwise it stays infinity
			meshRay.intersects(child.m_box, &childIntersection.t);
		}

		std::ranges::sort(boxIntersections, std::less<OctantIntersection>{});
		for (uint32_t octantIndex = 0; octantIndex < 8; ++octantIndex)
		{
			OctantIntersection& childIntersection = boxIntersections[octantIndex];
			if (!childIntersection.isValid() || childIntersection.t > meshIntersection.t)
			{
				continue;
			}

			if (m_children->at(childIntersection.index).intersectInternal(meshRay, meshIntersection))
			{
				found = true;
			}
		}

		return found;
	}

}; // engi namespace