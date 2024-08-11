#include "RuntimeLayer.h"

namespace Engine
{

	RuntimeLayer::RuntimeLayer()
	{
	}

	void RuntimeLayer::OnAttach()
	{
		m_Game = CreateGame();
		m_Game->SwitchScene(Scene::CreateDefault());
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
		InstrumentationTimer timer = CREATE_PROFILEI();
		timer.Start("Recored Commands");

		m_Game->OnRender();

		Ref<CommandList> commandList = Renderer::GetMainCommandList();

		Engine::Ref<Engine::FrameBuffer> framBuffer = m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget();
		Engine::GPUTimer::BeginEvent(commandList, "gizmo's");
		commandList->SetRenderTarget(framBuffer);
		Renderer::Build(commandList);
		commandList->Present(framBuffer); // present the render target

		Engine::Ref<Engine::GraphicsShaderPass> blitPass = Engine::Renderer::GetBlitShader()->GetGraphicsPass("BlitRaster");
		Ref<RenderTarget2D> screen = Application::Get().GetWindow().GetSwapChain()->GetCurrentRenderTarget();
		commandList->SetRenderTarget(screen);
		commandList->SetShader(blitPass);
		commandList->SetTexture(blitPass->GetUniformLocation("src"), framBuffer->GetAttachment(0));
		commandList->DrawMesh(Renderer::GetScreenMesh());
		commandList->Present();
		Engine::GPUTimer::EndEvent(commandList);

		timer.End();
	}

	void RuntimeLayer::OnImGuiRender()
	{

	}

	void RuntimeLayer::OnEvent(Event* event)
	{

	}

}

