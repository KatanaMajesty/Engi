#include "Editor/ResourcePanel.h"

#include <imgui.h>
#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Core/EventBus.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/TextureLoader.h"
#include "Renderer/ShaderLibrary.h"
#include "Editor/EditorEvent.h"

// TODO: Remove this header
#include "Renderer/ReflectionCapture.h"

namespace engi
{

	ResourcePanel::ResourcePanel(Renderer* renderer)
		: m_renderer(renderer)
	{
	}

	void ResourcePanel::onUpdate() noexcept
	{
		ImGui::SeparatorText("Resource Panel");

		ImGuiTabBarFlags tabFlags = ImGuiTabBarFlags_Reorderable;
		if (ImGui::BeginTabBar("##resource_panel", tabFlags))
		{
			ImGuiTabItemFlags itemFlags = 0;
			if (ImGui::BeginTabItem("Models", nullptr, itemFlags))
			{
				drawAllModels();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Materials", nullptr, itemFlags))
			{
				drawAllMaterials();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Shaders", nullptr, itemFlags))
			{
				drawAllShaders();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::SeparatorText("Resource Inspector");

		tabFlags = ImGuiTabBarFlags_Reorderable;
		if (ImGui::BeginTabBar("##resource_inspector", tabFlags))
		{
			ImGuiTabItemFlags itemFlags = 0;
			if (ImGui::BeginTabItem("Model", nullptr, itemFlags))
			{
				drawSelectedModel();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Material", nullptr, itemFlags))
			{
				drawSelectedMaterial();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Shader", nullptr, itemFlags))
			{
				drawSelectedProgram();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	void ResourcePanel::drawAllModels() noexcept
	{
		ModelRegistry* modelRegistry = m_renderer->getModelRegistry();
		const auto& models = modelRegistry->getAllModels();
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;
		if (ImGui::BeginTable("##model_table", 2, tableFlags))
		{
			ImGui::TableSetupColumn("Model name");
			ImGui::TableSetupColumn("Mesh count");
			ImGui::TableHeadersRow();
			for (const auto& [modelname, model] : models)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				bool isSelected = (m_selectedModel == model);
				if (ImGui::Selectable(modelname.c_str(), isSelected))
					m_selectedModel = model;

				ImGui::TableNextColumn();
				ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "%d", model->getNumStaticMeshes());
			}
			ImGui::EndTable();
		}
	}

	void ResourcePanel::drawAllMaterials() noexcept
	{
		MaterialRegistry* materialRegistry = m_renderer->getMaterialRegistry();
		const auto& materials = materialRegistry->getAllMaterials();
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;
		if (ImGui::BeginTable("##material_table", 2, tableFlags))
		{
			ImGui::TableSetupColumn("Material name");
			ImGui::TableSetupColumn("Program name");
			ImGui::TableHeadersRow();
			for (const auto& [materialName, material] : materials)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				bool isSelected = (m_selectedMaterial == material);
				if (ImGui::Selectable(materialName.c_str(), isSelected))
					m_selectedMaterial = material;

				if (isSelected)
					ImGui::SetItemDefaultFocus();

				ImGui::TableNextColumn();
				const char* shadername = material->getShader()->getName().c_str();
				ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "%s", shadername);
			}
			ImGui::EndTable();
		}
	}

	void ResourcePanel::drawAllShaders() noexcept
	{
		const auto& shaderPrograms = m_renderer->getShaderLibrary()->getAllShaders();
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable;
		if (ImGui::BeginTable("##shader_table", 3, tableFlags))
		{
			ImGui::TableSetupColumn("Program name");
			ImGui::TableSetupColumn("Geometry stage");
			ImGui::TableSetupColumn("Tesselation stage");
			ImGui::TableHeadersRow();
			for (const auto& [shadername, program] : shaderPrograms)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				bool isSelected = (m_selectedProgram == program.get());
				if (ImGui::Selectable(shadername.c_str(), isSelected))
					m_selectedProgram = program.get();

				if (isSelected)
					ImGui::SetItemDefaultFocus();

				ImGui::TableNextColumn();
				(program->hasGeometryStage() ? 
					ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "Enabled") :
					ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.3f, 1.0f), "Disabled"));

				ImGui::TableNextColumn();
				(program->hasTesselationStage() ?
					ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "Enabled") :
					ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.3f, 1.0f), "Disabled"));
			}
			ImGui::EndTable();
		}
	}

	void ResourcePanel::drawSelectedModel() noexcept
	{
		if (!m_selectedModel)
		{
			ImGui::Text("No model was selected");
			return;
		}

		ImGui::Text("Model name is: %s", m_selectedModel->getName().c_str());
		if (m_selectedModel->hasPath())
			ImGui::Text("Model's path is %s", m_selectedModel->getPath().c_str());
	}

	void ResourcePanel::drawSelectedMaterial() noexcept
	{
		if (!m_selectedMaterial)
		{
			ImGui::Text("No material was selected");
			return;
		}

		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Material name: %s", m_selectedMaterial->getName().c_str());
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Program name: %s", m_selectedMaterial->getShader()->getName().c_str());
		ImGui::Checkbox("Render wireframe", &m_selectedMaterial->getRasterizerState().wireframe);
		//ImGui::Checkbox("Backface culling", &m_selectedMaterial->getRasterizerState().cullBackFaces);
		ImGui::Checkbox("Depth testing", &m_selectedMaterial->getDepthStencilState().depthEnabled);
		ImGui::Checkbox("Mutable z-buffer", &m_selectedMaterial->getDepthStencilState().depthWritable);

		if (ImGui::SmallButton("Reinit"))
		{
			m_selectedMaterial->init();
		}
	}

	void ResourcePanel::drawSelectedProgram() noexcept
	{
		if (!m_selectedProgram)
		{
			ImGui::Text("No shader program was selected");
			return;
		}

		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.9f, 1.0f), "Program name: %s", m_selectedProgram->getName().c_str());
		if (ImGui::SmallButton("Hot Reload"))
		{
			m_selectedProgram->recompileAll();
		}
	}

	ParsedModelInfo* ResourcePanel::LoadFromFBX(const std::string& filename, const SharedHandle<Material>& material) noexcept
	{
		ModelLoader* modelLoader = m_renderer->getModelLoader();
		std::string filepath = (m_modelsPath / filename).string();
		return modelLoader->loadFromFBX(filepath, filename, material);
	}

	SharedHandle<Material> ResourcePanel::GetMaterial(MaterialType type) noexcept
	{
		return m_renderer->getMaterialRegistry()->getMaterial(type);
	}

	Texture2D* ResourcePanel::GetTexture2D(const std::string& filename) noexcept
	{
		std::string filepath = (m_texturePath / filename).string();
		return m_renderer->getTextureLoader()->loadTexture2D(filepath, true);
	}

	Texture2D* ResourcePanel::GetTextureAtlas(const std::string& filename, uint32_t numWidth, uint32_t numHeight) noexcept
	{
		std::string filepath = (m_texturePath / filename).string();
		return m_renderer->getTextureLoader()->loadTextureAtlas(filepath, numWidth, numHeight, true);
	}

	TextureCube* ResourcePanel::GetTextureCube(const std::string& filename) noexcept
	{
		std::string filepath = (m_texturePath / filename).string();
		return m_renderer->getTextureLoader()->loadTextureCube(filepath, true);
	}

}; // engi namespace