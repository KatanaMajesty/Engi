#pragma once

#include <string>
#include "Utility/Memory.h"
#include "Renderer/MaterialInstance.h"
#include "Renderer/ReflectionCapture.h"

namespace engi
{

	namespace gfx 
	{ 
		class IGpuDevice;
	}
	class ShaderProgram;
	class TextureCube;

	class Skybox
	{
	public:
		Skybox(const std::string& name, gfx::IGpuDevice* device);
		~Skybox() = default;

		bool init(ShaderProgram* shaderProgram, ReflectionCapture* reflectionCapture) noexcept;
		void bindIBL() noexcept;
		void setTexture(TextureCube* texture) noexcept;
		void render() noexcept;
		TextureCube* getTexture() noexcept { return m_texture; }

	private:
		gfx::IGpuDevice* m_device;
		std::string m_name;
		ReflectionCapture* m_reflectionCapture = nullptr;
		UniqueHandle<class Material> m_skyboxMat = nullptr;
		TextureCube* m_texture;
	};

}; // engi namespace