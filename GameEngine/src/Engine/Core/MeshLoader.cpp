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
		if (mesh->mTangents == nullptr)
			return { 0,0,0 };
		return { mesh->mTangents[index].x, mesh->mTangents[index].y, mesh->mTangents[index].z };
	}

	Math::Vector2 GetUVCoords(aiMesh* mesh, uint32 index, uint32 uvSet = 0)
	{
		if (mesh->mTextureCoords[uvSet] == nullptr)
			return { 0.0f, 0.0f };
		const auto coords = mesh->mTextureCoords[uvSet][index];
		return { coords.x, coords.y };
	}


	void ProcessNode(const aiScene* scene, aiNode* node, Ref<Model> model, const VertexLayout& vertLayout, const BufferLayout& bufferLayout)
	{
		for (uint32 i = 0; i < node->mNumMeshes; i++)
		{
			MeshBuilder meshBuilder;
			uint32 vertexOffset = (uint32)meshBuilder.vertices.size();
			aiMesh* m = scene->mMeshes[node->mMeshes[i]];

			meshBuilder.vertices.resize((size_t)vertexOffset + m->mNumVertices);

			// vertices
			for (uint32 v = 0; v < m->mNumVertices; v++)
			{
				if (vertLayout.HasElement(VertexDataType::Position3))
				{
					meshBuilder.vertices[v].position = GetPosition(m ,v);
				}

				if (vertLayout.HasElement(VertexDataType::UV))
				{
					meshBuilder.vertices[v].uv = GetUVCoords(m, v);
				}

				if (vertLayout.HasElement(VertexDataType::Normal))
				{
					meshBuilder.vertices[v].normal = GetNormal(m, v);
				}

				if (vertLayout.HasElement(VertexDataType::Tangent))
				{
					meshBuilder.vertices[v].tangent = GetTangent(m, v);
				}
			}

			// indices
			meshBuilder.indices.reserve((size_t)m->mNumFaces*3);
			for (uint32 j = 0; j < m->mNumFaces; j++)
			{
				for (uint32 k = 0; k < 3; k++)
					meshBuilder.indices.push_back(m->mFaces[j].mIndices[k]);
			}

			Ref<Mesh> mesh = Mesh::Create(bufferLayout);
			mesh->SetVertices(meshBuilder.vertices.data(), (uint32)meshBuilder.vertices.size());
			mesh->SetIndices(meshBuilder.indices.data(), (uint32)meshBuilder.indices.size());

			model->AddMesh(mesh);
		}

		for (uint32 i = 0; i < node->mNumChildren; i++)
		{
			Ref<Model> child = CreateRef<Model>();
			ProcessNode(scene, node->mChildren[i], child, vertLayout, bufferLayout);
			model->AddChild(child);
		}
	}


	Ref<Model> MeshLoader::LoadModel(const fs::path& path, VertexLayout layout)
	{
		Assimp::Importer imp;
		auto scene = imp.ReadFile(path.string(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			(layout.HasElement(VertexDataType::Normal) ? aiProcess_GenNormals : 0) |
			(layout.HasElement(VertexDataType::Tangent) ? aiProcess_CalcTangentSpace : 0) |
			aiProcess_FlipWindingOrder
		);


		std::vector<BufferElement> elements;
		for (auto l : layout)
		{
			switch (l.type)
			{
			case VertexDataType::Position3:
				elements.push_back({ ShaderDataType::Float3, "Position" });
				break;
			case VertexDataType::Position4:
				elements.push_back({ ShaderDataType::Float4, "Position" });
				break;
			case VertexDataType::Normal:
				elements.push_back({ ShaderDataType::Float3, "Normal" });
				break;
			case VertexDataType::Tangent:
				elements.push_back({ ShaderDataType::Float3, "Tangent" });
				break;
			case VertexDataType::UV:
				elements.push_back({ ShaderDataType::Float2, "UV" });
				break;
			default:
				break;
			}
		}

		BufferLayout bufferLayout(elements);

		Ref<Model> model = CreateRef<Model>();
		ProcessNode(scene, scene->mRootNode, model, layout, bufferLayout);

		return model;
	}
}
