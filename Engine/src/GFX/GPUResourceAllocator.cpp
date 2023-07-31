#include "GFX/GpuResourceAllocator.h"

#include <algorithm>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"

namespace engi::gfx
{

	GpuResourceAllocator::~GpuResourceAllocator()
	{
		for (IGpuResource* resource : m_allocatedResources)
		{
			ENGI_LOG_WARN("Resource {} was not properly destroyed!", resource->getName());
			deleteResource(resource);
		}
	}

	void GpuResourceAllocator::destroyResource(IGpuResource*& resource)
	{
		if (!resource || m_allocatedResources.empty())
		{
			return;
		}

		auto it = std::ranges::find(m_allocatedResources, resource);
		if (it == m_allocatedResources.end())
		{
			return;
		}

		resource = nullptr;
		deleteResource(*it);
		m_allocatedResources.erase(it);
	}

	void GpuResourceAllocator::deleteResource(IGpuResource* resource)
	{
		delete resource;
	}

	GpuResourceDeleter::GpuResourceDeleter(GpuResourceAllocator* allocator)
		: m_allocator(allocator)
	{
		ENGI_ASSERT(allocator && "Provided allocator is nullptr");
	}

	void GpuResourceDeleter::operator()(IGpuResource* resource)
	{
		/*if (!m_allocator)
		{
			return;
		}*/
		ENGI_ASSERT(m_allocator && "Allocator was freed before invoking this GpuReousrceDeleter");
		m_allocator->destroyResource(resource);
	}

}; // engi::gfx namespace