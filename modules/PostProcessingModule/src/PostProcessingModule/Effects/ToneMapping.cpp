#include "pch.h"
#include "ToneMapping.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Resource.h"
#include "Engine/Core/Application.h"

namespace Game
{

	void ToneMapping::Init()
	{
		m_ToneMappingShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(TONEMAPPING);
	}

	void ToneMapping::RecordCommands(Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src)
	{
		CREATE_PROFILE_FUNCTIONI();
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("None");
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("HillACES");
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("NarkowiczACES");
		//Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass("Uncharted");
		Engine::Ref<Engine::ComputeShaderPass> pass = m_ToneMappingShader->GetComputePass(m_ToneMapper);

		m_CommandList->ValidateStates({
			{ renderTarget->GetResource(), Engine::ResourceState::RenderTarget },
			{ src->GetResource(), Engine::ResourceState::ShaderResource },
			});

		Engine::GPUTimer::BeginEvent(m_CommandList, "ToneMapping");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "ToneMapping");

		m_CommandList->SetShader(pass);
		m_CommandList->SetRootConstant("u_SrcLoc", src->GetSRVDescriptor()->GetIndex());
		m_CommandList->SetRWTexture("u_DstTexture", renderTarget->GetRWTexture2D(), 0);
		m_CommandList->DispatchThreads(renderTarget->GetWidth(), renderTarget->GetHeight(), 1);

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);
	}

}

