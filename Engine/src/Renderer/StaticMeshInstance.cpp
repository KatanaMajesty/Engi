#include "Renderer/StaticMeshInstance.h"

namespace engi
{

	StaticMeshInstance::StaticMeshInstance(StaticMesh* mesh, const MaterialInstance& materialInstance)
		: mesh(mesh)
		, materialInstance(materialInstance)
	{
	}

}; // engi namespace