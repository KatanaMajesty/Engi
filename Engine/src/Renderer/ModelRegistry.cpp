#include "Renderer/ModelRegistry.h"

#include "Core/Logger.h"
#include "Core/CommonDefinitions.h"
#include "Core/FileSystem.h"
#include "Renderer/AssimpUtils.h"
#include "Renderer/Renderer.h"

namespace engi
{

	ModelRegistry::ModelRegistry(gfx::IGpuDevice* device) 
		: m_device(device)
	{
		ENGI_ASSERT(device && "Logical device cannot be nullptr");
	}

	ModelRegistry::~ModelRegistry()
	{

		ENGI_LOG_INFO("Destroying model registry");
		bool successfullyDestroyed = true;
		for (auto& pair : m_loadedModels)
		{
			const SharedHandle<Model>& model = pair.second;
			if (model.use_count() > 1)
			{
				successfullyDestroyed = false;
				ENGI_LOG_WARN("Failed to destroy {} model as it is used somewhere else. Skipping", model->getPath());
			}
		}

		if (successfullyDestroyed)
			ENGI_LOG_INFO("Successfully destroyed model registry");
		else ENGI_LOG_ERROR("Failed to successfully destroy model registry");
	}

	bool loadCube(ModelRegistry* modelRegistry);
	bool loadSphere(ModelRegistry* modelRegistry);
	bool ModelRegistry::init() noexcept
	{
		if (!loadCube(this)) return false;
		if (!loadSphere(this)) return false;
		return true;
	}

	SharedHandle<Model> ModelRegistry::addModel(const std::string& name, uint32_t numMeshes) noexcept
	{
		SharedHandle<Model> model = getModel(name);
		if (model)
			return model;

		model = makeShared<Model>(new Model(name, m_device, numMeshes));
		m_loadedModels[name] = model;
		return model;
	}

	SharedHandle<Model> ModelRegistry::addModel(const std::string& name, const std::string& filepath, uint32_t numMeshes) noexcept
	{
		SharedHandle<Model> model = addModel(name, numMeshes);
		model->setPath(filepath);
		return model;
	}

	static std::string g_defaultModelNames[] =
	{
		"ENGI_Cube",
		"ENGI_Sphere",
	};

	SharedHandle<Model> ModelRegistry::addModel(ModelType type, uint32_t numMeshes) noexcept
	{
		std::string name = g_defaultModelNames[type];
		return addModel(name, numMeshes);
	}

	SharedHandle<Model> ModelRegistry::getModel(const std::string& name) noexcept
	{
		auto it = m_loadedModels.find(name);
		return (it == m_loadedModels.end()) ? nullptr : it->second;
	}

	SharedHandle<Model> ModelRegistry::getModel(ModelType type) noexcept
	{
		return getModel(g_defaultModelNames[type]);
	}

	bool ModelRegistry::removeModel(const std::string& filepath) noexcept
	{
		return m_loadedModels.erase(filepath) == 1;
	}

	bool ModelRegistry::removeModel(ModelType type) noexcept
	{
		return removeModel(g_defaultModelNames[type]);
	}

