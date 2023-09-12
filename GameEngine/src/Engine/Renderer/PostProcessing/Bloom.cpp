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
		Ref<ShaderPass> composite = m_BloomShader->GetPass("composite");

		GPUTimer::BeginEvent(commandList, "Down Sample");
		for (uint32 i = 0; i < m_NumberDownSamples; i++)
		{
			GPUTimer::BeginEvent(commandList, std::to_string(i));
			Ref<Texture2D> srcTexture = (i==0) ? src : m_GaussianSumTextures[i - 1];

			commandList->ValidateStates({
				{ m_GaussianSumTextures[i]->GetResource(), ResourceState::RenderTarget },
				{ srcTexture->GetResource(), ResourceState::ShaderResource },
			});

			commandList->SetRenderTarget(m_GaussianSumTextures[i]);
			commandList->ClearRenderTarget(m_GaussianSumTextures[i]);
			commandList->SetShader(downSample);
			commandList->SetRootConstant(downSample->GetUniformLocation("RC_SrcLoc"), srcTexture->GetSRVDescriptor()->GetIndex());
			float threshold = i == 0 ? 1 : 0;
			commandList->SetRootConstant(downSample->GetUniformLocation("RC_Threshold"), threshold);
			commandList->DrawMesh(screenMesh);

			GPUTimer::EndEvent(commandList);
		}

		{ // validate that all sum textures are render targets
			std::vector<ResourceStateObject> transitions(m_GaussianSumTextures.size());
			for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
				transitions[i] = { m_GaussianSumTextures[i]->GetResource(), ResourceState::RenderTarget };
			commandList->ValidateStates(transitions);
		}
		GPUTimer::EndEvent(commandList);

		GPUTimer::BeginEvent(commandList, "Up Sample");
		for (int i = m_NumberDownSamples - 2; i >= 0; i--)
		{
			GPUTimer::BeginEvent(commandList, std::to_string(i));
			Ref<Texture2D> srcTexture = m_GaussianSumTextures[i + 1];

			commandList->ValidateStates({
				{ m_GaussianSumTextures[i]->GetResource(), ResourceState::RenderTarget },
				{ srcTexture->GetResource(), ResourceState::ShaderResource },
			});

			commandList->SetRenderTarget(m_GaussianSumTextures[i]);
			commandList->SetShader(upSample);
			commandList->SetRootConstant(upSample->GetUniformLocation("RC_SrcLoc"), srcTexture->GetSRVDescriptor()->GetIndex());
			commandList->DrawMesh(screenMesh);

			GPUTimer::EndEvent(commandList);
		}

		GPUTimer::EndEvent(commandList);

		GPUTimer::BeginEvent(commandList, "Composite");

		commandList->ValidateStates({
			{ src->GetResource(), ResourceState::ShaderResource },
			{ m_GaussianSumTextures[0]->GetResource(), ResourceState::ShaderResource },
		});

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composite);
		commandList->SetRootConstant(composite->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composite->GetUniformLocation("RC_Bloom"), m_GaussianSumTextures[0]->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		{ // validate that all sum textures are render targets
			std::vector<ResourceStateObject> transitions(m_GaussianSumTextures.size());
			for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
				transitions[i] = { m_GaussianSumTextures[i]->GetResource(), ResourceState::RenderTarget };
			commandList->ValidateStates(transitions);
		}
		GPUTimer::EndEvent(commandList);

		GPUTimer::EndEvent(commandList);

	}

	void Bloom::OnViewportResize(uint32 width, uint32 height)
	{
		m_NumberDownSamples = (uint32)std::floor(std::log2(std::max(width, height)));
		if (m_GaussianSumTextures.size() != m_NumberDownSamples)
		{
			m_GaussianSumTextures.resize(m_NumberDownSamples);
			for (uint32 i = 0; i < m_NumberDownSamples; i++)
			{
				uint32 fac = Math::Pow(2, i + 1);
				uint32 w = width / fac;
				uint32 h = height / fac;

				w = (w == 0) ? 1 : w;
				h = (h == 0) ? 1 : h;

				if (m_GaussianSumTextures[i])
					m_GaussianSumTextures[i]->Resize(w, h);
				else
					m_GaussianSumTextures[i] = RenderTarget2D::Create(w, h, 1, TextureFormat::RGBA16);
			}
		}
	}

}

