#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class Camera;
	class EditorCamera;
	class CPUCommandList;
}

namespace Engine
{
	struct DebugVertex
	{
		Math::Vector4 Position;
		Math::Vector4 Color;
	};

	struct DebugMesh
	{
		Utils::Vector<DebugVertex> m_Vertices;
		Utils::Vector<uint32> m_Indices;
	};

	class DebugRenderer
	{
	public:
		static void Init();
		static void Destroy();

	public:
		
		static void EndScene();
		static void SetCamera(const Camera& camera);

		static void Build(Ref<CPUCommandList> commandList);

	public:
		// line mesh drawing
		static void DrawLine(Math::Vector3 p1, Math::Vector3 p2, Math::Vector4 color, const Math::Mat4& transform = Math::Mat4(1.0f), bool depthTest = true);
		static void DrawLineMesh(DebugMesh& mesh, const Math::Mat4& transform = Math::Mat4(1.0f), bool depthTest = true);

		static void DrawWireBox(const Math::Mat4& transform, Math::Vector4 color, bool depthTest = true);
		static void DrawWireBox(Math::Vector3 center, Math::Vector3 halfExtent, Math::Vector4 color, const Math::Mat4& transform = Math::Mat4(1.0f), bool depthTest = true);

		// solid mesh drawing
		static void DrawMesh(DebugMesh& mesh, const Math::Mat4& transform = Math::Mat4(1.0f), bool depthTest = true);

		static void DrawBox(const Math::Mat4& transform, Math::Vector4 color, bool depthTest = true);
		static void DrawBox(Math::Vector3 center, Math::Vector3 halfExtent, Math::Vector4 color, const Math::Mat4& transform = Math::Mat4(1.0f), bool depthTest = true);


		static void DrawFrustom(const Math::Mat4& vp, Math::Vector4 color, bool depthTest = true);

	private:
		static void DrawMesh(const DebugMesh& mesh, const Math::Mat4& transform, class Pass& pass);

	};
}
