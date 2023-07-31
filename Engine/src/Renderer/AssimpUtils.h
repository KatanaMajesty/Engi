#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Mat4x4.h"
#include "Renderer/StaticMesh.h"

namespace engi::assimp
{
	extern Assimp::Importer g_importer;

	void convertToVec2(const aiVector3D& srcVec, math::Vec2& dstVec);
	void convertToVec3(const aiVector3D& srcVec, math::Vec3& dstVec);
	void convertToMat4x4(const aiMatrix4x4& srcMat, math::Mat4x4& dstMat);
	void convertToStaticMesh(const aiMesh* srcMesh, StaticMesh& dstMesh, uint32_t numVertices, uint32_t numFaces, uint32_t numIndices);
}