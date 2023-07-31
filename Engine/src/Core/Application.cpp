#include "Application.h"

// TODO: Remove this headers
#include <backends/imgui_impl_win32.h>

#include "Core/CommonDefinitions.h"
#include "Core/Input.h"
#include "Core/InputEvent.h"
#include "Core/EventBus.h"
#include "Core/Logger.h"
#include "Editor/Editor.h"
#include "Renderer/Renderer.h"
#include "Utility/Timer.h"

// Is needed to use far in MAKEPOINTS
#include "GFX/WinAPIDef.h"

// TODO: Temporary fix of mouse cursor events not being parsed
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace engi
{
	Application::Application(const ApplicationSpecs& specs, HINSTANCE instance, char* cmdline, int32_t cmdshow)
		: m_specs(specs)
	{
		ENGI_ASSERT(instance && "Application handle cannot be nullptr");
		m_context.cmdshow = cmdshow;
		if (!this->initConsole())
		{
			ENGI_LOG_ERROR("Failed to initialize a console");
			return;
		}
		
		if (!this->initializeWin32Instance(instance))
		{
			// TODO: Add Logger
			ENGI_LOG_ERROR("Failed to initialize win32 instance");
			return;
		}
	}

	Application::~Application()
	{
		EventBus& eventBus = EventBus::get();
		eventBus.unsubscribeAll(m_editor.get());
		eventBus.unsubscribeAll(&EngiIO::get());
	}

	bool Application::init(const WindowSpecs& winSpecs)
	{
		createWindow(winSpecs);
		createRenderer();
		createEditor();
		EventBus& eventBus = EventBus::get();
		eventBus.subscribe(m_editor.get(), &Editor::onResize);
		eventBus.subscribe(m_editor.get(), &Editor::onKeyPressed);
		return true;
	}

	void Application::createWindow(const WindowSpecs& winSpecs)
	{
		ENGI_ASSERT(winSpecs.width > 0 && winSpecs.height > 0 && "Window's width and height should be more than zero");
		m_window = makeUnique<Window>(new Window(getWin32Instance(), getInstanceName(), m_context.cmdshow));
		m_window->initialize(winSpecs);
	}

	void Application::createRenderer()
	{
		ENGI_ASSERT(m_window && "Window must be created before renderer creation");
		m_renderer = makeUnique<Renderer>(new Renderer());
		m_renderer->init(m_window->getHandle(), m_window->getWidth(), m_window->getHeight());
	}

	void Application::createEditor()
	{
		ENGI_ASSERT(m_window && m_renderer && "Window and renderer must be created before editor creation");
		// Editor should be initialized after model manager
		m_editor.reset(new Editor(m_renderer.get()));
		if (!m_editor->init(m_window->getWidth(), m_window->getHeight()))
		{
			ENGI_LOG_ERROR("Failed to init the editor");
			return;
		}
	}

	int32_t Application::run()
	{
		if (!m_window)
		{
			// TODO: Add Logger
			ENGI_LOG_ERROR("Window was not created yet");
			return -1;
		}

		m_window->open();

		MSG message;
		ZeroMemory(&message, sizeof(MSG));

		Timer::reset();
		while (!m_window->shouldClose())
		{
			this->parseMessages(message);

			float time = Timer::getTime();
			float timestep = time - m_context.lastFrame;
			m_context.lastFrame = time;

			m_window->onUpdate(timestep);

			EngiIO& io = EngiIO::get();
			io.onUpdate(m_window->getHandle());
			// TODO: Should be removed, this code is temporal
			if (io.isPressed(Keycode::ESCAPE))
			{
				m_window->close();
			}

			m_renderer->beginFrame();
			m_editor->onUpdate(timestep);
			m_renderer->endFrame();
		}

		return static_cast<int32_t>(message.wParam);
	}

	void Application::parseMessages(MSG& msg)
	{
		// After we processed the WM_DESTROY message, 
		// the handle becomes invalid. So it will not be recognized by PeekMessage() 
		// Always use nullptr for the second argument!
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				// we don't really care about other messages in queue at this point
				m_window->close();
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	int64_t Application::onMessage(uint32_t message, WPARAM wparam, LPARAM lparam)
	{
		// Result variable
		int64_t result = 0;
		Window* win = m_window.get();
		// TODO: This is temporary due to how ImGui works with WndProc function
		// This will be changed when we rewrite our Application class to abstract from WinApi
		if ((result = ImGui_ImplWin32_WndProcHandler(win->getHandle(), message, wparam, lparam)) != 0)
			return result;

		ImGuiIO& imguiIo = ImGui::GetIO();
		bool imguiWantsMouse = imguiIo.WantCaptureMouse;
		bool imguiWantsKeyboard = imguiIo.WantCaptureKeyboard;

		EngiIO& io = EngiIO::get();
		switch (message)
		{
		case WM_SIZE:
		{
			RECT clientRect;
			GetClientRect(m_window->getHandle(), &clientRect);
			int32_t width = clientRect.right - clientRect.left;
			int32_t height = clientRect.bottom - clientRect.top;
			m_window->requestResize(width, height);
		} break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (imguiWantsKeyboard)
				break;

			if (auto it = detail::win32keyRemap.find(wparam); it != detail::win32keyRemap.end())
			{
				Keycode keycode = (Keycode)it->second;
				EventBus::get().publish(EvKeyPressed(win, keycode));
				io.getState(keycode).pressed = true;
			}
		}; break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (imguiWantsKeyboard)
				break;
			
			if (auto it = detail::win32keyRemap.find(wparam); it != detail::win32keyRemap.end())
			{
				Keycode keycode = (Keycode)it->second;
				EventBus::get().publish(EvKeyReleased(win, keycode));
				io.getState(keycode).pressed = false;
			}
		}; break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			if (imguiWantsMouse)
				break;

			bool pressed = false;
			Keycode keycode = NUMBER_OF_KEYCODES;
			if (message == WM_LBUTTONDOWN) { keycode = Keycode::LMB; pressed = true; }
			if (message == WM_LBUTTONUP) { keycode = Keycode::LMB; pressed = false; }
			if (message == WM_RBUTTONDOWN) { keycode = Keycode::RMB; pressed = true; }
			if (message == WM_RBUTTONUP) { keycode = Keycode::RMB; pressed = false; }

			ENGI_ASSERT(keycode != NUMBER_OF_KEYCODES);
			EventBus& bus = EventBus::get();
			(pressed ? bus.publish(EvKeyPressed(win, keycode)) : bus.publish(EvKeyReleased(win, keycode)));
			io.getState(keycode).pressed = pressed;
		}; break;
		case WM_MOUSEWHEEL:
		{
			// we will use delta param for events
			// A position of cursor at the moment of scroll
			POINTS point = MAKEPOINTS(wparam);
			math::Vec2 scrollDelta(static_cast<float>(point.x) / WHEEL_DELTA, static_cast<float>(point.y) / WHEEL_DELTA);
			io.getMouse().addScrollDelta(scrollDelta);
		}; break;
		case WM_DESTROY: PostQuitMessage(0); break;
		default:
		{
			result = DefWindowProc(m_window->getHandle(), message, wparam, lparam);
		}; break;
		}

		return result;
	}

	void Application::create(HINSTANCE instance, char* cmdline, int32_t cmdshow)
	{
		ApplicationSpecs specs("Engi Application"); // TODO: This is currently immutable
		s_instance = new Application(specs, instance, cmdline, cmdshow);
		bool r = EventBus::init();
		ENGI_ASSERT(r && "Failed to create event bus");
	}

	void Application::destroy()
	{
		delete s_instance;
		s_instance = nullptr;
		EventBus::deinit();
	}

	bool Application::initConsole()
	{
		if (!AllocConsole())
		{
			return false;
		}
		FILE* dummy;
		freopen_s(&dummy, "CONOUT$", "w", stdout); // stdout will print to the newly created console
		return (dummy != nullptr); // If error occured in freopen_s then null will be written to dummy
	}

	bool Application::initializeWin32Instance(HINSTANCE instance)
	{
		m_context.instance = instance;

		auto& wc = m_context.instanceClass;
		ZeroMemory(&wc, sizeof(WNDCLASSEXW));

		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.hInstance = instance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.cbWndExtra = sizeof(LONG_PTR); // TODO: Only supports 1 window now
		wc.lpszClassName = Application::getInstanceName().c_str();
		wc.lpfnWndProc = [](HWND handle, uint32_t message, WPARAM wparam, LPARAM lparam) -> LRESULT
		{
			Application* app = reinterpret_cast<Application*>(GetWindowLongPtrW(handle, 0));
			return app ? (LRESULT)app->onMessage(message, wparam, lparam) : DefWindowProc(handle, message, wparam, lparam);
		};

		if (!RegisterClassExW(&wc))
		{
			// TODO: Add Logger
			ENGI_LOG_ERROR("Failed to init the window class");
			return false;
		}

		return true;
	}

	ApplicationSpecs::ApplicationSpecs(const std::string& name)
		: name(name)
	{
	}
}

