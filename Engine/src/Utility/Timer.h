#pragma once

#include <memory>
#include <chrono>
#include <Windows.h>

namespace engi
{

	// Timer with Dur = nanos (using steady_clock) will be initialized right after the engi::Main function
	// using uninitialized Timer is UB
	class Timer
	{
	public:
		using clock = std::chrono::steady_clock;
		using duration = clock::duration;
		using time_point = clock::time_point;

	public:
		~Timer() = default;

		static void reset() noexcept;
		static float getTime() noexcept;

	private:
		friend int32_t WINAPI ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int32_t nCmdShow);

		Timer();

		static void initialize();
		static void deinitialize() noexcept;
		static std::unique_ptr<Timer> s_timer;

		time_point m_begin;
	};

}