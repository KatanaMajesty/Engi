#include "Renderer/ModelLoader.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Renderer/AssimpUtils.h"

namespace engi
{

	ModelLoader::ModelLoader(TextureLoader* textureLoader, ModelRegistry* modelRegistry, MaterialRegistry* materialRegistry)
		: m_textureLoader(textureLoader)
		, m_modelRegistry(modelRegistry)
		, m_materialRegistry(materialRegistry)
	{
	}

	static void processInstances(aiNode* node, SharedHandle<Model> model)
	{
		ENGI_ASSERT(node && "Node cannot be nullptr");
		ENGI_ASSERT(model && "Model cannot be nullptr");

		math::Mat4x4 nodeToParent;
		assimp::convertToMat4x4(node->mTransformation.Transpose(), nodeToParent);
		math::Mat4x4 parentToNode = nodeToParent.inverse();

		auto& entries = model->getStaticMeshEntries();
		uint32_t numMeshes = node->mNumMeshes;
		for (uint32_t i = 0; i < numMeshes; ++i)
		{
			uint32_t meshIndex = node->mMeshes[i];
			StaticMeshEntry& entry = entries[meshIndex];
			entry.mesh.getMeshToModel() = nodeToParent;
			entry.mesh.getModelToMesh() = parentToNode;
		}

		uint32_t numChildren = node->mNumChildren;
		for (uint32_t child = 0; child < numChildren; ++child)
		{
			processInstances(node->mChildren[child], model);
		}
	}

	static void processTexture(TextureLoader* textureLoader,
		const std::filesystem::path& modelFolder, aiMaterial* meshMaterial, aiTextureType assimpType, TextureType type, MaterialInstance& instance)
	{
		aiString filename;
		if (meshMaterial->GetTexture(assimpType, 0, &filename) == aiReturn_SUCCESS)
		{
			std::string texturepath = (modelFolder / filename.C_Str()).string();
			Texture2D* texture = textureLoader->getLibrary()->getTexture2D(texturepath);
			if (!texture)
			{
				texture = textureLoader->loadTexture2D(texturepath, true);
				if (!texture)
					ENGI_LOG_ERROR("Failed to parse the {} texture", texturepath);
			}
			instance.setTexture(type, texture);
		}
	}

	ParsedModelInfo* ModelLoader::loadFromFBX(const std::string& filepath, const std::string& name, const SharedHandle<Material>& material) noexcept
	{
		ENGI_ASSERT(std::filesystem::path(filepath).extension() == ".fbx");
		// try to find a cached fbx model info
		ParsedModelInfo* cachedInfo = getParsedModel(filepath);
		if (cachedInfo)
			return cachedInfo;

		std::filesystem::path fullpath(filepath);
		std::filesystem::path modelFolder = fullpath.parent_path();
		uint32_t flags = aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace;
		const aiScene* scene = assimp::g_importer.ReadFile(filepath, flags);
		if (!scene)
		{
			ENGI_LOG_ERROR("Assimp model loader failed to parse a model at {}. Aborting", filepath);
			return nullptr;
		}

		ENGI_LOG_INFO("Parsing the {} model", filepath);
		uint32_t numMeshes = scene->mNumMeshes;

		SharedHandle<Model> model = m_modelRegistry->addModel(name, filepath, numMeshes);
		ENGI_ASSERT(model->hasPath());

		std::vector<MaterialInstance> materialInstances;
		materialInstances.reserve(numMeshes);

		uint32_t currentIndexOffset = 0;
		uint32_t currentVertexOffset = 0;
		for (uint32_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
		{
			const aiMesh* srcMesh = scene->mMeshes[meshIndex];
			uint32_t numVertices = srcMesh->mNumVertices;
			uint32_t numFaces = srcMesh->mNumFaces;
			uint32_t numIndices = numFaces * 3; // assert triangles

			MeshRange dstRange;
			dstRange.vboOffset = currentVertexOffset;
			dstRange.iboOffset = currentIndexOffset;
			dstRange.numVertices = numVertices;
			dstRange.numIndices = numIndices;
			currentVertexOffset += numVertices;
			currentIndexOffset += numIndices;

			StaticMesh dstMesh;
			assimp::convertToStaticMesh(srcMesh, dstMesh, numVertices, numFaces, numIndices);
			ENGI_ASSERT(!dstMesh.isEmpty() && dstMesh.isVertexFull() && dstMesh.isTriangleFull() && "Mesh was loaded incorrectly");

			aiMaterial* meshMaterial = scene->mMaterials[srcMesh->mMaterialIndex];
			float roughness = 0.0f;
			float metallic = 0.0f;
			meshMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
			meshMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

			SharedHandle<Material> meshShading = nullptr;
			if (!material)
			{
				int32_t twosided = 0;
				meshMaterial->Get(AI_MATKEY_TWOSIDED, twosided);
				dstMesh.setTwoSided((bool)twosided);
				meshShading = m_materialRegistry->getMaterial(dstMesh.isTwoSided() ? MATERIAL_BRDF_PBR_NO_CULLING : MATERIAL_BRDF_PBR);
			}
			else meshShading = material;

			MaterialInstance currentMaterialInstance(std::string(meshMaterial->GetName().C_Str()), meshShading);
			currentMaterialInstance.setRoughness(roughness);
			currentMaterialInstance.setMetallic(metallic);
			
			processTexture(m_textureLoader, modelFolder, meshMaterial, aiTextureType_DIFFUSE, TEXTURE_ALBEDO, currentMaterialInstance);
			processTexture(m_textureLoader, modelFolder, meshMaterial, aiTextureType_NORMALS, TEXTURE_NORMAL, currentMaterialInstance);
			processTexture(m_textureLoader, modelFolder, meshMaterial, aiTextureType_METALNESS, TEXTURE_METALNESS, currentMaterialInstance);
			processTexture(m_textureLoader, modelFolder, meshMaterial, aiTextureType_SHININESS, TEXTURE_ROUGHNESS, currentMaterialInstance);
			if (currentMaterialInstance.isNormalMapUsed() && !dstMesh.hasTangents())
			{
				ENGI_ASSERT(false && "Failed to correctly load the tangents for normal map");
			}

			materialInstances.push_back(std::move(currentMaterialInstance));
			model->addStaticMeshEntry(StaticMeshEntry(std::move(dstMesh), dstRange));
		}

		processInstances(scene->mRootNode, model);
		if (!model->initialize())
		{
			ENGI_LOG_ERROR("Failed to initialize model {}", filepath);
			m_modelRegistry->removeModel(name);
			return nullptr;
		}

		ParsedModelInfo& info = m_parsedFiles[filepath];
		info.model = model;
		info.materials = std::move(materialInstances);
		return &info;
	}

	ParsedModelInfo* ModelLoader::getParsedModel(const std::string& filepath) noexcept
	{
		auto it = m_parsedFiles.find(filepath);
		return (it == m_parsedFiles.end()) ? nullptr : &it->second;
	}

}; // engi namespace