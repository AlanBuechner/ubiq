#include "pch.h"
#include "MeshLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Engine
{

	Math::Vector3 GetPosition(aiMesh* mesh, uint32 index)
	{
		return { mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z };
	}

	Math::Vector3 GetNormal(aiMesh* mesh, uint32 index)
	{
		return { mesh->mNormals[index].x, mesh->mNormals[index].y, mesh->mNormals[index].z };
	}

	Math::Vector3 GetTangent(aiMesh* mesh, uint32 index)
	{
		return { mesh->mTangents[index].x, mesh->mTangents[index].y, mesh->mTangents[index].z };
	}

	Math::Vector2 GetUVCoords(aiMesh* mesh, uint32 index)
	{
		if (mesh->mTextureCoords[0] == nullptr)
			return { 0.0f, 0.0f };
		const auto coords = mesh->mTextureCoords[0][index];
		return { coords.x, coords.y };
	}


	void ProcessNode(aiNode* node, const aiScene* scene, Ref<Mesh> mesh, VertexLayout layout)
	{
		for (uint32 i = 0; i < node->mNumMeshes; i++)
		{
			uint32 vertexOffset = (uint32)mesh->vertices.size();
			aiMesh* m = scene->mMeshes[node->mMeshes[i]];

			mesh->vertices.resize((size_t)vertexOffset + m->mNumVertices);

			// vertices
			for (uint32 v = 0; v < m->mNumVertices; v++)
			{
				if (layout.HasElement(VertexDataType::Position3))
				{
					mesh->vertices[v].position = GetPosition(m ,v);
				}

				if (layout.HasElement(VertexDataType::UV))
				{
					mesh->vertices[v].uv = GetUVCoords(m, v);
				}

				if (layout.HasElement(VertexDataType::Normal))
				{
					mesh->vertices[v].normal = GetNormal(m, v);
				}

				if (layout.HasElement(VertexDataType::Tangent))
				{
					mesh->vertices[i].tangent = GetTangent(m, v);
				}
			}

			// indices
			mesh->indices.reserve((size_t)m->mNumFaces*3);
			for (uint32 j = 0; j < m->mNumFaces; j++)
			{
				for (uint32 k = 0; k < 3; k++)
					mesh->indices.push_back(m->mFaces[j].mIndices[k]);
			}

		}

		for (uint32 i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene, mesh, layout);
	}


	Ref<Mesh> MeshLoader::LoadStaticMesh(std::string file, VertexLayout layout)
	{
		Assimp::Importer imp;
		auto model = imp.ReadFile(file,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			(layout.HasElement(VertexDataType::Normal) ? aiProcess_GenNormals : 0) |
			(layout.HasElement(VertexDataType::Tangent) ? aiProcess_CalcTangentSpace : 0) |
			aiProcess_FlipWindingOrder
		);

		Ref<Mesh> mesh = CreateRef<Mesh>();

		ProcessNode(model->mRootNode, model, mesh, layout);

		return mesh;
	}
}
