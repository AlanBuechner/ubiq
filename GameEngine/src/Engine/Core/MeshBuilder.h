#pragma once
#include "Core.h"
#include "Engine/Renderer/Mesh.h"
#include <Engine/Math/Math.h>

#include <vector>

namespace Engine
{
	struct RendererVertex
	{
		Math::Vector3 position;
		Math::Vector2 uv;
		Math::Vector3 normal;
		Math::Vector3 tangent;
	};

	template<class V>
	class TMeshBuilder
	{
	public:
		using Vertex = V;

		std::vector<V> vertices;
		std::vector<uint32> indices;

		Ref<Mesh> ToMesh()
		{
			Ref<Mesh> mesh = Mesh::Create(sizeof(V));
			mesh->SetVertices(vertices.data(), (uint32)vertices.size());
			mesh->SetIndices(indices.data(), (uint32)indices.size());
			return mesh;
		}
	};

	typedef TMeshBuilder<RendererVertex> MeshBuilder;

	MeshBuilder CreateUVSphere(uint32 uSeg, uint32 vSeg);
}
