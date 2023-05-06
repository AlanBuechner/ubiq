#include "pch.h"
#include "Model.h"
#include "Engine/Core/MeshLoader.h"

namespace Engine
{

	Ref<Model> Model::Create(const fs::path& path)
	{
		VertexLayout layout = {
			{ VertexDataType::Position3 },
			{ VertexDataType::UV },
			{ VertexDataType::Normal },
			{ VertexDataType::Tangent }
		};


		return MeshLoader::LoadModel(path, layout);
	}

	bool Model::ValidExtension(const fs::path& ext)
	{
		return (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb");
	}

}

