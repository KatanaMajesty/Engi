#include "Timer.h"

namespace engi
{

	std::unique_ptr<Timer> Timer::s_timer = { nullptr };

	Timer::Timer()
		: m_begin(clock::now())
	{
	}

	void Timer::initialize()
	{
		if (s_timer)
		{
			return;
		}

		s_timer = std::unique_ptr<Timer>(new Timer());
	}

	void Timer::deinitialize() noexcept
	{
		if (s_timer)
		{
			s_timer.release();
		}
	}

	void Timer::reset() noexcept
	{
		s_timer->m_begin = clock::now();
	}

	float Timer::getTime() noexcept
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(clock::now() - s_timer->m_begin).count();
	}

}; // engi namespace

