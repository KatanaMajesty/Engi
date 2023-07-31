#include "Renderer/Model.h"

#include <type_traits>
#include <algorithm>
#include "Core/CommonDefinitions.h"
#include "GFX/GPUDevice.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/ImmutableBuffer.h"

// TODO: Remove this header
#include <iostream>

namespace engi
{

	StaticMeshEntry::StaticMeshEntry(StaticMesh&& mesh, const MeshRange& range)
		: mesh(std::move(mesh))
		, range(range)
	{
	}

	bool StaticMeshEntry::initialize() noexcept
	{
		return bvh.initialize(&this->mesh);
	}

	Model::Model(const std::string& name, gfx::IGpuDevice* device, uint32_t staticMeshEntries)
		: m_name(name)
		, m_device(device)
		, m_staticEntriesCapacity(staticMeshEntries)
	{
		m_staticMeshes.reserve(static_cast<size_t>(staticMeshEntries));
	}

	Model::~Model()
	{
	}

	bool Model::addStaticMeshEntry(StaticMeshEntry&& meshEntry) noexcept
	{
		uint32_t entries = static_cast<uint32_t>(m_staticMeshes.size());
		if (entries >= m_staticEntriesCapacity)
		{
			std::cout << "Model's static mesh entry container is full. Refusing to add a mesh entry\n";
			return false;
		}

		if (!meshEntry.isValid())
		{
			std::cout << "Trying to add a mesh entry of " << meshEntry.mesh.getName() << ", which is an invalid mesh. Refusing to do so\n";
			std::cout << "Check if the mesh is triangle-full and vertex-full\n";
			return false;
		}

		m_staticMeshes.emplace_back(std::move(meshEntry));
		return true;
	}

	bool Model::initialize() noexcept
	{
		if (m_staticEntriesCapacity == 0 || m_staticMeshes.size() == 0)
		{
			std::cout << "Tried to initialize empty model\n";
			return false;
		}

		//static constexpr bool validType = std::is_same_v<StaticMesh::TriangleType::IndexType, IndexBuffer::IndexType>;
		//ENGI_ASSERT(validType && "StaticMesh indexing type is not suitable for Engi's index buffer");
		uint32_t numVertices = 0;
		uint32_t numIndices = 0;
		for (StaticMeshEntry& entry : m_staticMeshes)
		{
			if (!entry.isValid())
			{
				std::cout << "Static mesh entry of " << entry.mesh.getName() << " is invalid\n";
				return false;
			}

			entry.initialize();
			numVertices += entry.mesh.getNumVertices();
			numIndices += entry.mesh.getNumTriangles() * 3;
		}

		std::vector<StaticMeshVertex> modelVertices;
		std::vector<StaticMeshTriangle::IndexType> modelIndices;
		modelVertices.reserve(numVertices);
		modelIndices.reserve(numIndices);
		for (const StaticMeshEntry& entry : m_staticMeshes)
		{
			std::ranges::copy(entry.mesh.getVertices(), std::back_inserter(modelVertices));

			const StaticMesh::TriangleContainerType& triangles = entry.mesh.getTriangles();
			for (const StaticMeshTriangle& tri : triangles)
			{
				modelIndices.push_back(tri.indices[0]);
				modelIndices.push_back(tri.indices[1]);
				modelIndices.push_back(tri.indices[2]);
			}
		}

		m_vbo.reset(new ImmutableBuffer("Model_VBO_" + this->getPath(), m_device));
		if (!m_vbo || !m_vbo->init(modelVertices.data(), numVertices, sizeof(StaticMeshVertex)))
		{
			std::cout << "Failed to create model's vertex buffer\n";
			return false;
		}
		m_ibo.reset(new IndexBuffer("Model_IBO_" + this->getPath(), m_device));
		if (!m_ibo || !m_ibo->initialize(modelIndices.data(), numIndices))
		{
			std::cout << "Failed to create model's index buffer\n";
			return false;
		}

		return true;
	}

}; // engi namespace