#include "pch.h"
#include "Model.h"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Game
{
	void Model::BakeMesh()
	{
		CollapseNode(m_Root, Math::Mat4(1));
	}

	void Model::CollapseNode(Node& node, Math::Mat4 parentTransform)
	{
		for (uint32 i = 0; i < node.m_MeshBuilders.size(); i++)
		{
			Engine::MeshBuilder builder(node.m_MeshBuilders[i]);
			for(uint32 i = 0; i < builder.vertices.size(); i++)
				builder.vertices[i].position = parentTransform * Math::Vector4(builder.vertices[i].position, 1);

			builder.Apply();
			m_BakedMeshes.push_back(builder.mesh);
			m_Names.push_back(node.m_Names[i]);
		}

		for (Node& child : node.m_Children)
			CollapseNode(child, parentTransform * node.m_Transform);
	}

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


	void ProcessNode(const aiScene* scene, aiNode* node, Model::Node& model)
	{
		aiMatrix4x4& t = node->mTransformation;
		model.m_Transform = Math::Mat4(
			t.a1, t.b1, t.c1, t.d1,
			t.a2, t.b2, t.c2, t.d2,
			t.a3, t.b3, t.c3, t.d3,
			t.a4, t.b4, t.c4, t.d4
		);

		model.m_MeshBuilders.resize(node->mNumMeshes);
		model.m_Names.resize(node->mNumMeshes);
		for (uint32 i = 0; i < node->mNumMeshes; i++)
		{
			model.m_Names[i] = scene->mMeshes[node->mMeshes[i]]->mName.C_Str();
			Engine::MeshBuilder& meshBuilder = model.m_MeshBuilders[i];
			uint32 vertexOffset = (uint32)meshBuilder.vertices.size();
			aiMesh* m = scene->mMeshes[node->mMeshes[i]];

			meshBuilder.vertices.resize((size_t)vertexOffset + m->mNumVertices);

			// vertices
			for (uint32 v = 0; v < m->mNumVertices; v++)
			{
				meshBuilder.vertices[v].position = GetPosition(m, v);
				meshBuilder.vertices[v].uv = GetUVCoords(m, v);
				meshBuilder.vertices[v].normal = GetNormal(m, v);
				meshBuilder.vertices[v].tangent = GetTangent(m, v);
			}

			// indices
			meshBuilder.indices.resize((size_t)m->mNumFaces * 3);
			for (uint32 j = 0; j < m->mNumFaces; j++)
			{
				for (uint32 k = 0; k < 3; k++)
					meshBuilder.indices[(j * 3) + k] = m->mFaces[j].mIndices[k];
			}

			meshBuilder.Apply();
		}

		model.m_Children.resize(node->mNumChildren);
		for (uint32 i = 0; i < node->mNumChildren; i++)
		{
			Model::Node& child = model.m_Children[i];
			ProcessNode(scene, node->mChildren[i], child);
		}
	}

	Engine::Ref<Model> Model::Create(const fs::path& path)
	{
		Assimp::Importer imp;
		auto scene = imp.ReadFile(path.string(), 0
			| aiProcess_Triangulate
			| aiProcess_GenNormals
			| aiProcess_GenSmoothNormals
			| aiProcess_GenUVCoords
			| aiProcess_CalcTangentSpace
			//| aiProcess_MakeLeftHanded
			//| aiProcess_FlipWindingOrder

			| aiProcess_JoinIdenticalVertices
			| aiProcess_ImproveCacheLocality
			| aiProcess_LimitBoneWeights
			| aiProcess_RemoveRedundantMaterials
			| aiProcess_SplitLargeMeshes
			| aiProcess_SortByPType
			| aiProcess_FindDegenerates
			| aiProcess_FindInvalidData
			| aiProcess_FindInstances
			| aiProcess_ValidateDataStructure
			| aiProcess_OptimizeMeshes
		);

		Engine::Ref<Model> model = Engine::CreateRef<Model>();
		ProcessNode(scene, scene->mRootNode, model->GetRoot());

		model->BakeMesh();

		return model;
	}

	bool Model::ValidExtension(const fs::path& ext)
	{
		return (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb");
	}

}

