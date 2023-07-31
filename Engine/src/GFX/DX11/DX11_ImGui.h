#pragma once

#include "GFX/ImGui.h"

namespace engi::gfx
{

	class DX11ImGuiContext : public IImGuiContext
	{
	public:
		DX11ImGuiContext() = default;
		virtual ~DX11ImGuiContext() = default;

		virtual bool initialize(void* handle, IGpuDevice* device) override;
		virtual void beginFrame() override;
		virtual void endFrame() override;
		virtual void deinitialize() override;
	};

}; // engi::gfx namespace