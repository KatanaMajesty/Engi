#pragma once

#include <cstdint>
#include "Utility/Memory.h"
#include "Core/WindowEvent.h"
#include "Core/InputEvent.h"
#include "World/Scene.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ModelRegistry.h"
#include "Editor/ResourcePanel.h"
#include "Editor/SceneInspector.h"

namespace engi
{

	class Renderer;
	class ShaderProgram;
	class Texture2D;
	class TextureCube;

	class Editor
	{
	public:
		Editor(Renderer* renderer);
		Editor(const Editor&) = delete;
		Editor& operator=(const Editor&) = delete;
		~Editor();

		bool init(uint32_t width, uint32_t height);
		void onUpdate(float timestep);
		void onResize(const EvWindowResized& ev) noexcept;
		void onKeyPressed(const EvKeyPressed& ev) noexcept;

		ResourcePanel& getResourcePanel() noexcept { return *m_resourcePanel.get(); }
		SceneInspector& getSceneInspector() noexcept { return *m_sceneInspector.get(); }
		Scene& getCurrentScene() noexcept { return *m_scene.get(); } // TODO: Temporary just a getter

	private:
		void drawHeader() noexcept;

		void spawnInstance() noexcept;

		// Queries instance intersection under the cursor
		void queryInstanceIntersection() noexcept;
		void resetInstanceIntersection() noexcept; // should be called at the beginning of each onUpdate
		
		void removeInstanceUnderCursor() noexcept;
		void spawnDecalUnderCursor() noexcept;

		Renderer* m_renderer;
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		float m_headerHeight = 0.0f;
		bool m_isSceneWindowOpened = false;
		bool m_isResourceWindowOpened = false;
		bool m_shouldRenderHelpWindow = true;
		bool m_shouldDrawUI = true;
		
		bool m_hasQueriedInstanceIntersection = false;
		uint32_t m_intersectedInstanceID = uint32_t(-1);
		InstanceIntersection m_queriedIntersection;
		math::Ray m_frameCursorRay;

		UniqueHandle<Scene> m_scene = nullptr;
		UniqueHandle<ResourcePanel> m_resourcePanel = nullptr;
		UniqueHandle<SceneInspector> m_sceneInspector = nullptr;
	};

}; // engi namespace