#include "RuntimeLayer.h"

namespace Game
{

	RuntimeLayer::RuntimeLayer()
	{
	}

	void RuntimeLayer::OnAttach()
	{
		m_Game = CreateGame();
		m_Game->SwitchScene(Engine::Scene::CreateDefault());
	}

	void RuntimeLayer::OnDetach()
	{

	}

	void RuntimeLayer::OnUpdate()
	{
		m_Game->OnUpdate();
	}

	void RuntimeLayer::OnRender()
	{
		CREATE_PROFILE_FUNCTIONI();
		Profiler::InstrumentationTimer timer = CREATE_PROFILEI();
		START_PROFILEI(timer, "Recored Commands");

		m_Game->OnRender();

		Engine::Ref<Engine::CPUCommandList> commandList = Engine::Renderer::GetMainCommandList();

		Engine::Ref<Engine::FrameBuffer> framBuffer = m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget();
		Engine::GPUTimer::BeginEvent(commandList, "gizmo's");
		commandList->SetRenderTarget(framBuffer);
		Engine::Renderer::Build(commandList);
		commandList->Present(framBuffer); // present the render target

		Engine::Ref<Engine::GraphicsShaderPass> blitPass = Engine::Renderer::GetBlitShader()->GetGraphicsPass("BlitRaster");
		Engine::Ref<Engine::RenderTarget2D> screen = Engine::Application::Get().GetWindow().GetSwapChain()->GetCurrentRenderTarget();
		commandList->SetRenderTarget(screen);
		commandList->SetShader(blitPass);
		commandList->SetTexture(blitPass->GetUniformLocation("src"), framBuffer->GetAttachment(0));
		commandList->DrawMesh(Engine::Renderer::GetScreenMesh());
		commandList->Present();
		Engine::GPUTimer::EndEvent(commandList);

		END_PROFILEI(timer);
	}

	void RuntimeLayer::OnImGuiRender()
	{

	}

	void RuntimeLayer::OnEvent(Engine::Event* event)
	{

	}

}

