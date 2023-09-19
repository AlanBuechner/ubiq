#include "pch.h"
#include "ToneMapping.h"
#include "EngineResource.h"
#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{

	void ToneMapping::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_ToneMappingShader = Shader::CreateFromEmbeded(TONEMAPPING, "ToneMapping.hlsl");
	}

	void ToneMapping::RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		Ref<ShaderPass> pass = m_ToneMappingShader->GetPass("HillACES");
		//Ref<ShaderPass> pass = m_ToneMappingShader->GetPass("NarkowiczACES");
		//Ref<ShaderPass> pass = m_ToneMappingShader->GetPass("Uncharted");

		commandList->ValidateStates({
			{ renderTarget->GetResource(), ResourceState::RenderTarget },
			{ src->GetResource(), ResourceState::ShaderResource },
		});

		GPUTimer::BeginEvent(commandList, "ToneMapping");

		commandList->SetShader(pass);
		commandList->SetRootConstant(pass->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRWTexture(pass->GetUniformLocation("DstTexture"), renderTarget->GetRWTexture2D(), 0);
		commandList->Dispatch(std::max(renderTarget->GetWidth() / 8, 1u) + 1, std::max(renderTarget->GetHeight() / 8, 1u) + 1, 1);

		GPUTimer::EndEvent(commandList);
	}

}

