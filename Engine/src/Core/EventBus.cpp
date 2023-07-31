#include "Core/EventBus.h"

namespace engi
{

	bool EventBus::init() noexcept
	{
		if (s_eventBus)
			EventBus::deinit();

		s_eventBus = new EventBus();
		return true;
	}

	void EventBus::deinit() noexcept
	{
		if (s_eventBus)
			delete s_eventBus;

		s_eventBus = nullptr;
	}

}; // engi namespace