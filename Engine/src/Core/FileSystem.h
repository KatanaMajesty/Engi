#pragma once

#include <filesystem>

namespace engi
{

	class FileSystem
	{
	public:
		FileSystem();
		~FileSystem() = default;

		static FileSystem& getInstance();

		std::filesystem::path getExecutablePath();
		std::filesystem::path getAssetsPath();
		std::filesystem::path getShaderPath();

	private:
		std::string m_executablePath;
	};

}; // engi namespace