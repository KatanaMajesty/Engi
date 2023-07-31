#pragma once

#include <string>
#include "Core/FileSystem.h"
#include "Utility/Memory.h"
#include "Renderer/TextureCube.h"
#include "Renderer/Texture2D.h"
#include "Renderer/ConstantBuffer.h"

namespace engi
{

	class Renderer;
	class Material;

	class ReflectionCapture
	{
	public:
		ReflectionCapture(Renderer* renderer);

		bool init(TextureCube* texture) noexcept;
		bool isInitialized() const noexcept { return m_diffuseIBL && m_specularIBL && m_LUT; }
		bool& shouldUseIBL() noexcept { return m_useIBL; }
		
		TextureCube* getDiffuseIBL() noexcept { return m_diffuseIBL; }
		TextureCube* getSpecularIBL() noexcept { return m_specularIBL; }
		Texture2D* getLUT() noexcept { return m_LUT; }

	private:
		TextureCube* loadDiffuseIBL(const std::string& filepath, TextureCube* skybox) noexcept;
		TextureCube* generateDiffuseIBL(const std::string& filepath, TextureCube* skybox) noexcept;
		
		TextureCube* loadSpecularIBL(const std::string& filepath, TextureCube* skybox) noexcept;
		TextureCube* generateSpecularIBL(const std::string& filepath, TextureCube* skybox) noexcept;
		
		Texture2D* loadLUT(const std::string& filepath) noexcept;
		Texture2D* generateLUT(const std::string& filepath) noexcept;

		std::filesystem::path m_iblPath = FileSystem::getInstance().getAssetsPath() / "Textures" / "IBL";
		Renderer* m_renderer;

		TextureCube* m_diffuseIBL = nullptr;
		TextureCube* m_specularIBL = nullptr;
		Texture2D* m_LUT = nullptr;

		SharedHandle<Material> m_diffuseMat;
		SharedHandle<Material> m_specularMat;
		SharedHandle<Material> m_lutMat;
		UniqueHandle<ConstantBuffer> m_specularIrradianceCB;
		bool m_useIBL = true;
	};

}; // engi namespace