#pragma once
#include "glm/glm.hpp"

namespace Engine
{
	class Camera;
	class EditorCamera;
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
		std::vector<LineVertex> m_Vertices;
		std::vector<uint32> m_Indices;

	};

	class LineRenderer
	{
	public:
		static void Init();
		static void ShutDown();

	public:

		static void BeginScene(const Camera& camera, const Math::Mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
		static void Flush();

	private:
		static void BeginBatch();

	public:

		static void DrawLine(Math::Vector3 p1, Math::Vector3 p2, const Math::Vector4 color, const Math::Mat4& transform = Math::Mat4(1.0f));
		static void DrawLineMesh(LineMesh& mesh, const Math::Mat4& transform = Math::Mat4(1.0f));

	};
}
