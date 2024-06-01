#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraph.h"

namespace Engine
{
	class CommandList;
	class CommandQueue;
	class Mesh;
	class Material;
	class InstanceBuffer;
	class Camera;
	class EditorCamera;
	class ConstantBuffer;
	class FrameBuffer;
	class RenderGraph;
	class Shader;
	class Texture2D;
}

namespace Engine
{
	class SceneRenderer
	{
	public:

		Ref<FrameBuffer> GetRenderTarget() { return m_RenderGraph->GetRenderTarget(); }
		Ref<Camera> GetMainCamera() { return m_MainCamera; }
		void SetMainCamera(Ref<Camera> camera) { m_MainCamera = camera; }

		virtual void OnViewportResize(uint32 width, uint32 height) {};
		virtual void UpdateBuffers() {};

		virtual void Build() {};
		virtual void Render(Ref<CommandQueue> queue) {};

	protected:
		Ref<Camera> m_MainCamera;
		Ref<RenderGraph> m_RenderGraph;
	};
}
