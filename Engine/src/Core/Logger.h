#pragma once

#include <spdlog/spdlog.h>
#include "Core/CommonDefinitions.h"
#include "Utility/Memory.h"

namespace engi
{
	
	enum LogLevel
	{
		LEVEL_TRACE = SPDLOG_LEVEL_TRACE,
		LEVEL_DEBUG = SPDLOG_LEVEL_DEBUG,
		LEVEL_INFO = SPDLOG_LEVEL_INFO,
		LEVEL_WARN = SPDLOG_LEVEL_WARN,
		LEVEL_ERROR = SPDLOG_LEVEL_ERROR,
		LEVEL_CRITICAL = SPDLOG_LEVEL_CRITICAL,
		LEVEL_OFF = SPDLOG_LEVEL_OFF,
	};

	class Logger
	{
	public:
		static bool init(LogLevel level = LogLevel::LEVEL_TRACE);

		template<typename... Args>
		static void log(LogLevel level, const spdlog::format_string_t<Args...>& message, Args&&... args) noexcept
		{
			m_handle->log(spdlog::level::level_enum(level), message, std::forward<Args>(args)...);
		}

	private:
		static inline SharedHandle<spdlog::logger> m_handle = nullptr;
	};
	
}; // engi namespace

#define ENGI_LOG_TRACE(message, ...) (Logger::log(LEVEL_TRACE, message, ##__VA_ARGS__))
#define ENGI_LOG_DEBUG(message, ...) (Logger::log(LEVEL_DEBUG, message, ##__VA_ARGS__))
#define ENGI_LOG_INFO(message, ...) (Logger::log(LEVEL_INFO, message, ##__VA_ARGS__))
#define ENGI_LOG_WARN(message, ...) (Logger::log(LEVEL_WARN, message, ##__VA_ARGS__))
#define ENGI_LOG_ERROR(message, ...) (Logger::log(LEVEL_ERROR, message, ##__VA_ARGS__))
#define ENGI_LOG_CRITICAL(message, ...) (Logger::log(LEVEL_CRITICAL, message, ##__VA_ARGS__))
