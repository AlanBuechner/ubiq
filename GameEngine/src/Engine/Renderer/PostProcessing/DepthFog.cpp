#include <pch.h>
#include "DepthFog.h"
#include "EngineResource.h"
#include "Engine/Renderer/Camera.h"
#include <unordered_map>

namespace Engine
{
	struct Params
	{
		uint32 colorBuffer;
		uint32 depthBuffer;
	};

	void DepthFog::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_Scene = &scene;

		m_DepthFogShader = Shader::CreateFromEmbeded(DEPTHFOG, "DepthFog.hlsl");
		m_Params = ConstantBuffer::Create(sizeof(Params));

		Params params;
		params.colorBuffer = input.m_TextureHandles.at("Color Buffer");
		params.depthBuffer = input.m_TextureHandles.at("Depth Buffer");

		m_Params->SetData(&params);
	}

	void DepthFog::RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		Ref<ShaderPass> depthFogPass = m_DepthFogShader->GetPass("depthFog");
		commandList->SetShader(depthFogPass);
		commandList->SetConstantBuffer(depthFogPass->GetUniformLocation("texInput"), m_Params);
		commandList->SetConstantBuffer(depthFogPass->GetUniformLocation("camera"), m_Scene->m_MainCamera->GetCameraBuffer());
		commandList->SetRootConstant(depthFogPass->GetUniformLocation("RC_SrcLoc"), (uint32)srcDescriptorLocation);

		commandList->DrawMesh(screenMesh);
	}

}
