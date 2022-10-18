#pragma once
#include "Core.h"
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
	};

	typedef TMeshBuilder<RendererVertex> MeshBuilder;
}
