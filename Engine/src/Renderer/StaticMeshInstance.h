#pragma once

#include "Renderer/StaticMesh.h"
#include "Renderer/MaterialInstance.h"

namespace engi
{

	// Represents a single instance of a mesh combined with a material currently applied to it
	struct StaticMeshInstance
	{
		StaticMeshInstance(StaticMesh* mesh, const MaterialInstance& materialInstance = MaterialInstance::empty());
		~StaticMeshInstance() = default;

		inline bool hasMaterial() const noexcept { return !this->materialInstance.isEmpty(); }
		inline void setMaterial(const MaterialInstance& materialInstance) noexcept { this->materialInstance = materialInstance; }

		StaticMesh* mesh;
		MaterialInstance materialInstance;
	};

}; // engi namespace