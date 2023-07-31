#pragma once

#include <string>
#include <cstdint>
#include "Utility/Memory.h"
#include "Core/Window.h"

// TODO: Remove this
#include "GFX/WinAPI.h"

namespace engi
{
	struct ApplicationSpecs
	{
		ApplicationSpecs(const std::string& name = "Unnamed Application");

		std::string name;
	};

	// TODO: Application should not be dependant on WinAPI at all. Everything that is somehow related to WinApi should be moved to Window.h
	struct ApplicationContext
	{
		std::wstring instanceName = L"engi_WindowInstance";
		HINSTANCE instance = nullptr;
		WNDCLASSEXW instanceClass{};
		int32_t cmdshow = 0;
		float lastFrame = 0.0f;
	}; // ApplicationContext struct

	class Editor;
	class Renderer;

	class Application
	{
	private:
		Application(const ApplicationSpecs&, HINSTANCE instance, char* cmdline, int32_t cmdshow);

	public:
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		~Application();

		static inline Application& get() { return *s_instance; }
		
		bool init(const WindowSpecs& winSpecs);
		void createWindow(const WindowSpecs& winSpecs);
		void createRenderer();
		void createEditor();
		int32_t run();
		Window* getWindow() noexcept { return m_window.get(); }
		Renderer* getRenderer() noexcept { return m_renderer.get(); } // TODO: Should be removed
		Editor* getEditor() noexcept { return m_editor.get(); }

	private:
		friend int32_t WINAPI ::WinMain(HINSTANCE, HINSTANCE, LPSTR, int32_t);
		static void create(HINSTANCE instance, char* cmdline, int32_t cmdshow);
		static void destroy();

		void parseMessages(MSG& msg);
		int64_t onMessage(uint32_t message, WPARAM wparam, LPARAM lparam);

		bool initConsole();
		bool initializeWin32Instance(HINSTANCE instance);

		constexpr const std::wstring& getInstanceName() const { return m_context.instanceName; }
		HINSTANCE getWin32Instance() { return m_context.instance; };

		static inline Application* s_instance = nullptr;
		ApplicationSpecs m_specs;
		ApplicationContext m_context;
		UniqueHandle<Window> m_window = nullptr;
		UniqueHandle<Renderer> m_renderer = nullptr;
		UniqueHandle<Editor> m_editor = nullptr;
	};
} // engi namespace
