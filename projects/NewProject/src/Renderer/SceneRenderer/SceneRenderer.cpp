#include "pch.h"
#include "SceneRenderer.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "RenderingUtils/Lighting/DirectionalLight.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/Application.h"

// nodes
#include "Renderer/RenderPipline/GBufferPassNode.h"
#include "Renderer/RenderPipline/MainPassNode.h"
#include "Renderer/RenderPipline/SkyboxNode.h"
#include "Renderer/RenderPipline/ShadowPassNode.h"
#include "PostProcessingModule/PostProcessNode.h"

// post process effects
#include "PostProcessingModule/Effects/Bloom.h"
#include "PostProcessingModule/Effects/DepthOfField.h"
#include "PostProcessingModule/Effects/ToneMapping.h"


namespace Game
{

	// scene renderer
	SceneRenderer::SceneRenderer()
	{
		Engine::Window& window = Engine::Application::Get().GetWindow();

		PushNewCommandList();

		float clearval = pow(0.2f, 2.2);
		Engine::MSAASampleCount sampleCount = Engine::MSAASampleCount::MSAA4;
		Engine::Ref<Engine::FrameBufferNode> msaaRenderTargetNode = AddNode<Engine::FrameBufferNode>(Utils::Vector<Engine::Ref<Engine::RenderTarget2D>>{
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::RGBA16_FLOAT, { clearval, clearval, clearval, 1 }, sampleCount),
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::Depth, { 1,0,0,0 }, sampleCount),
		});

		// set frame buffer to render target
		Engine::Ref<Engine::TransitionNode> t1 = AddNode<Engine::TransitionNode>();
		t1->AddBuffer({ msaaRenderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), Engine::ResourceState::RenderTarget });
		t1->AddBuffer({ msaaRenderTargetNode->m_Buffer->GetAttachment(1)->GetResourceHandle(), Engine::ResourceState::RenderTarget });

		// shadow pass
		Engine::Ref<ShadowPassNode> shadowPass = AddNode<ShadowPassNode>();
		shadowPass->SetDirectionalLightBind(&m_DirectinalLight);
		shadowPass->SetDrawCommandsBind(&m_MainPassDrawCommands);

		// gbuffer pass
		Engine::Ref<GBufferPassNode> gBufferPass = AddNode<GBufferPassNode>();
		gBufferPass->SetRenderTarget(msaaRenderTargetNode->m_Buffer);
		gBufferPass->SetCameraBind(&m_MainCamera);
		gBufferPass->SetDrawCommandsBind(&m_MainPassDrawCommands);

		// skybox pass
		Engine::Ref<SkyboxNode> skyboxPass = AddNode<SkyboxNode>();
		skyboxPass->SetRenderTarget(msaaRenderTargetNode->m_Buffer);
		skyboxPass->SetCameraBind(&m_MainCamera);
		skyboxPass->SetSkyboxTextureBind(&m_Skybox);

		// main lit pass
		Engine::Ref<ShaderPassNode> mainPass = AddNode<ShaderPassNode>("lit");
		mainPass->SetRenderTarget(msaaRenderTargetNode->m_Buffer);
		mainPass->SetCameraBind(&m_MainCamera);
		mainPass->SetDrawCommandsBind(&m_MainPassDrawCommands);
		mainPass->SetDirectionalLightBind(&m_DirectinalLight);

		// resolve msaa
		Engine::Ref<Engine::FrameBufferNode> renderTargetNode = AddNode<Engine::FrameBufferNode>(Utils::Vector<Engine::Ref<Engine::RenderTarget2D>>{
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::RGBA16_FLOAT, { clearval, clearval, clearval, 1 }),
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::Depth, { 1,0,0,0 }),
		});

		Engine::Ref<Engine::ResolveMSAANode> resolveNode = AddNode<Engine::ResolveMSAANode>();
		resolveNode->SetDestination(renderTargetNode->m_Buffer);
		resolveNode->SetSource(msaaRenderTargetNode->m_Buffer);

		// create post processing render target
		Engine::Ref<Engine::FrameBufferNode> postRenderTargetNode = AddNode<Engine::FrameBufferNode>(Utils::Vector<Engine::Ref<Engine::RenderTarget2D>>{
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::RGBA16_FLOAT, Engine::ResourceCapabilities::ReadWrite),
		});

		// set frame buffer to srv for use in post processing
		Engine::Ref<Engine::TransitionNode> t2 = AddNode<Engine::TransitionNode>();
		t2->AddBuffer({ postRenderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), Engine::ResourceState::UnorderedResource });
		t2->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), Engine::ResourceState::ShaderResource });
		t2->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(1)->GetResourceHandle(), Engine::ResourceState::ShaderResource });

		// post processing
		PostProcessInput input;
		input.m_TextureHandles["Color Buffer"] = renderTargetNode->m_Buffer->GetAttachment(0);
		input.m_TextureHandles["Depth Buffer"] = renderTargetNode->m_Buffer->GetAttachment(1);

		Engine::Ref<PostProcessNode> postPass = AddNode<PostProcessNode>();
		postPass->SetRenderTarget(postRenderTargetNode->m_Buffer->GetAttachment(0));
		postPass->SetInput(input);
		//postPass->AddPostProcess(CreateRef<DepthOfField>());
		postPass->AddPostProcess(Engine::CreateRef<Bloom>());
		postPass->AddPostProcess(Engine::CreateRef<ToneMapping>("HillACES"));
		postPass->SetSrc(renderTargetNode->m_Buffer->GetAttachment(0));
		postPass->InitPostProcessStack();

		// create outputNode
		m_OutputNode = AddNode<Engine::OutputNode>();
		m_OutputNode->m_Buffer = Engine::FrameBuffer::Create({
			postRenderTargetNode->m_Buffer->GetAttachment(0),
			renderTargetNode->m_Buffer->GetAttachment(1),
		});
	}

	void SceneRenderer::UpdateBuffers()
	{
		m_MainPassObject.UpdateBuffers();
	}

	void SceneRenderer::Build()
	{
		CREATE_PROFILE_SCOPEI("Build Scene Renderer");

		// compile commands
		m_MainPassObject.BuildDrawCommands(m_MainPassDrawCommands);

		BuildCommands();
		Submit();
	}

	Engine::Ref<SceneRenderer> SceneRenderer::Create()
	{
		return Engine::CreateRef<SceneRenderer>();
	}
}

