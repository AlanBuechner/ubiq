#include "pch.h"
#include "ToneMapping.h"
#include "EngineResource.h"

namespace Engine
{

	void ToneMapping::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_ToneMappingShader = Shader::CreateFromEmbeded(TONEMAPPING, "ToneMapping.hlsl");
	}

	void ToneMapping::RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		Ref<ShaderPass> pass = m_ToneMappingShader->GetPass("HillACES");
		//Ref<ShaderPass> pass = m_ToneMappingShader->GetPass("NarkowiczACES");

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(pass);
		commandList->SetRootConstant(pass->GetUniformLocation("RC_SrcLoc"), (uint32)srcDescriptorLocation);
		commandList->DrawMesh(screenMesh);

	}

}

