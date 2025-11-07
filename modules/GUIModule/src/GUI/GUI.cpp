#include "GUI.h"
#include "Engine/Renderer/CPUCommandList.h"
#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Renderer/Abstractions/Resources/Buffer.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Core/Application.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Core/Input/KeyCodes.h"
#include "Engine/Core/Input/Input.h"
#include "Resource.h"
#include <cmath>
#include <math.h>

thread_local GUI::Context* GUI::Context::s_CurrentContext = nullptr;

namespace GUI
{
	static Engine::Ref<Engine::Mesh> g_QuadMesh = nullptr;
	static Engine::Ref<Engine::Shader> g_Shader = nullptr;
}


namespace GUI
{

	PanelRef::PanelRef(PanelBase* panel)
	{
		if (panel == nullptr)
			return;

		m_ControleBlock = new PanelControleBlock();
		m_ControleBlock->refCount++;
		m_ControleBlock->panel = panel;
	}

	PanelRef::PanelRef(const PanelRef& other)
	{
		m_ControleBlock = other.m_ControleBlock;
		m_ControleBlock->refCount++;
	}

	PanelRef::~PanelRef()
	{
		if (m_ControleBlock == nullptr)
			return;

		m_ControleBlock->refCount--;
		if (m_ControleBlock->refCount == 0)
			delete m_ControleBlock;

		m_ControleBlock == nullptr;
	}

	







	DrawData::DrawData(Engine::Ref<Engine::GraphicsShaderPass> pass, uint32 dataStride)
	{
		m_ShaderPass = pass;

		m_ShaderData.SetStride(dataStride);
	}



	Context::~Context()
	{
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			delete m_Panels[i];
		m_Panels.Clear();
	}

	void Context::DestroyPanel(PanelBase* panel)
	{
		// clear input lock
		if (m_PanelLockInput == panel)
			ClearInputLock();

		// remove panel
		m_Panels.Remove(m_Panels.Find(panel));

		// delete panel
		delete panel;
	}

	void Context::OnGUIRender()
	{
		SetContext(this);

		for (uint32 i = 0; i < m_Panels.Count(); i++)
		{
			PanelBase* panel = m_Panels[i];
			if(panel->m_Parent != nullptr) continue; // only render root panels
			if (panel->IsFloating()) // only render non floating windows
				continue;

			panel->OnGUIRender();
			PushDepth();
		}

		// render floating windows last 
		for (uint32 i = 0; i < m_Panels.Count(); i++)
		{
			PanelBase* panel = m_Panels[i];
			if (panel->m_Parent != nullptr) continue; // only render root panels
			if (!panel->IsFloating())
				continue;

			panel->OnGUIRender();
			PushDepth();
		}
	}

	void Context::Draw(Engine::Ref<Engine::CPUCommandList> commandList)
	{
		if (m_RenderTarget == nullptr) return;
		if (m_DepthBuffer == nullptr) return;

		commandList->SetRenderTarget(Engine::FrameBuffer::Create({ m_RenderTarget, m_DepthBuffer }));

		for (auto& drawData : m_DrawList)
		{
			DrawData& data = drawData.second;
			if (data.m_ShaderPass == nullptr) continue;
			if (data.m_InstanceData.Count() == 0) continue;

			CORE_ASSERT(data.m_ShaderData.Count() == data.m_InstanceData.Count(), "shader data and instance data buffers not the same length", "");

			// upload data to GPU
			if (data.m_InstanceDataBuffer == nullptr)
				data.m_InstanceDataBuffer = Engine::VertexBuffer::Create(data.m_InstanceData.Count(), data.m_InstanceData.ElementSize());
			else if (data.m_InstanceDataBuffer->GetCount() < data.m_InstanceData.Count())
				data.m_InstanceDataBuffer->Resize(data.m_InstanceData.Count());
			data.m_InstanceDataBuffer->SetData(data.m_InstanceData);

			if (data.m_ShaderDataBuffer == nullptr)
				data.m_ShaderDataBuffer = Engine::VertexBuffer::Create(data.m_ShaderData.Count(), data.m_ShaderData.Stride());
			else if (data.m_ShaderDataBuffer->GetCount() > data.m_ShaderData.Count())
				data.m_ShaderDataBuffer->Resize(data.m_ShaderData.Count());
			data.m_ShaderDataBuffer->SetData(data.m_ShaderData.GetData(), data.m_ShaderData.Count());

			// draw
			commandList->SetShader(data.m_ShaderPass);
			commandList->SetRootConstant("u_AspectRatio", m_RenderTarget->GetAspect());
			commandList->SetRootConstant("u_MaxDepth", m_MaxDepth);
			commandList->SetVertexBuffer(0, g_QuadMesh->GetVertexBuffer(0));
			commandList->SetVertexBuffer(1, data.m_InstanceDataBuffer);
			commandList->SetVertexBuffer(2, data.m_ShaderDataBuffer);
			commandList->SetIndexBuffer(g_QuadMesh->GetIndexBuffer());
			commandList->DrawInstanced(data.m_InstanceDataBuffer->GetCount());
		}
	}

