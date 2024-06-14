#include "RenderGraph.h"

#include "Engine/Core/Window.h"
#include "Engine/Core/Application.h"

#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/CommandQueue.h"
#include "Engine/Renderer/Abstractions/Shader.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/RenderGraphNode.h"

// nodes
#include "GBufferPassNode.h"
#include "MainPassNode.h"
#include "SkyboxNode.h"
#include "ShadowPassNode.h"
#include "PostProcessNode.h"

// post process
#include "PostProcessing/Bloom.h"
#include "PostProcessing/DepthOfField.h"
#include "PostProcessing/ToneMapping.h"

namespace Game
{

	RenderGraph::RenderGraph()
	{
		Engine::Window& window = Engine::Application::Get().GetWindow();

		Engine::Ref<Engine::CommandList> commandList = Engine::CommandList::Create(Engine::CommandList::Direct);;
		m_CommandLists.Push(commandList);

		float clearval = pow(0.2f, 2.2);
		Engine::Ref<Engine::FrameBufferNode> renderTargetNode = Engine::CreateRef<Engine::FrameBufferNode>(*this, Utils::Vector<Engine::Ref<Engine::RenderTarget2D>>{
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::RGBA16_FLOAT, { clearval, clearval, clearval, 1 }),
				Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::Depth, { 1,0,0,0 }),
		});
		m_Nodes.Push(renderTargetNode);

		// set frame buffer to render target
		Engine::Ref<Engine::TransitionNode> t1 = Engine::CreateRef<Engine::TransitionNode>(*this);
		t1->SetCommandList(commandList);
		t1->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), Engine::ResourceState::RenderTarget });
		t1->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(1)->GetResourceHandle(), Engine::ResourceState::RenderTarget });
		m_Nodes.Push(t1);

		// shadow pass
		Engine::Ref<ShadowPassNode> shadowPass = Engine::CreateRef<ShadowPassNode>(*this);
		shadowPass->SetCommandList(commandList);
		m_Nodes.Push(shadowPass);

		// gbuffer pass
		Engine::Ref<GBufferPassNode> gBufferPass = Engine::CreateRef<GBufferPassNode>(*this);
		gBufferPass->SetCommandList(commandList);
		gBufferPass->SetRenderTarget(renderTargetNode->m_Buffer);
		gBufferPass->AddDependincy(t1);
		m_Nodes.Push(gBufferPass);

		// skybox pass
		Engine::Ref<SkyboxNode> skyboxPass = Engine::CreateRef<SkyboxNode>(*this);
		skyboxPass->SetCommandList(commandList);
		skyboxPass->SetRenderTarget(renderTargetNode->m_Buffer);
		skyboxPass->AddDependincy(gBufferPass);
		m_Nodes.Push(skyboxPass);

		// main lit pass
		Engine::Ref<ShaderPassNode> mainPass = Engine::CreateRef<ShaderPassNode>(*this, "lit");
		mainPass->SetCommandList(commandList);
		mainPass->SetRenderTarget(renderTargetNode->m_Buffer);
		mainPass->AddDependincy(shadowPass);
		mainPass->AddDependincy(skyboxPass);
		m_Nodes.Push(mainPass);

		// create post processing render target
		Engine::Ref<Engine::FrameBufferNode> postRenderTargetNode = Engine::CreateRef<Engine::FrameBufferNode>(*this, Utils::Vector<Engine::Ref<Engine::RenderTarget2D>>{
			Engine::RenderTarget2D::Create(window.GetWidth(), window.GetHeight(), Engine::TextureFormat::RGBA16_FLOAT, true),
		});
		m_Nodes.Push(postRenderTargetNode);

		// set frame buffer to srv for use in post processing
		Engine::Ref<Engine::TransitionNode> t2 = Engine::CreateRef<Engine::TransitionNode>(*this);
		t2->SetCommandList(commandList);
		t2->AddBuffer({ postRenderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), Engine::ResourceState::UnorderedResource });
		t2->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(0)->GetResourceHandle(), Engine::ResourceState::ShaderResource });
		t2->AddBuffer({ renderTargetNode->m_Buffer->GetAttachment(1)->GetResourceHandle(), Engine::ResourceState::ShaderResource });
		t2->AddDependincy(mainPass);
		m_Nodes.Push(t2);

		// post processing
		PostProcessInput input;
		input.m_TextureHandles["Color Buffer"] = renderTargetNode->m_Buffer->GetAttachment(0);
		input.m_TextureHandles["Depth Buffer"] = renderTargetNode->m_Buffer->GetAttachment(1);

		Engine::Ref<PostProcessNode> postPass = Engine::CreateRef<PostProcessNode>(*this);
		postPass->SetCommandList(commandList);
		postPass->SetRenderTarget(postRenderTargetNode->m_Buffer->GetAttachment(0));
		postPass->SetInput(input);
		//postPass->AddPostProcess(CreateRef<DepthOfField>());
		postPass->AddPostProcess(Engine::CreateRef<Bloom>());
		postPass->AddPostProcess(Engine::CreateRef<ToneMapping>());
		postPass->SetSrc(renderTargetNode->m_Buffer->GetAttachment(0));
		postPass->InitPostProcessStack();
		postPass->AddDependincy(t2);
		m_Nodes.Push(postPass);

		// create outputNode
		m_OutputNode = Engine::CreateRef<Engine::OutputNode>(*this);
		m_OutputNode->SetCommandList(commandList);
		m_OutputNode->m_Buffer = Engine::FrameBuffer::Create({
			postRenderTargetNode->m_Buffer->GetAttachment(0),
			renderTargetNode->m_Buffer->GetAttachment(1),
		});
		m_Nodes.Push(m_OutputNode);
	}

}

