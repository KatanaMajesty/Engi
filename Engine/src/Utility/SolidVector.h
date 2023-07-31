#pragma once

#include <cstdint>
#include <vector>
#include "Core/CommonDefinitions.h"

namespace engi
{

    template<typename T>
    class SolidVector
    {
    public:
        using IdType = uint32_t;

        SolidVector() = default;
        ~SolidVector() = default;

        bool isOccupied(IdType id) const noexcept;
        uint32_t getIndex(IdType id) const noexcept;
        constexpr uint32_t size() const noexcept;

        const T* data() const noexcept;
        const T& at(uint32_t index) const noexcept;
        const T& operator[](IdType id) const noexcept;
        
        T* data() noexcept;
        T& at(uint32_t index) noexcept;
        T& operator[](IdType id) noexcept;

        // TODO: Rewrite both insert funcs to use a single func with std::forward
        IdType insert(const T& value) noexcept;
        IdType insert(T&& value) noexcept;
        void erase(IdType id) noexcept;
        void clear() noexcept;

        constexpr auto begin() noexcept { return m_data.begin(); }
        constexpr auto end() noexcept { return m_data.end(); }

        constexpr const auto& getAllIDs() const noexcept { return m_backwardMap; }

    private:
        std::vector<T> m_data;
        std::vector<uint32_t> m_forwardMap;
        std::vector<IdType> m_backwardMap;
        std::vector<bool> m_occupied;
        IdType m_nextId = 0;
    };

}; // engi namespace

#include "Utility/SolidVector.inl"