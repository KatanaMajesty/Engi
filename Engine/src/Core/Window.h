#pragma once

#include <memory>
#include <vector>
#include <string>

// TODO: This may be removed when we implement an event bus
#include "GFX/DX11/D3D11_API.h"

namespace engi
{

	struct WindowSpecs
	{
		WindowSpecs(const std::string& title = "Unnamed Window", int32_t width = 800, int32_t height = 600);

		std::string title;
		int32_t width;
		int32_t height;
	};

	struct WindowContext
	{
		int32_t width;
		int32_t height;
		int32_t cmdShow = 0;
		HINSTANCE instance = nullptr;
		HWND handle = nullptr;
		std::wstring instanceName; // should be immutable!
		bool vsync = false;
		bool closed = false;
	};

	class Window
	{
	public:
		Window(HINSTANCE instance, const std::wstring& instanceName, int32_t cmdshow);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window() = default;

		bool initialize(const WindowSpecs& specs);
		void open();
		void close();
		bool shouldClose() const noexcept { return m_context.closed; }
		void requestResize(int32_t width, int32_t height);
		void onUpdate(float timestep);
		constexpr int32_t getWidth() const noexcept { return m_context.width; }
		constexpr int32_t getHeight() const noexcept { return m_context.height; }
		HWND getHandle() { return m_context.handle; }
		bool isVsync() { return m_context.vsync; }

	private:
		WindowSpecs m_specification;
		WindowContext m_context;
		bool m_shouldResize = false;
	};

}; // engi namespace
