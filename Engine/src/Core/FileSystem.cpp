#include "Core/FileSystem.h"

#include "GFX/WinAPI.h"
#include <array>

#include <iostream>

namespace engi
{
	FileSystem::FileSystem()
	{
		std::array<char, MAX_PATH> rawPath;
		GetModuleFileNameA(nullptr, rawPath.data(), (DWORD)rawPath.size()); // this will always return null-termination character
		m_executablePath = std::string(rawPath.data()); // This std::string constructor will seek for the null-termination character
	}

	FileSystem& FileSystem::getInstance()
	{
		static FileSystem s_instance;
		return s_instance;
		// TODO: insert return statement here
	}
	std::filesystem::path FileSystem::getExecutablePath()
	{
		return std::filesystem::path(m_executablePath).parent_path();
	}

	std::filesystem::path FileSystem::getAssetsPath()
	{
		// TODO: Maybe rewrite?
		return getExecutablePath().parent_path().parent_path() / "Assets";
	}

	std::filesystem::path FileSystem::getShaderPath()
	{
		// TODO: Maybe rewrite?
		return getExecutablePath().parent_path().parent_path() / "Engine" / "src" / "Shaders";
	}

}; // engi namespace