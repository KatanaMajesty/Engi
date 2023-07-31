#pragma once

#include "Utility/SolidVector.h"
#include "Renderer/InstanceData.h"

namespace engi
{

	class InstanceTable
	{
	public:
		InstanceTable() = default;
		~InstanceTable() = default;

		uint32_t addInstanceData(const InstanceData& data) noexcept { return m_instanceData.insert(data); }
		void removeInstanceData(uint32_t id) noexcept { m_instanceData.erase(id); }
		InstanceData& getInstanceData(uint32_t id) noexcept { return m_instanceData[id]; }
		bool isOccupied(uint32_t id) const noexcept { return m_instanceData.isOccupied(id); }
		auto& getAllInstanceData() noexcept { return m_instanceData; }
		auto& getAllInstanceData() const noexcept { return m_instanceData; }

	private:
		SolidVector<InstanceData> m_instanceData;
	};

}; // engi namespace