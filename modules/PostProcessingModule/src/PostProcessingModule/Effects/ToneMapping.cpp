#include "pch.h"
#include "ToneMapping.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Resource.h"
#include "Engine/Core/Application.h"

namespace Game
{

	void ToneMapping::Init(const PostProcessInput& input)
	{
		m_ToneMappingShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(TONEMAPPING);
	}

	void ToneMapping::RecordCommands(Engine::Ref<Engine::CPUCommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh)
	{
		CREATE_PROFILE_FUNCTIONI();
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("None");
		Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("HillACES");
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("NarkowiczACES");
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("Uncharted");

		commandList->ValidateStates({
			{ renderTarget->GetResource(), Engine::ResourceState::RenderTarget },
			{ src->GetResource(), Engine::ResourceState::ShaderResource },
			});

		Engine::GPUTimer::BeginEvent(commandList, "ToneMapping");

		commandList->SetShader(pass);
		commandList->SetRootConstant("u_SrcLoc", src->GetSRVDescriptor()->GetIndex());
		commandList->SetRWTexture("u_DstTexture", renderTarget->GetRWTexture2D(), 0);
		commandList->DispatchThreads(renderTarget->GetWidth(), renderTarget->GetHeight(), 1);

		Engine::GPUTimer::EndEvent(commandList);
	}

}

