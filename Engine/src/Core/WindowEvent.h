#pragma once

#include <cstdint>
#include "Core/Event.h"

namespace engi
{

	class Window;

	struct EvWindowResized : Event
	{
		EvWindowResized(Window* window, uint32_t width, uint32_t height)
			: window(window)
			, width(width)
			, height(height)
		{
		}

		Window* window;
		uint32_t width;
		uint32_t height;
	};

}; // engi namespace