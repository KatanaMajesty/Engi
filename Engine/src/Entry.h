#pragma once

#include <cstdint>

// include Windows.h before DirectXMath.h in order to check CPU support
#include <Windows.h>
#include <DirectXMath.h>

#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/Logger.h"
#include "Utility/Timer.h"

namespace engi
{
	extern void Main(char* cmdline, int32_t cmdshow);
}; // engi namespace

int32_t WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR cmdline, _In_ int32_t cmdshow)
{
	ENGI_ASSERT(DirectX::XMVerifyCPUSupport() && "DirectXMath is not supported on this CPU");
	engi::Application::create(instance, cmdline, cmdshow);
	engi::LogLevel level = engi::LogLevel::LEVEL_ERROR;
#ifndef NDEBUG
	level = engi::LogLevel::LEVEL_TRACE;
#endif
	engi::Logger::init(level);
	engi::Timer::initialize();
	engi::EngiIO::init();

	engi::Main(cmdline, cmdshow);
	int32_t res = engi::Application::get().run(); // currently unused

	engi::EngiIO::deinit();
	engi::Timer::deinitialize();
	engi::Application::destroy();

	return res;
}