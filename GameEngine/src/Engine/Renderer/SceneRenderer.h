#pragma once
#include "Engine/Core/Core.h"
#include "RenderPiplineNode.h"

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

	class RenderPiplineNode;
}

namespace Engine
{
	class SceneRenderer
	{
	public:

		Ref<FrameBuffer> GetRenderTarget() { return m_OutputNode->m_Buffer; }
		Ref<Camera> GetMainCamera() { return m_MainCamera; }
		void SetMainCamera(Ref<Camera> camera) { m_MainCamera = camera; }
		void AddCamera(Ref<Camera> camera) { m_Cameras.Push(camera); }
		void RemoveCamera(Ref<Camera> camera) { m_Cameras.Remove(m_Cameras.Find(camera)); }
		const Utils::Vector<Ref<Camera>>& GetCameras() { return m_Cameras; }

		virtual void OnViewportResize(uint32 width, uint32 height);
		virtual void UpdateBuffers() {};

		virtual void Build() {};

		void BuildCommands();

	protected:

		void PushNewCommandList() { m_CommandLists.Push(CPUCommandList::Create(CommandListType::Graphics)); }
		Ref<CPUCommandList> GetCommandList() { return m_CommandLists.Back(); }

		template<class T, typename ... Args>
		Ref<T> AddNode(Args&& ... args)
		{
			Ref<T> node = CreateRef<T>(std::forward<Args>(args)...);
			node->SetCommandList(GetCommandList());
			m_Nodes.Push(node);
			return node;
		}

		void Submit();


	protected:
		Ref<Camera> m_MainCamera;
		Utils::Vector<Ref<Camera>> m_Cameras;
		Utils::Vector<Ref<RenderPiplineNode>> m_Nodes;
		Ref<OutputNode> m_OutputNode;

		Utils::Vector<Ref<CPUCommandList>> m_CommandLists;
	};
}
