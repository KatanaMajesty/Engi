#include "Core/Logger.h"

#include <iostream>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace engi
{

	bool Logger::init(LogLevel level)
	{
		spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
		m_handle = spdlog::stdout_color_mt("ENGINE");
		m_handle->set_pattern("%^[%H:%M:%S] [%n](%l): %v%$");
		ENGI_ASSERT(m_handle);
		return m_handle != nullptr;
	}

}; // engi namespace