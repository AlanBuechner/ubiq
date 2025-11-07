#pragma once
#include "Engine/Core/Core.h"
#include "Utils/Vector.h"
#include "Utils/DynamicBuffer.h"
#include "Engine/Events/Event.h"

#include "PanelRef.h"
#include "Panel.h"

namespace Engine
{
	class RenderTarget2D;
	class CPUCommandList;
	class VertexBuffer;
	class GraphicsShaderPass;
}

namespace GUI
{
	class Context;
	class PanelBase;
	class SplitPanel;


	class DrawData
	{
	public:
		DrawData(Engine::Ref<Engine::GraphicsShaderPass> pass, uint32 dataStride);

		struct InstanceData
		{
			Math::Vector2 position; // screen space bounds (0-aspect, 0-1)
			Math::Vector2 size;
			uint32 depth;
		};

		template<typename T>
		void AddInstance(const InstanceData& instance, const T& data)
		{
			m_InstanceData.Push(instance);
			m_ShaderData.Push(data);
		}

		Engine::Ref<Engine::GraphicsShaderPass> m_ShaderPass = nullptr;

		Utils::Vector<InstanceData> m_InstanceData;
		Utils::DynamicBuffer m_ShaderData;
		Engine::Ref<Engine::VertexBuffer> m_InstanceDataBuffer = nullptr;
		Engine::Ref<Engine::VertexBuffer> m_ShaderDataBuffer = nullptr;
	};

	class Context
	{
	public:
		static Context* GetCurrentContext() { return s_CurrentContext; }
		static void SetContext(Context* context) { s_CurrentContext = context; }

		static Context* Create();

		~Context();

		template<class T, typename ... Args>
		PanelRef CreatePanel(Args&& ... args)
		{
			T* panel = new T(std::forward<Args>(args)...);
			m_Panels.Push(panel);
			panel->m_Context = this;
			panel->Init();
			return PanelRef(panel);
		}
		void DestroyPanel(PanelBase* panel);

		void PushDepth() { m_Depth++; m_MaxDepth = std::max(m_MaxDepth, m_Depth); }
		void PopDepth() { m_Depth--; }

		void SetWindowSize(uint32 width, uint32 height) { m_Width = width; m_Height = height; }
		void SetRenderTarget(Engine::Ref<Engine::RenderTarget2D> rt) { m_RenderTarget = rt; }
		void SetDepthBuffer(Engine::Ref<Engine::RenderTarget2D> rt) { m_DepthBuffer = rt; }
		void OnGUIRender();
		void Draw(Engine::Ref<Engine::CPUCommandList> commandList);
		void EndFrame();

		void ConvertToGUICoords(Math::Vector2& val) { val = { (val.x / m_Width) * GetAspect(), val.y / m_Height }; }
		bool OnEvent(Engine::Event* e);

		DrawData& GetOrAddDrawData(Engine::Ref<Engine::GraphicsShaderPass> pass, uint32 dataStride);

		template<typename T>
		void AddDraw(Engine::Ref<Engine::GraphicsShaderPass> shader, DrawData::InstanceData instance, const T& data)
		{
			instance.depth = m_Depth;
			DrawData& drawData = GetOrAddDrawData(shader, sizeof(T));
			drawData.AddInstance(instance, data);
		}

		void Panel(Math::Vector2 position, Math::Vector2 size, const PanelData& data);

		float GetAspect() { return (float)m_Width / (float)m_Height; }

		void SetInputLock(PanelBase* panel) { m_PanelLockInput = panel; }
		void ClearInputLock() { m_PanelLockInput = nullptr; }
		PanelBase* GetInputLock() { return m_PanelLockInput; }

	private:

		Engine::Ref<Engine::RenderTarget2D> m_RenderTarget = nullptr;
		Engine::Ref<Engine::RenderTarget2D> m_DepthBuffer = nullptr;

		std::unordered_map<Engine::Ref<Engine::GraphicsShaderPass>, DrawData> m_DrawList;

		Utils::Vector<PanelBase*> m_Panels;
		PanelBase* m_PanelLockInput = nullptr;

		float m_Width = 0;
		float m_Height = 0;

		uint32 m_Depth = 0;
		uint32 m_MaxDepth = 1;

		static thread_local Context* s_CurrentContext;
	};

	void Initalize();
	void Destroy();

	inline Context* GetCurrentContext() { return Context::GetCurrentContext(); }
	inline void SetContext(Context* context) { Context::SetContext(context); }
#define GET_GUI_CONTEXT_CURR GUI::Context* context = GUI::GetCurrentContext(); if(context == nullptr) return;



}
