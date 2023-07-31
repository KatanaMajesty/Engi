#include "Core/Input.h"

#include "Utility/Memory.h"
#include "Core/EventBus.h"

namespace engi
{
	EngiIO& EngiIO::get()
	{
		return *s_io.get();
	}

	bool EngiIO::isPressed(Keycode code)
	{
		return m_keyMap[code].pressed;
	}

	bool EngiIO::isReleased(Keycode code)
	{
		return !m_keyMap[code].pressed;
	}

	math::Vec2 EngiIO::getCursorPosNDC() const
	{
		const math::Vec2& pos = m_mouse.getCursor().pos;
		math::Vec2 ndc;
		ndc.x = ((2.0f * pos.x) / m_width) - 1.0f;
		ndc.y = -((2.0f * pos.y) / m_height) + 1.0f;
		return ndc;
	}

	void EngiIO::onUpdate(HWND handle)
	{
		updateCursorPos(handle);
	}

	void EngiIO::onResize(const EvWindowResized& ev) noexcept
	{
		m_width = ev.width;
		m_height = ev.height;
	}

	void EngiIO::updateCursorPos(HWND handle)
	{
		POINT point;
		if (GetCursorPos(&point) && ScreenToClient(handle, &point))
		{
			// We want to invert the Y axis, because of the way, we cast rays and treat our view frustum
			math::Vec2 pos(static_cast<float>(point.x), m_height - static_cast<float>(point.y));
			pos.clamp(math::Vec2(0.0f), math::Vec2(static_cast<float>(m_width), static_cast<float>(m_height)));
			m_mouse.getCursor().pos = pos;
		}
	}

	void EngiIO::init()
	{
		if (s_io)
		{
			deinit();
		}

		s_io = UniqueHandle<EngiIO>(new EngiIO());
		EventBus::get().subscribe(s_io.get(), &EngiIO::onResize);
	}

	void EngiIO::deinit()
	{
		if (s_io)
		{
			EventBus::get().unsubscribeAll(s_io.get());
			s_io.release();
		}
	}

	namespace detail
	{
		std::unordered_map<WPARAM, uint32_t> win32keyRemap = 
		{
			{ VK_BACK, Keycode::BACKSPACE},
			{ VK_TAB,	Keycode::TAB },
			{ VK_RETURN, Keycode::ENTER },
			{ VK_SHIFT,	Keycode::SHIFT },
			{ VK_CONTROL,Keycode::CTRL },
			{ VK_MENU,	Keycode::ALT },
			{ VK_ESCAPE,	Keycode::ESCAPE },
			{ VK_SPACE,	Keycode::SPACE },
			{ VK_CAPITAL,Keycode::CAPS_LOCK },
			{ 0x30,	Keycode::K0 },
			{ 0x31,	Keycode::K1 },
			{ 0x32,	Keycode::K2 },
			{ 0x33,	Keycode::K3 },
			{ 0x34,	Keycode::K4 },
			{ 0x35,	Keycode::K5 },
			{ 0x36,	Keycode::K6 },
			{ 0x37,	Keycode::K7 },
			{ 0x38,	Keycode::K8 },
			{ 0x39,	Keycode::K9 },
			{ 0x41,	Keycode::A },
			{ 0x42,	Keycode::B },
			{ 0x43,	Keycode::C },
			{ 0x44,	Keycode::D },
			{ 0x45,	Keycode::E },
			{ 0x46,	Keycode::F },
			{ 0x47,	Keycode::G },
			{ 0x48,	Keycode::H },
			{ 0x49,	Keycode::I },
			{ 0x4A,	Keycode::J },
			{ 0x4B,	Keycode::K },
			{ 0x4C,	Keycode::L },
			{ 0x4D,	Keycode::M },
			{ 0x4E,	Keycode::N },
			{ 0x4F,	Keycode::O },
			{ 0x50,	Keycode::P },
			{ 0x51,	Keycode::Q },
			{ 0x52,	Keycode::R },
			{ 0x53,	Keycode::S },
			{ 0x54,	Keycode::T },
			{ 0x55,	Keycode::U },
			{ 0x56,	Keycode::V },
			{ 0x57,	Keycode::W },
			{ 0x58,	Keycode::X },
			{ 0x59,	Keycode::Y },
			{ 0x5A,	Keycode::Z },
			{ VK_NUMPAD0, Keycode::NUM_K0 },
			{ VK_NUMPAD1, Keycode::NUM_K1 },
			{ VK_NUMPAD2, Keycode::NUM_K2 },
			{ VK_NUMPAD3, Keycode::NUM_K3 },
			{ VK_NUMPAD4, Keycode::NUM_K4 },
			{ VK_NUMPAD5, Keycode::NUM_K5 },
			{ VK_NUMPAD6, Keycode::NUM_K6 },
			{ VK_NUMPAD7, Keycode::NUM_K7 },
			{ VK_NUMPAD8, Keycode::NUM_K8 },
			{ VK_NUMPAD9, Keycode::NUM_K9 },
			{ VK_F1, Keycode::F1 },		// F1 key
			{ VK_F2, Keycode::F2 },		// F2 key
			{ VK_F3, Keycode::F3 },		// F3 key
			{ VK_F4, Keycode::F4 },		// F4 key
			{ VK_F5, Keycode::F5 },		// F5 key
			{ VK_F6, Keycode::F6 },		// F6 key
			{ VK_F7, Keycode::F7 },		// F7 key
			{ VK_F8, Keycode::F8 },		// F8 key
			{ VK_F9, Keycode::F9 },		// F9 key
			{ VK_F10, Keycode::F10 },		// F10 key
			{ VK_F11, Keycode::F11 },		// F11 key
			{ VK_F12, Keycode::F12 },		// F12 key
		};

	}; // detail namespace

}; // engi namespace

