#include "Decal.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/Renderer.h"
#include "Renderer/ShaderLibrary.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ModelRegistry.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/ImmutableBuffer.h"
#include "Renderer/DynamicBuffer.h"

namespace engi
{
	void DecalInstance::setNormalMap(Texture2D* texture) noexcept
	{
		ENGI_ASSERT(texture && "Normal map cannot be nullptr");
		this->normalMap = texture;
	}

	void DecalInstance::setParentInstance(InstanceTable* instanceTable, uint32_t parentInstanceID) noexcept
	{
		ENGI_ASSERT(instanceTable && "Provided instnace table cannot be nullptr");
		ENGI_ASSERT(parentInstanceID != uint32_t(-1) && instanceTable->isOccupied(parentInstanceID) && "Wrong instance ID is provided for a decal");
		this->instanceTable = instanceTable;
		this->parentInstanceID = parentInstanceID;
	}

	void DecalInstance::setLocalSpaceTransform(const math::Mat4x4& decalToWorld) noexcept
	{
		ENGI_ASSERT(this->instanceTable && "Instance table is needed in order to set local space transform from world space transform");
		ENGI_ASSERT(this->parentInstanceID != uint32_t(-1) && this->instanceTable->isOccupied(parentInstanceID) && "Wrong instance ID is provided for a decal instance");

		InstanceData& instanceData = this->instanceTable->getInstanceData(this->parentInstanceID);
		math::Mat4x4 decalToInstance = decalToWorld * instanceData.worldToModel;

		this->decalToInstance = decalToInstance;
		this->instanceToDecal = decalToInstance.inverse();
	}

	math::Mat4x4 DecalInstance::getWorldToDecal() const noexcept
	{
		ENGI_ASSERT(this->instanceTable && "Instance table is needed in order to set local space transform from world space transform");
		ENGI_ASSERT(this->parentInstanceID != uint32_t(-1) && this->instanceTable->isOccupied(parentInstanceID) && "Wrong instance ID is provided for a decal instance");

		InstanceData& instanceData = this->instanceTable->getInstanceData(this->parentInstanceID);
		return instanceData.worldToModel * this->instanceToDecal;
	}

	math::Mat4x4 DecalInstance::getDecalToWorld() const noexcept
	{
		ENGI_ASSERT(this->instanceTable && "Instance table is needed in order to set local space transform from world space transform");
		ENGI_ASSERT(this->parentInstanceID != uint32_t(-1) && this->instanceTable->isOccupied(parentInstanceID) && "Wrong instance ID is provided for a decal instance");

		InstanceData& instanceData = this->instanceTable->getInstanceData(this->parentInstanceID);
		return this->decalToInstance * instanceData.modelToWorld;
	}

}; // engi namespace