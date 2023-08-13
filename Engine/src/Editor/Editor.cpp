#include "Editor/Editor.h"

#include <algorithm>
#include <imgui.h>
#include "Math/Math.h"
#include "Utility/Timer.h"
#include "Utility/Random.h"
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Core/Input.h"
#include "Core/EventBus.h"
#include "Renderer/Renderer.h"
#include "Renderer/TextureLibrary.h"
#include "Renderer/MaterialRegistry.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ShaderLibrary.h"
#include "World/ModelInstanceRegistry.h"
#include "World/Camera.h"

extern void ImGui::ShowDemoWindow(bool*);

namespace engi
{

	Editor::Editor(Renderer* renderer)
		: m_renderer(renderer)
	{
		ENGI_ASSERT(renderer && "Renderer cannot be nullptr");
	}

	Editor::~Editor()
	{
		EventBus& bus = EventBus::get();
		if (m_sceneInspector)
			bus.unsubscribeAll(m_sceneInspector.get());
	}

	bool Editor::init(uint32_t width, uint32_t height)
	{
		if (!m_renderer->isImGuiInitialized())
			return false;

		EventBus& bus = EventBus::get();

		m_resourcePanel.reset(new ResourcePanel(m_renderer));

		m_scene.reset(new Scene());
		m_scene->init(m_renderer, width, height);
		m_scene->setDissolutionTime(5.0f);
		m_scene->setIncinerationTime(5.0f);

		m_sceneInspector.reset(new SceneInspector(m_renderer));
		m_sceneInspector->SetActiveScene(m_scene.get());
		m_sceneInspector->SetDissolutionTexture("Dissolution_Noise.dds");
		bus.subscribe(m_sceneInspector.get(), &SceneInspector::OnInstanceSelected);
		return true;
	}

	static bool s_showImGuiDemo = false;

	void Editor::onUpdate(float timestep)
	{
		EngiIO& io = EngiIO::get();
		if (!m_scene->hasCamera())
			return;

		CameraManager* cm = m_scene->getCameraManager();
		Camera& camera = cm->getCamera();

		m_frameCursorRay = camera.castRay(io.getCursorPos());
		this->resetInstanceIntersection();

		if (io.isPressed(Keycode::ALT) && io.isPressed(Keycode::LMB))
		{
			m_scene->isIOBorrowed() = true;
			this->queryInstanceIntersection();
			if (m_queriedIntersection.isValid())
			{
				ENGI_ASSERT(m_intersectedInstanceID != uint32_t(-1));
				m_isSceneWindowOpened = true; // Open SceneInspector on instance selection
			}

			EventBus::get().publish(editor::EvInstanceSelected(m_intersectedInstanceID, m_queriedIntersection, m_frameCursorRay));
		}

		m_scene->onUpdate(timestep, io.isPressed(Keycode::N));

		if (m_shouldDrawUI)
		{
			m_renderer->beginUIFrame();
			{
				drawHeader();
				if (m_isResourceWindowOpened)
				{
					if (ImGui::Begin("Resource Window", &m_isResourceWindowOpened))
						m_resourcePanel->onUpdate();

					ImGui::End();
				}

				if (m_isSceneWindowOpened)
				{
					if (ImGui::Begin("Scene Window", &m_isSceneWindowOpened))
					{
						m_sceneInspector->OnUpdate();
					}
					ImGui::End();
				}

				if (s_showImGuiDemo)
					ImGui::ShowDemoWindow(&s_showImGuiDemo);

				if (m_shouldRenderHelpWindow)
				{
					uint32_t helpWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;
					ImGui::SetNextWindowPos(ImVec2(0.0f, m_headerHeight));
					ImGui::SetNextWindowSize(ImVec2(m_width / 2.0f, m_height / 2.0f));
					if (ImGui::Begin("##help_window", nullptr, helpWindowFlags))
					{
						ImGui::TextWrapped("Press 'ALT + LMB' to Select an Instance (It might seem a bit clanky, sry)\n"
							"Select an \"empty\" space in order to remove instance selection if any\n\n"
							"Press 'M' to spawn an instance\n"
							"Press 'G' while cursor is on instance to spawn a decal on it\n"
							"Press 'Z' while cursor is on instance to remove it from the scene\n"
							"Hold 'N' in order to perform debug render pass\n\n"
							"Press 'H' to toggle this text (Or press 'F1' to toggle all UI)");
					}
					ImGui::End();
				}
			}
			m_renderer->endUIFrame();
		}
	}

	void Editor::onResize(const EvWindowResized& ev) noexcept
	{
		m_width = ev.width;
		m_height = ev.height;

		// TODO: Maybe move this somewhere? But its the best place for now
		m_renderer->resize(ev.width, ev.height);
	}

	void Editor::onKeyPressed(const EvKeyPressed& ev) noexcept
	{
		if (ev.keycode == Keycode::M)
		{
			this->spawnInstance();
		}

		if (ev.keycode == Keycode::G)
		{
			this->spawnDecalUnderCursor();
		}

		if (ev.keycode == Keycode::H)
		{
			ENGI_LOG_INFO("Help window toggled");
			m_shouldRenderHelpWindow = !m_shouldRenderHelpWindow;
		}

		if (ev.keycode == Keycode::Z)
		{
			this->removeInstanceUnderCursor();
		}

		if (ev.keycode == Keycode::F1)
		{
			m_shouldDrawUI = !m_shouldDrawUI;
		}
	}

