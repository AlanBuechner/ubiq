#include "pch.h"
#include "ToneMapping.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Resource.h"
#include "Engine/Core/Application.h"

namespace Game
{

	void ToneMapping::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_ToneMappingShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(TONEMAPPING);
	}

	void ToneMapping::RecordCommands(Engine::Ref<Engine::CPUCommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh)
	{
		Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("HillACES");
		//Engine::Ref<Engine::ShaderPass> pass = m_ToneMappingShader->GetPass("NarkowiczACES");
		//Engine::Ref<Engine::ShaderPass> pass = m_ToneMappingShader->GetPass("Uncharted");

		commandList->ValidateStates({
			{ renderTarget->GetResource(), Engine::ResourceState::RenderTarget },
			{ src->GetResource(), Engine::ResourceState::ShaderResource },
		});

		Engine::GPUTimer::BeginEvent(commandList, "ToneMapping");

		commandList->SetShader(pass);
		commandList->SetRootConstant(pass->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRWTexture(pass->GetUniformLocation("DstTexture"), renderTarget->GetRWTexture2D(), 0);
		commandList->DispatchThreads(renderTarget->GetWidth(), renderTarget->GetHeight(), 1);

		Engine::GPUTimer::EndEvent(commandList);
	}

}