	void Context::EndFrame()
	{
		m_Depth = 0;
		m_MaxDepth = 1;

		for (auto& drawData : m_DrawList)
		{
			DrawData& data = drawData.second;
			data.m_InstanceData.Clear();
			data.m_ShaderData.Clear();
		}
	}

	bool Context::OnEvent(Engine::Event* e)
	{
		// if a panel is locking input only send input to that panel
		if (m_PanelLockInput != nullptr)
			return m_PanelLockInput->OnEvent(e);

		for (uint32 i = m_Panels.Count() - 1; i < m_Panels.Count(); i--)
		{
			PanelBase* panel = m_Panels[i];
			if (!panel->IsContentPanel()) continue; // check if is split panel
			if (!panel->IsFloating()) continue; // send input to floating panels first

			if (panel->OnEvent(e))
				return true;
		}

		for (uint32 i = m_Panels.Count() - 1; i < m_Panels.Count(); i--)
		{
			PanelBase* panel = m_Panels[i];
			if (!panel->IsContentPanel()) continue; // check if is split panel
			if (panel->IsFloating()) continue; // send input to non floating panels second

			if (panel->OnEvent(e))
				return true;
		}

		return false;
	}

	DrawData& Context::GetOrAddDrawData(Engine::Ref<Engine::GraphicsShaderPass> pass, uint32 dataStride)
	{
		auto loc = m_DrawList.find(pass);
		if (loc != m_DrawList.end())
			return loc->second;
		
		auto data = m_DrawList.emplace(pass, DrawData(pass, dataStride));

		return data.first->second;
	}

	void Context::Panel(Math::Vector2 position, Math::Vector2 size, const PanelData& data)
	{
		DrawData::InstanceData instance;
		instance.position = position;
		instance.size = size;
		instance.depth = 0;
	
		AddDraw(g_Shader->GetGraphicsPass("panel"), instance, data);
	}

	void Initalize()
	{
		// create mesh
		struct Vert
		{
			Math::Vector2 pos;
		};
		Utils::Vector<Vert> vertes;
		vertes.Resize(4);
		vertes[0] = { {0,0} };
		vertes[1] = { {1,0} };
		vertes[2] = { {0,1} };
		vertes[3] = { {1,1} };

		Utils::Vector<uint32> indices = {
			0,1,2,
			1,3,2,
		};

		Engine::Ref<Engine::VertexBuffer> vb = Engine::VertexBuffer::Create(4, sizeof(Vert));
		Engine::Ref<Engine::IndexBuffer> ib = Engine::IndexBuffer::Create(indices.Count());
		vb->SetData(vertes);
		ib->SetData(indices);

		g_QuadMesh = Engine::Mesh::Create(vb, ib);

		g_Shader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(GUI_SHADER);
	}

	void Destroy()
	{
		g_QuadMesh = nullptr;
		g_Shader = nullptr;
	}

}
