#include "Renderer/AssimpUtils.h"

#include "Core/CommonDefinitions.h"

namespace engi::assimp
{

	Assimp::Importer g_importer = Assimp::Importer();

	void convertToVec2(const aiVector3D& srcVec, math::Vec2& dstVec)
	{
		dstVec = math::Vec2(srcVec.x, srcVec.y);
	}

	void convertToVec3(const aiVector3D& srcVec, math::Vec3& dstVec)
	{
		dstVec = math::Vec3(srcVec.x, srcVec.y, srcVec.z);
	}

	void convertToMat4x4(const aiMatrix4x4& srcMat, math::Mat4x4& dstMat)
	{
		dstMat = math::Mat4x4(&srcMat.a1);
	}

	void convertToStaticMesh(const aiMesh* srcMesh, StaticMesh& dstMesh, uint32_t numVertices, uint32_t numFaces, uint32_t numIndices)
	{
		if (!srcMesh)
			return;

		std::string dstMeshName(srcMesh->mName.C_Str());
		math::AABB dstAABB(math::Vec3(&srcMesh->mAABB.mMin.x), math::Vec3(&srcMesh->mAABB.mMax.x));
		uint32_t meshFlags = STATIC_MESH_FLAGS_NONE;
		if (srcMesh->HasNormals())
			meshFlags |= STATIC_MESH_FLAGS_NORMALS;
		if (srcMesh->HasTextureCoords(0))
			meshFlags |= STATIC_MESH_FLAGS_TEX_COORDS;
		if (srcMesh->HasTangentsAndBitangents())
			meshFlags |= STATIC_MESH_FLAGS_TANGENTS;

		dstMesh = StaticMesh(numVertices, numFaces, dstAABB, meshFlags, dstMeshName);
		for (uint32_t v = 0; v < numVertices; ++v)
		{
			StaticMeshVertex vertex;
			convertToVec3(srcMesh->mVertices[v], vertex.position);

			if (dstMesh.hasNormals())
				 convertToVec3(srcMesh->mNormals[v], vertex.normal);

			if (dstMesh.hasTexCoords())
				 convertToVec2(srcMesh->mTextureCoords[0][v], vertex.textureCoords);

			if (dstMesh.hasTangents())
			{
				convertToVec3(srcMesh->mTangents[v], vertex.tangent);
				convertToVec3(srcMesh->mBitangents[v], vertex.bitangent);
			}

			dstMesh.addVertex(vertex);
		}

		ENGI_ASSERT(srcMesh->HasFaces() && "Failed to parse a mesh. Mesh should always contain faces");
		for (uint32_t f = 0; f < numFaces; ++f)
		{
			const aiFace& face = srcMesh->mFaces[f];
			ENGI_ASSERT(face.mNumIndices == 3 && "Failed to parse a mesh. Engi only supportes triangular faces");
			StaticMeshTriangle triangle{};
			triangle.indices[0] = face.mIndices[0];
			triangle.indices[1] = face.mIndices[1];
			triangle.indices[2] = face.mIndices[2];
			dstMesh.addTriangle(triangle);
		}
	}

}; // engi::assimp namespace