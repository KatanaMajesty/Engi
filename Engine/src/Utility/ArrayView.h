#pragma once

#include <span>
#include <concepts>

namespace engi
{

	template<typename T, size_t Extent = std::dynamic_extent>
	using ArrayView = std::span<T, Extent>;

	template<typename T>
	auto viewOf(const T* ptr, size_t num) -> ArrayView<const T>
	{
		return ArrayView(ptr, num);
	}

	template<typename T>
	auto viewOf(T& arg) -> ArrayView<const T>
	{
		return viewOf(&arg, 1);
	}

}; // engi namespace