	static bool loadCube(ModelRegistry* modelRegistry)
	{
		static constexpr std::array<math::Vec3, 8> positions =
		{
			math::Vec3(-0.5f, -0.5f, -0.5f), // 0
			math::Vec3(-0.5f,  0.5f, -0.5f), // 1
			math::Vec3(0.5f,  0.5f, -0.5f), // 2
			math::Vec3(0.5f, -0.5f, -0.5f), // 3
			math::Vec3(0.5f,  0.5f,  0.5f), // 4
			math::Vec3(0.5f, -0.5f,  0.5f), // 5
			math::Vec3(-0.5f, -0.5f,  0.5f), // 6
			math::Vec3(-0.5f,  0.5f,  0.5f), // 7
		};

		static constexpr std::array<math::Vec3, 6> normals =
		{
			math::Vec3(0.0f,  0.0f, -1.0f), // -Z
			math::Vec3(-1.0f,  0.0f,  0.0f), // -X
			math::Vec3(0.0f,  0.0f,  1.0f), // +Z
			math::Vec3(1.0f,  0.0f,  0.0f), // +X
			math::Vec3(0.0f,  1.0f,  0.0f), // +Y
			math::Vec3(0.0f, -1.0f,  0.0f), // -Y
		};

		static constexpr std::array<math::Vec2, 4> texuvs =
		{
			math::Vec2(0.0f, 1.0f),
			math::Vec2(0.0f, 0.0f),
			math::Vec2(1.0f, 0.0f),
			math::Vec2(1.0f, 1.0f),
		};

		static constexpr uint32_t numMeshes = 1;
		static constexpr uint32_t numVertices = 24;
		static constexpr uint32_t numTriangles = 12;
		static constexpr uint32_t numIndices = 36;
		std::array<StaticMeshVertex, numVertices> vertices =
		{
			// -Z side
			StaticMeshVertex{ positions[0], normals[0], texuvs[0] }, // 0
			StaticMeshVertex{ positions[1], normals[0], texuvs[1] }, // 1
			StaticMeshVertex{ positions[2], normals[0], texuvs[2] }, // 2
			StaticMeshVertex{ positions[3], normals[0], texuvs[3] }, // 3
			// -X side
			StaticMeshVertex{ positions[6], normals[1], texuvs[0] }, // 4
			StaticMeshVertex{ positions[7], normals[1], texuvs[1] }, // 5
			StaticMeshVertex{ positions[1], normals[1], texuvs[2] }, // 6
			StaticMeshVertex{ positions[0], normals[1], texuvs[3] }, // 7
			// +Z side
			StaticMeshVertex{ positions[5], normals[2], texuvs[0] }, // 8
			StaticMeshVertex{ positions[4], normals[2], texuvs[1] }, // 9
			StaticMeshVertex{ positions[7], normals[2], texuvs[2] }, // 10
			StaticMeshVertex{ positions[6], normals[2], texuvs[3] }, // 11
			// +X side
			StaticMeshVertex{ positions[3], normals[3], texuvs[0] }, // 12
			StaticMeshVertex{ positions[2], normals[3], texuvs[1] }, // 13
			StaticMeshVertex{ positions[4], normals[3], texuvs[2] }, // 14
			StaticMeshVertex{ positions[5], normals[3], texuvs[3] }, // 15
			// +Y side
			StaticMeshVertex{ positions[1], normals[4], texuvs[0] }, // 16
			StaticMeshVertex{ positions[7], normals[4], texuvs[1] }, // 17
			StaticMeshVertex{ positions[4], normals[4], texuvs[2] }, // 18
			StaticMeshVertex{ positions[2], normals[4], texuvs[3] }, // 19
			// -Y side
			StaticMeshVertex{ positions[5], normals[5], texuvs[0] }, // 20
			StaticMeshVertex{ positions[6], normals[5], texuvs[1] }, // 21
			StaticMeshVertex{ positions[0], normals[5], texuvs[2] }, // 22
			StaticMeshVertex{ positions[3], normals[5], texuvs[3] }, // 23
		};

		static constexpr std::array<uint32_t, numIndices> indices =
		{
			0, 1, 2, 2, 3, 0, // -Z
			4, 5, 6, 6, 7, 4, // -X
			8, 9, 10, 10, 11, 8, // +Z
			12, 13, 14, 14, 15, 12, // +X
			16, 17, 18, 18, 19, 16, // +Y
			20, 21, 22, 22, 23, 20, // -Y
		};

		uint8_t meshFlags = STATIC_MESH_FLAGS_NORMALS | STATIC_MESH_FLAGS_TEX_COORDS | STATIC_MESH_FLAGS_TANGENTS;
		StaticMesh cubeMesh(numVertices, numTriangles, math::AABB(math::Vec3(-0.5f), math::Vec3(0.5f)), meshFlags, "UnitCube_mesh");

		// do this before vertices
		for (uint32_t triIndex = 0; triIndex < numTriangles; ++triIndex)
		{
			StaticMeshTriangle triangle;
			triangle.indices[0] = indices[triIndex * 3 + 0];
			triangle.indices[1] = indices[triIndex * 3 + 1];
			triangle.indices[2] = indices[triIndex * 3 + 2];
			cubeMesh.addTriangle(triangle);

			using namespace math;
			Vec3 pos1 = vertices[triangle.indices[0]].position;
			Vec3 pos2 = vertices[triangle.indices[1]].position;
			Vec3 pos3 = vertices[triangle.indices[2]].position;
			Vec2 uv1 = vertices[triangle.indices[0]].textureCoords;
			Vec2 uv2 = vertices[triangle.indices[1]].textureCoords;
			Vec2 uv3 = vertices[triangle.indices[2]].textureCoords;

			Vec3 edge1 = pos2 - pos1;
			Vec3 edge2 = pos3 - pos1;
			Vec2 duv1 = uv2 - uv1;
			Vec2 duv2 = uv3 - uv1;

			float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);
			Vec3 tangent;
			tangent.x = f * (duv2.y * edge1.x - duv1.y * edge2.x);
			tangent.y = f * (duv2.y * edge1.y - duv1.y * edge2.y);
			tangent.z = f * (duv2.y * edge1.z - duv1.y * edge2.z);

			Vec3 bitangent;
			bitangent.x = f * (-duv2.x * edge1.x + duv1.x * edge2.x);
			bitangent.y = f * (-duv2.x * edge1.y + duv1.x * edge2.y);
			bitangent.z = f * (-duv2.x * edge1.z + duv1.x * edge2.z);

			for (uint32_t i = 0; i < 3; ++i)
			{
				vertices[triangle.indices[i]].tangent = tangent;
				vertices[triangle.indices[i]].bitangent = bitangent;
			}
		}

