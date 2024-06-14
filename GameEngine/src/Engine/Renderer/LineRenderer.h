#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class Camera;
	class EditorCamera;
	class CommandList;
}

namespace Engine
{
	struct LineVertex
	{
		Math::Vector3 Position;
		Math::Vector4 Color;
	};

	struct LineMesh
	{
		Utils::Vector<LineVertex> m_Vertices;
		Utils::Vector<uint32> m_Indices;

	};

	class LineRenderer
	{
	public:
		static void Init();
		static void Destroy();

	public:

		static void BeginScene(const Camera& camera, const Math::Mat4& transform);
		static void BeginScene(Ref<EditorCamera> camera);
		static void EndScene();

		static void Build(Ref<CommandList> commandList);

	public:

		static void DrawLine(Math::Vector3 p1, Math::Vector3 p2, const Math::Vector4 color, const Math::Mat4& transform = Math::Mat4(1.0f));
		static void DrawLineMesh(LineMesh& mesh, const Math::Mat4& transform = Math::Mat4(1.0f));

	};
}