	void Editor::drawHeader() noexcept
	{

		if (ImGui::BeginMainMenuBar())
		{
			m_headerHeight = ImGui::GetWindowHeight();
			ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Engi Editor");
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::SeparatorText("Windows");
				if (ImGui::MenuItem("Resource Window")) m_isResourceWindowOpened = true;
				if (ImGui::MenuItem("Scene Window")) m_isSceneWindowOpened = true;
				if (ImGui::MenuItem("IMGUI_DEMO")) s_showImGuiDemo = true;
				ImGui::EndMenu();
			}
			ImGuiIO& io = ImGui::GetIO();
			ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "frametime %.3f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::EndMainMenuBar();
		}

	}

	void Editor::spawnInstance() noexcept
	{
		if (m_sceneInspector && m_resourcePanel)
		{
			SharedHandle<Model> selected = m_resourcePanel->getSelectedModel();
			if (!selected)
			{
				ENGI_LOG_WARN("Tried to create an instance of a model, but none was selected. Select a model in resource panel");
				m_isResourceWindowOpened = true;
				return;
			}

			Scene* scene = m_sceneInspector->GetActiveScene();
			if (!scene || !scene->hasCamera())
				return;

			static uint32_t s_InstanceIndex = 1;
			ModelInstance* instance;
			if (selected->hasPath())
			{
				ParsedModelInfo* info = m_renderer->getModelLoader()->getParsedModel(selected->getPath());
				instance = m_sceneInspector->SpawnInstance("SpawnedInstance" + std::to_string(s_InstanceIndex++), info->model, info->materials);
			}
			else
			{
				MaterialRegistry* mr = m_renderer->getMaterialRegistry();
				SharedHandle<Material> dissolutionMaterial = mr->getMaterial(MATERIAL_BRDF_PBR_DISSOLUTION);
				MaterialInstance material("SpawnedInstance_Mat", dissolutionMaterial);
				material.setUseAlbedoTexture(false);
				material.setUseMetalnessMap(false);
				material.setUseNormalMap(false);
				material.setUseRoughnessMap(false);
				material.setRoughness(0.1f);
				material.setMetallic(0.8f);

				std::vector<MaterialInstance> materials(selected->getNumStaticMeshes(), material);
				instance = m_sceneInspector->SpawnInstance("Spawned_Instance" + std::to_string(s_InstanceIndex++), selected, materials, math::Vec3(0.2f, 0.5f, 0.8f));
			}

			if (instance)
			{
				ENGI_LOG_DEBUG("Created an instance of {} model", selected->getName());
			}
			else
			{
				ENGI_LOG_WARN("Failed to create an instance of {} model", selected->getName());
			}
		}
	}

	void Editor::queryInstanceIntersection() noexcept
	{
		if (m_hasQueriedInstanceIntersection)
			return;

		m_hasQueriedInstanceIntersection = true;
		ModelInstanceRegistry* ir = m_scene->getSceneRenderer()->getInstanceRegistry();

		InstanceIntersection result;
		result.meshIntersection.reset();

		uint32_t intersectedModelInstanceID = uint32_t(-1);
		for (uint32_t modelInstanceID : ir->getAllModelInstanceIDs())
		{
			ModelInstance* instance = ir->getModelInstance(modelInstanceID);
			InstanceIntersection i = instance->intersect(m_frameCursorRay);
			if (i < result)
			{
				result = i;
				intersectedModelInstanceID = modelInstanceID;
			}
		}

		m_intersectedInstanceID = intersectedModelInstanceID;
		m_queriedIntersection = result;
	}

	void Editor::resetInstanceIntersection() noexcept
	{
		m_hasQueriedInstanceIntersection = false;
		m_intersectedInstanceID = uint32_t(-1);
		m_queriedIntersection = InstanceIntersection();
	}

	void Editor::removeInstanceUnderCursor() noexcept
	{
		this->queryInstanceIntersection();

		if (m_queriedIntersection.isValid())
		{
			ModelInstance* intersectedInstance = m_scene->getModelInstanceByID(m_intersectedInstanceID);
			ENGI_ASSERT(intersectedInstance);

			m_scene->removeInstance(m_intersectedInstanceID, m_queriedIntersection.meshIntersection.hitpos);
			m_intersectedInstanceID = uint32_t(-1);
		}
	}

	void Editor::spawnDecalUnderCursor() noexcept
	{
		this->queryInstanceIntersection();

		// Query the instance, selected in Scene Inspector. If its null, we have no place to create decal at
		ModelInstance* selectedInstance = m_scene->getModelInstanceByID(m_intersectedInstanceID);
		if (!selectedInstance)
			return;

		const InstanceIntersection& intersection = m_queriedIntersection;
		const math::Ray& intersectionRay = m_frameCursorRay;
		const math::Vec3& pos = intersection.meshIntersection.hitpos;
		math::Vec3 front = intersectionRay.direction;
		front.normalize();

		math::Vec3 right = front.cross(math::Vec3(0.0f, 1.0f, 0.0f));
		math::Vec3 up = right.cross(front);

		math::Mat4x4 transformation = math::Mat4x4::toWorld(pos, right, up, front, math::Vec3(1.0f));

		float rotationZ = Random::GenerateFloat(0.0f, math::Numeric::pi());
		math::Mat4x4 rotation = math::Mat4x4::rotationRPY(0.0f, 0.0f, rotationZ);

		transformation = rotation * transformation;

		math::Vec3 albedo(Random::GenerateFloat3(math::Vec3(), math::Vec3(1.0f)));

		Texture2D* normalMap = m_resourcePanel->GetTexture2D("Decal/Decal_Splatter_512.dds");
		selectedInstance->addDecal(normalMap, albedo, transformation);
		ENGI_LOG_TRACE("Spawned a decal at model instance {}", selectedInstance->getName());
	}

}; // engi namespace