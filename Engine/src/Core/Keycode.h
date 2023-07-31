#pragma once

namespace engi
{
	
	enum Keycode
	{
		LMB,
		RMB,
		BACKSPACE,
		TAB,
		ENTER,
		SHIFT,
		CTRL,
		ALT,
		//WIN_L, WIN_R, // Left / Right Windows key(Natural keyboard)
		ESCAPE,
		SPACE,
		CAPS_LOCK,
		K0, K1, K2, K3, K4, K5, K6, K7, K8, K9, // Keys 0 - 9
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, // Keys A - Z
		NUM_K0, NUM_K1, NUM_K2, NUM_K3, NUM_K4, NUM_K5, NUM_K6, NUM_K7, NUM_K8, NUM_K9, // Numeric keypad 0 - 9 keys
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, // F1 - F12 keys
		NUMBER_OF_KEYCODES // DONT USE! SHOULD ALWAYS BE IN THE END!
	};
};