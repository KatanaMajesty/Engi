#pragma once

#include <memory>

namespace engi
{

	template<typename T>
	using DefaultDeleter = std::default_delete<T>;

	template<typename T, typename Deleter = DefaultDeleter<T>> using UniqueHandle = std::unique_ptr<T, Deleter>;
	template<typename T> using SharedHandle = std::shared_ptr<T>;

	template<typename T, typename U, typename Deleter = DefaultDeleter<T>>
	UniqueHandle<T, Deleter> makeUnique(U* ptr, Deleter&& deleter = {})
	{
		return UniqueHandle<T, Deleter>(static_cast<T*>(ptr), deleter);
	}

	// We want to use pointer as a parameter, despite 2 heap allocation, because this would allow us to use allocators in future
	template<typename T, typename U>
	SharedHandle<T> makeShared(U* ptr)
	{
		return SharedHandle<T>(static_cast<T*>(ptr));
	}

}; // engi namespace