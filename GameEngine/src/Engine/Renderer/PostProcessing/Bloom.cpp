#include "pch.h"
#include "Bloom.h"
#include "EngineResource.h"

#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{
	void Bloom::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_Scene = &scene;

		m_BloomShader = Shader::CreateFromEmbeded(BLOOM, "Bloom.hlsl");
	}

	void Bloom::RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		GPUTimer::BeginEvent(commandList, "Bloom");

		Ref<ShaderPass> downSample = m_BloomShader->GetPass("downSample");
		Ref<ShaderPass> upSample = m_BloomShader->GetPass("upSample");
		Ref<ShaderPass> composit = m_BloomShader->GetPass("composit");

		for (uint32 i = 0; i < m_NumberDownSamples; i++)
		{
			uint32 srcLoc = (i==0) ? src->GetSRVDescriptor()->GetIndex() : m_GaussianSumTextures[i - 1]->GetSRVDescriptor()->GetIndex();
			commandList->SetRenderTarget(m_GaussianSumTextures[i]);
			commandList->ClearRenderTarget(m_GaussianSumTextures[i]);
			commandList->SetShader(downSample);
			commandList->SetRootConstant(downSample->GetUniformLocation("RC_SrcLoc"), srcLoc);
			float threshold = i == 0 ? 1 : 0;
			commandList->SetRootConstant(downSample->GetUniformLocation("RC_Threshold"), threshold);
			commandList->DrawMesh(screenMesh);
			commandList->ValidateState({m_GaussianSumTextures[i]->GetResource(), ResourceState::ShaderResource});
		}

		{ // validate that all sum textures are render targets
			std::vector<ResourceStateObject> transitions(m_GaussianSumTextures.size());
			for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
				transitions[i] = { m_GaussianSumTextures[i]->GetResource(), ResourceState::RenderTarget };
			commandList->ValidateStates(transitions);
		}


		for (int i = m_NumberDownSamples - 2; i >= 0; i--)
		{
			uint32 srcLoc = m_GaussianSumTextures[i + 1]->GetSRVDescriptor()->GetIndex();
			commandList->SetRenderTarget(m_GaussianSumTextures[i]);
			commandList->DrawMesh(screenMesh);
			commandList->SetShader(upSample);
			commandList->SetRootConstant(upSample->GetUniformLocation("RC_SrcLoc"), srcLoc);
			commandList->ValidateState({ m_GaussianSumTextures[i]->GetResource(), ResourceState::ShaderResource });
		}

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composit);
		commandList->SetRootConstant(composit->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_Bloom"), m_GaussianSumTextures[0]->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		{ // validate that all sum textures are render targets
			std::vector<ResourceStateObject> transitions(m_GaussianSumTextures.size());
			for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
				transitions[i] = { m_GaussianSumTextures[i]->GetResource(), ResourceState::RenderTarget };
			commandList->ValidateStates(transitions);
		}

		GPUTimer::EndEvent(commandList);

	}

	void Bloom::OnViewportResize(uint32 width, uint32 height)
	{

		if (m_GaussianSumTextures.size() != m_NumberDownSamples)
		{
			m_GaussianSumTextures.resize(m_NumberDownSamples);
			for (uint32 i = 0; i < m_NumberDownSamples; i++)
			{
				uint32 w = width;
				uint32 h = height;
				uint32 fac = Math::Pow(2, i + 1);

				if (m_GaussianSumTextures[i])
					m_GaussianSumTextures[i]->Resize(w / fac, h / fac);
				else
					m_GaussianSumTextures[i] = RenderTarget2D::Create(w / fac, h / fac, 1, TextureFormat::RGBA16);
			}
		}
	}

}