		for (uint32_t vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
			cubeMesh.addVertex(vertices[vertexIndex]);

		cubeMesh.getMeshToModel() = math::Mat4x4();
		cubeMesh.getModelToMesh() = math::Mat4x4();
		ENGI_ASSERT(!cubeMesh.isEmpty()
			&& cubeMesh.isVertexFull()
			&& cubeMesh.isTriangleFull()
			&& "Failed to initialize cube mesh (somehow)");

		SharedHandle<Model> model = modelRegistry->addModel(MODEL_TYPE_CUBE, numMeshes);

		MeshRange range;
		range.vboOffset = 0;
		range.iboOffset = 0;
		range.numVertices = numVertices;
		range.numIndices = numIndices;
		model->addStaticMeshEntry(StaticMeshEntry(std::move(cubeMesh), range));
		if (!model->initialize())
		{
			[[maybe_unused]] bool r = modelRegistry->removeModel(MODEL_TYPE_CUBE);
			ENGI_ASSERT(r);
			return false;
		}
		return true;
	}

	static bool loadSphere(ModelRegistry* modelRegistry)
	{
		static constexpr uint32_t sides = 6;
		static constexpr uint32_t gridSize = 12;
		static constexpr uint32_t sideTriangles = gridSize * gridSize * 2;
		static constexpr uint32_t sideVertices = (gridSize + 1) * (gridSize + 1);

		static constexpr uint32_t sideMasks[sides][3] =
		{
			{ 2, 1, 0 },
			{ 0, 1, 2 },
			{ 2, 1, 0 },
			{ 0, 1, 2 },
			{ 0, 2, 1 },
			{ 0, 2, 1 }
		};

		static constexpr float sideSigns[sides][3] =
		{
			{ +1.0f, +1.0f, +1.0f },
			{ -1.0f, +1.0f, +1.0f },
			{ -1.0f, +1.0f, -1.0f },
			{ +1.0f, +1.0f, -1.0f },
			{ +1.0f, -1.0f, -1.0f },
			{ +1.0f, +1.0f, +1.0f }
		};

		static constexpr uint32_t meshFlags = STATIC_MESH_FLAGS_NORMALS;
		static constexpr uint32_t numVertices = sides * sideVertices;
		static constexpr uint32_t numTriangles = sides * sideTriangles;
		static constexpr uint32_t numIndices = numTriangles * 3;
		StaticMesh sphereMesh(numVertices, numTriangles, math::AABB(), meshFlags, "UnitSphere_mesh");
		sphereMesh.getMeshToModel() = math::Mat4x4();
		sphereMesh.getModelToMesh() = math::Mat4x4();
		for (uint32_t side = 0; side < sides; ++side)
		{
			for (uint32_t row = 0; row < gridSize + 1; ++row)
			{
				for (uint32_t col = 0; col < gridSize + 1; ++col)
				{
					float g = static_cast<float>(gridSize);
					math::Vec3 v;
					v.x = col / g * 2.0f - 1.0f;
					v.y = row / g * 2.0f - 1.0f;
					v.z = 1.0f;

					StaticMeshVertex vertex;
					vertex.position[sideMasks[side][0]] = v.x * sideSigns[side][0];
					vertex.position[sideMasks[side][1]] = v.y * sideSigns[side][1];
					vertex.position[sideMasks[side][2]] = v.z * sideSigns[side][2];
					vertex.position.normalize();
					vertex.normal = vertex.position;

					sphereMesh.addVertex(vertex);
				}
			}
		}

		for (uint32_t side = 0; side < sides; ++side)
		{
			uint32_t sideOffset = sideVertices * side;
			for (uint32_t row = 0; row < gridSize; ++row)
			{
				for (uint32_t col = 0; col < gridSize; ++col)
				{
					StaticMeshTriangle triangle0;
					triangle0.indices[0] = sideOffset + (row + 0) * (gridSize + 1) + col + 0;
					triangle0.indices[1] = sideOffset + (row + 1) * (gridSize + 1) + col + 0;
					triangle0.indices[2] = sideOffset + (row + 0) * (gridSize + 1) + col + 1;
					sphereMesh.addTriangle(triangle0);

					StaticMeshTriangle triangle1;
					triangle1.indices[0] = sideOffset + (row + 1) * (gridSize + 1) + col + 0;
					triangle1.indices[1] = sideOffset + (row + 1) * (gridSize + 1) + col + 1;
					triangle1.indices[2] = sideOffset + (row + 0) * (gridSize + 1) + col + 1;
					sphereMesh.addTriangle(triangle1);
				}
			}
		}
		ENGI_ASSERT(!sphereMesh.isEmpty()
			&& sphereMesh.isVertexFull()
			&& sphereMesh.isTriangleFull()
			&& "Failed to initialize sphere mesh (somehow)");

		SharedHandle<Model> model = modelRegistry->addModel(MODEL_TYPE_SPHERE, 1);
		MeshRange range;
		range.vboOffset = 0;
		range.iboOffset = 0;
		range.numVertices = numVertices;
		range.numIndices = numIndices;
		model->addStaticMeshEntry(StaticMeshEntry(std::move(sphereMesh), range));
		if (!model->initialize())
		{
			[[maybe_unused]] bool r = modelRegistry->removeModel(MODEL_TYPE_SPHERE);
			ENGI_ASSERT(r);
			return false;
		}
		return true;
	}

}; // engi namespace