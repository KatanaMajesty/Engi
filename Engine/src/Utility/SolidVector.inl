#pragma once

#include "Utility/SolidVector.h"
#include "SolidVector.h"

namespace engi
{

	template<typename T>
	inline bool SolidVector<T>::isOccupied(IdType id) const noexcept
	{ 
		ENGI_ASSERT(id < m_occupied.size()); 
		return m_occupied[id];
	}

	template<typename T>
	inline uint32_t SolidVector<T>::getIndex(IdType id) const noexcept
	{
		ENGI_ASSERT(id < m_occupied.size());
		ENGI_ASSERT(m_occupied[id]);
		return m_forwardMap[id];
	}

	template<typename T>
	inline constexpr uint32_t SolidVector<T>::size() const noexcept
	{
		return static_cast<uint32_t>(m_data.size());
	}

	template<typename T>
	inline const T* SolidVector<T>::data() const noexcept
	{
		return m_data.data();
	}

	template<typename T>
	inline T* SolidVector<T>::data() noexcept
	{
		return m_data.data();
	}

	template<typename T>
	inline const T& SolidVector<T>::at(uint32_t index) const noexcept
	{
		ENGI_ASSERT(index < m_data.size()); 
		return m_data[index];
	}

	template<typename T>
	inline const T& SolidVector<T>::operator[](IdType id) const noexcept
	{
		ENGI_ASSERT(id < m_occupied.size());
		ENGI_ASSERT(m_occupied[id]);
		return m_data[m_forwardMap[id]];
	}

	template<typename T>
	inline T& SolidVector<T>::at(uint32_t index) noexcept
	{
		ENGI_ASSERT(index < m_data.size()); 
		return m_data[index];
	}

	template<typename T>
	inline T& SolidVector<T>::operator[](IdType id) noexcept
	{
		ENGI_ASSERT(id < m_occupied.size());
		ENGI_ASSERT(m_occupied[id]);
		return m_data[m_forwardMap[id]];
	}

	template<typename T>
	inline SolidVector<T>::IdType SolidVector<T>::insert(const T& value) noexcept
	{
		IdType id = m_nextId;
		ENGI_ASSERT(id <= m_forwardMap.size() && m_forwardMap.size() == m_occupied.size());

		if (id == m_forwardMap.size())
		{
			m_forwardMap.push_back(uint32_t(m_forwardMap.size() + 1));
			m_occupied.push_back(false);
		}

		ENGI_ASSERT(!m_occupied[id]);

		m_nextId = m_forwardMap[id];
		m_forwardMap[id] = uint32_t(m_data.size());
		m_occupied[id] = true;
		m_data.emplace_back(value);
		m_backwardMap.emplace_back(id);
		return id;
	}

	template<typename T>
	inline SolidVector<T>::IdType SolidVector<T>::insert(T&& value) noexcept
	{
		IdType id = m_nextId;
		ENGI_ASSERT(id <= m_forwardMap.size() && m_forwardMap.size() == m_occupied.size());

		if (id == m_forwardMap.size())
		{
			m_forwardMap.push_back(uint32_t(m_forwardMap.size() + 1));
			m_occupied.push_back(false);
		}

		ENGI_ASSERT(!m_occupied[id]);

		m_nextId = m_forwardMap[id];
		m_forwardMap[id] = uint32_t(m_data.size());
		m_occupied[id] = true;
		m_data.emplace_back(std::move(value));
		m_backwardMap.emplace_back(id);
		return id;
	}

	template<typename T>
	inline void SolidVector<T>::erase(IdType id) noexcept
	{
		ENGI_ASSERT(id < m_forwardMap.size() && m_forwardMap.size() == m_occupied.size());

		uint32_t& forwardId = m_forwardMap[id];
		ENGI_ASSERT(m_occupied[id]);

		// swap-and-pop
		m_data[forwardId] = std::move(m_data.back());
		m_data.pop_back();

		IdType backwardId = m_backwardMap.back();
		m_backwardMap[forwardId] = backwardId;
		m_backwardMap.pop_back();
		m_forwardMap[backwardId] = forwardId;
		forwardId = m_nextId;
		m_occupied[id] = false;
		m_nextId = id;
	}

	template<typename T>
	inline void SolidVector<T>::clear() noexcept
	{
		m_forwardMap.clear();
		m_backwardMap.clear();
		m_occupied.clear();
		m_data.clear();
		m_nextId = 0;
	}

}; // engi namespace