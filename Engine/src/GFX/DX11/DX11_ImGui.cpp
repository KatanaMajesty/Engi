#include "GFX/DX11/DX11_ImGui.h"

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include "Core/CommonDefinitions.h"
#include "Core/FileSystem.h"
#include "GFX/DX11/D3D11_Device.h"

#define IMGUI_ENABLE_FREETYPE

namespace engi::gfx
{

	bool DX11ImGuiContext::initialize(void* handle, IGpuDevice* device)
	{
        D3D11Device* dev = dynamic_cast<D3D11Device*>(device);
        ENGI_ASSERT(dev && "Provided device is not suitable for D3D11 backend");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // TODO: Temp
        std::string fontpath = (FileSystem::getInstance().getAssetsPath() / "Fonts" / "SometypeMono-Bold.ttf").string();
        if (!io.Fonts->AddFontFromFileTTF(fontpath.c_str(), 14.0f))
            io.Fonts->AddFontDefault();

        // Setup Platform/Renderer backends
        if (!ImGui_ImplWin32_Init(handle))
            return false;

        if (!ImGui_ImplDX11_Init(dev->getHandle(), dev->getContext()))
            return false;

		return true;
	}

    void DX11ImGuiContext::beginFrame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void DX11ImGuiContext::endFrame()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

	void DX11ImGuiContext::deinitialize()
	{
        // Cleanup
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
	}

}