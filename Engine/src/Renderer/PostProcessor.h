#pragma once

#include <cstdint>
#include "Utility/Memory.h"

namespace engi
{

	namespace gfx { class IGpuTexture; }
	class Renderer;
	class ShaderProgram;
	class Texture2D;
	class Material;
	class ConstantBuffer;

	enum TonemapperType
	{
		ACES,
	};

	struct FXAASpec
	{
		float qualitySubpix = 0.75f;
		float qualityEdgeThreshold = 0.063f;
		float qualityEdgeThresholdMin = 0.0312f;
	};

	class PostProcessor
	{
	public:
		PostProcessor(Renderer* renderer);
		~PostProcessor() = default;

		bool init() noexcept;
		void setTonemapper(TonemapperType type) noexcept { m_type = type; }

		// Resolve from Renderer's HDR target to provided target
		void resolve(Texture2D* target, float ev100, float gamma) noexcept;
		void resolve(gfx::IGpuTexture* target, float ev100, float gamma) noexcept;

		// FXAA is always applied to LDR backbuffer
		void applyFXAA(Texture2D* src, const FXAASpec& fxaaSpec) noexcept;

		inline constexpr bool isFXAAInitted() const noexcept { return m_isFXAAInitted; }

	private:
		Renderer* m_renderer;

		TonemapperType m_type = TonemapperType::ACES;
		SharedHandle<Material> m_tonemapMaterial = nullptr;
		SharedHandle<Material> m_antiAliasingMaterial = nullptr;
		UniqueHandle<ConstantBuffer> m_tonemapData = nullptr;
		UniqueHandle<ConstantBuffer> m_antiAliasingSpec = nullptr;
	
		bool m_isFXAAInitted = true;
	};

}; // engi namespace