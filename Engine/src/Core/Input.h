#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <Windows.h>
#include "Core/Keycode.h"
#include "Core/WindowEvent.h"

#include "Math/Math.h"

namespace engi
{

	struct KeyState
	{
		bool pressed;
	};

	// Use composition for future code
	struct Cursor
	{
		// cursor's position is stored in ndc (ranges from -1 to 1 in both x and y)
		// y is inverted (from top to bottom)
		math::Vec2 pos;
	};

	class Mouse
	{
	public:
		inline constexpr const Cursor& getCursor() const { return m_cursor; }
		inline constexpr Cursor& getCursor() { return m_cursor; }

		void setScrollDelta(const math::Vec2& delta) { m_scrollDelta = delta; }
		void addScrollDelta(const math::Vec2& delta) { m_scrollDelta += delta; }

		inline constexpr const math::Vec2& getScrollDelta() const { return m_scrollDelta; }

	private:
		Cursor m_cursor;
		math::Vec2 m_scrollDelta;
	};

	class EngiIO
	{
	public:
		EngiIO() = default;
		~EngiIO() = default;

	public:
		static EngiIO& get();

	public:
		inline constexpr KeyState& getState(Keycode code) { return m_keyMap[code]; }
		inline constexpr const KeyState& getState(Keycode code) const { return m_keyMap[code]; }
		bool isPressed(Keycode code);
		bool isReleased(Keycode code);

		inline constexpr Mouse& getMouse() { return m_mouse; }
		inline constexpr const Mouse& getMouse() const { return m_mouse; }

		inline constexpr math::Vec2 getCursorPos() const { return m_mouse.getCursor().pos; }
		math::Vec2 getCursorPosNDC() const;

		void onUpdate(HWND handle);
		void onResize(const EvWindowResized& ev) noexcept;

	private:
		friend int32_t WINAPI::WinMain(HINSTANCE, HINSTANCE, LPSTR, int32_t);

		void updateCursorPos(HWND handle);
		static void init();
		static void deinit();

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		Mouse m_mouse;
		KeyState m_keyMap[Keycode::NUMBER_OF_KEYCODES];

	private:
		static inline std::unique_ptr<EngiIO> s_io;
	}; // EngiIO class

	namespace detail
	{
		// Key mapping from WinAPI to Engi's Keycodes
		extern std::unordered_map<WPARAM, uint32_t> win32keyRemap;
	}; // detail namespace

}; // engi namespace