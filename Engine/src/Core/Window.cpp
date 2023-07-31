#include "Core/Window.h"

#include <unordered_map>

#include "Core/Input.h"

// TODO: Remove this headers
#include <iostream>
#include "Core/EventBus.h"
#include "Core/WindowEvent.h"
#include "Core/Application.h" // TODO: THIS SHOULD BE REMOVED ASAP
#include "Renderer/Renderer.h" // TODO: THIS SHOULD BE REMOVED ASAP
#include "World/CameraController.h" // TODO: THIS SHOULD BE REMOVED ASAP

namespace engi
{
	Window::Window(HINSTANCE instance, const std::wstring& instanceName, int32_t cmdshow)
	{
		// TODO: Maybe this should be moved somewhere?
		m_context.instance = instance;
		m_context.instanceName = instanceName;
		m_context.instance = instance;
		m_context.cmdShow = cmdshow;
	}

	bool Window::initialize(const WindowSpecs& specs)
	{
		m_specification = specs;
		m_context.width = specs.width;
		m_context.height = specs.height;

		RECT clientRect = { 0, 0, m_specification.width, m_specification.height };
		AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
		int32_t width = clientRect.right - clientRect.left;
		int32_t height = clientRect.bottom - clientRect.top;

		auto& handle = m_context.handle;
		handle = CreateWindowExW(NULL,
			m_context.instanceName.c_str(),
			std::wstring(m_specification.title.begin(), m_specification.title.end()).c_str(), // temp, create a conversion functions
			WS_OVERLAPPEDWINDOW,
			0, 0, width, height,
			nullptr, nullptr, m_context.instance, nullptr);

		if (!handle)
		{
			return false;
		}

		return true;
	}

	void Window::close()
	{
		m_context.closed = true;
		m_context.handle = nullptr; // is this okay?
		if (m_context.instance)
		{
			UnregisterClass(m_context.instanceName.c_str(), m_context.instance);
		}
	}

	void Window::requestResize(int32_t width, int32_t height)
	{
		m_shouldResize = true;
		m_context.width = width;
		m_context.height = height;
	}

	void Window::onUpdate(float timestep)
	{
		if (m_shouldResize)
		{
			m_shouldResize = false;
			EventBus::get().publish(EvWindowResized(this, m_context.width, m_context.height));
		}
	}

	void Window::open()
	{
		SetWindowLongPtrW(m_context.handle, 0, reinterpret_cast<LONG_PTR>(&Application::get())); // TODO: This should be rewritten asap
		ShowWindow(m_context.handle, m_context.cmdShow);
	}

	WindowSpecs::WindowSpecs(const std::string& title, int32_t width, int32_t height)
		: title(title)
		, width(width)
		, height(height)
	{
	}

}; // engi namespace