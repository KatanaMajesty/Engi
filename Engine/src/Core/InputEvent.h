#pragma once

#include "Core/Keycode.h"
#include "Core/Event.h"

namespace engi
{

	class Window;

	// Window may be null, if the key was pressed in minimized window
	struct EvKeyPressed : Event
	{
		EvKeyPressed(Window* window, Keycode keycode)
			: window(window)
			, keycode(keycode)
		{
		}

		Window* window;
		Keycode keycode;
	};

	// Window may be null, if the key was pressed in minimized window
	struct EvKeyReleased : Event
	{
		EvKeyReleased(Window* window, Keycode keycode)
			: window(window)
			, keycode(keycode)
		{
		}

		Window* window;
		Keycode keycode;
	};

}; // engi namespace