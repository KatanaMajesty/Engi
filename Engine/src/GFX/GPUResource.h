#pragma once

#include <string>

namespace engi::gfx
{

	class IGpuDevice;

	class IGpuResource
	{
	public:
		IGpuResource() = default;
		virtual ~IGpuResource() = default;

		const std::string& getName() const { return m_name; }

		virtual void* getHandle() = 0;

	protected:
		IGpuDevice* m_device = nullptr;
		std::string m_name;
	};

}; // engi::gfx namespace