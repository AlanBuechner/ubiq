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

		GPUTimer::BeginEvent(commandList, "ToneMapping");

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(pass);
		commandList->SetRootConstant(pass->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		GPUTimer::EndEvent(commandList);
	}

}

