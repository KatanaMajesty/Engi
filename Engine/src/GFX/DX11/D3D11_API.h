#pragma once

// include the Direct3D Library file
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include <d3d11.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include <wrl/client.h>
#include <cstdint>

#include "GFX/WinAPI.h"

namespace engi::gfx
{

// Only use for non-polymorphic pointers
#define ENGI_ZEROMEM(ptr) ZeroMemory(ptr, sizeof(decltype(*ptr)))

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

}; // engi::gfx namespace