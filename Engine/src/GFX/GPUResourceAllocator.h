#pragma once

#include <vector>
#include <concepts>

#include "Utility/Memory.h"
#include "GFX/GPUResource.h"

namespace engi::gfx
{

	template<typename T>
	concept isGpuResourceType = std::derived_from<T, IGpuResource>;

	class GpuResourceAllocator
	{
	public:
		GpuResourceAllocator() = default;
		~GpuResourceAllocator();

		template<isGpuResourceType T, typename... Args>
		T* createResource(Args&&... args)
		{
			return static_cast<T*>(m_allocatedResources.emplace_back(new T(std::forward<Args>(args)...)));
		}

		// If this function succeeds, then the provided pointer will be set to null
		void destroyResource(IGpuResource*& resource);

	private:
		void deleteResource(IGpuResource* resource);

		std::vector<IGpuResource*> m_allocatedResources;
	};

	struct GpuResourceDeleter
	{
		GpuResourceDeleter() = default;
		GpuResourceDeleter(GpuResourceAllocator* allocator);
		~GpuResourceDeleter() = default;

		void operator()(IGpuResource* resource);

		GpuResourceAllocator* m_allocator = nullptr;
	};

	template<typename T>
	using GpuHandle = engi::UniqueHandle<T, GpuResourceDeleter>;

	template<typename T>
	GpuHandle<T> makeGpuHandle(T* ptr, GpuResourceAllocator* allocator)
	{
		return makeUnique<T, T, GpuResourceDeleter>(ptr, GpuResourceDeleter(allocator));
	}

}; // engi::gfx namespace
