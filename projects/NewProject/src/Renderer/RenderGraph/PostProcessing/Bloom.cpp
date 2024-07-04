#include "pch.h"
#include "Bloom.h"
#include "Resource.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Game
{
	void Bloom::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_Scene = &scene;


#ifdef USE_BLOOM_COMPUTE
			m_BloomShader = Engine::Shader::CreateFromEmbeded(BLOOMCOMPUTE, "Bloom_Compute.hlsl");
#else
			m_BloomShader = Engine::Shader::CreateFromEmbeded(BLOOM, "Bloom.hlsl");
#endif
	}

	void Bloom::RecordCommands(Engine::Ref<Engine::CommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh)
	{
		Engine::GPUTimer::BeginEvent(commandList, "Bloom");
		Engine::Ref<Engine::GraphicsShaderPass> downSample = m_BloomShader->GetGraphicsPass("downSample");
		Engine::Ref<Engine::GraphicsShaderPass> upSample = m_BloomShader->GetGraphicsPass("upSample");
		Engine::Ref<Engine::GraphicsShaderPass> composite = m_BloomShader->GetGraphicsPass("composite");

#ifdef USE_BLOOM_COMPUTE
			uint32 computeSize = 8;

			GPUTimer::BeginEvent(commandList, "Down Sample");
			for (uint32 i = 0; i < m_NumberDownSamples; i++)
			{
				GPUTimer::BeginEvent(commandList, std::to_string(i));
				Ref<Texture2D> srcTexture = (i == 0) ? src : m_GaussianSumTextures[i - 1];

				commandList->ValidateStates({
					{ m_GaussianSumTextures[i]->GetResource(), ResourceState::UnorderedResource },
					{ srcTexture->GetResource(), ResourceState::ShaderResource },
					});

				commandList->SetShader(downSample);
				commandList->SetRWTexture(downSample->GetUniformLocation("DstTexture"), m_GaussianSumTextures[i], 0);
				commandList->SetTexture(downSample->GetUniformLocation("src"), srcTexture);
				float threshold = i == 0 ? 1 : 0;
				commandList->SetRootConstant(downSample->GetUniformLocation("RC_Threshold"), threshold);
				commandList->Dispatch(std::max(m_GaussianSumTextures[i]->GetWidth() / computeSize, 1u) + 1, std::max(m_GaussianSumTextures[i]->GetHeight() / computeSize, 1u) + 1, 1);


				GPUTimer::EndEvent(commandList);
			}

			{ // validate that all sum textures are render targets
				std::vector<ResourceStateObject> transitions(m_GaussianSumTextures.size());
				for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
					transitions[i] = { m_GaussianSumTextures[i]->GetResource(), ResourceState::UnorderedResource };
				commandList->ValidateStates(transitions);
			}
			GPUTimer::EndEvent(commandList);

			GPUTimer::BeginEvent(commandList, "Up Sample");
			for (int i = m_NumberDownSamples - 2; i >= 0; i--)
			{
				GPUTimer::BeginEvent(commandList, std::to_string(i));
				Ref<Texture2D> srcTexture = m_GaussianSumTextures[i + 1];

				commandList->ValidateStates({
					{ m_GaussianSumTextures[i]->GetResource(), ResourceState::UnorderedResource },
					{ srcTexture->GetResource(), ResourceState::ShaderResource },
					});


				commandList->SetShader(upSample);
				commandList->SetRWTexture(upSample->GetUniformLocation("DstTexture"), m_GaussianSumTextures[i], 0);
				commandList->SetTexture(upSample->GetUniformLocation("src"), srcTexture);
				commandList->Dispatch(std::max(m_GaussianSumTextures[i]->GetWidth() / computeSize, 1u) + 1, std::max(m_GaussianSumTextures[i]->GetHeight() / computeSize, 1u) + 1, 1);

				GPUTimer::EndEvent(commandList);
			}

			GPUTimer::EndEvent(commandList);

			GPUTimer::BeginEvent(commandList, "Composite");

			commandList->ValidateStates({
				{ src->GetResource(), ResourceState::ShaderResource },
				{ m_GaussianSumTextures[0]->GetResource(), ResourceState::ShaderResource },
				});

			commandList->SetShader(composite);
			commandList->SetRWTexture(composite->GetUniformLocation("DstTexture"), renderTarget->GetRWTexture2D(), 0);
			commandList->SetTexture(composite->GetUniformLocation("src"), src);
			commandList->SetTexture(composite->GetUniformLocation("bloomTex"), m_GaussianSumTextures[0]);
			commandList->Dispatch(std::max(renderTarget->GetWidth() / computeSize, 1u) + 1, std::max(renderTarget->GetHeight() / computeSize, 1u) + 1, 1);

			{ // validate that all sum textures are render targets
				std::vector<ResourceStateObject> transitions(m_GaussianSumTextures.size());
				for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
					transitions[i] = { m_GaussianSumTextures[i]->GetResource(), ResourceState::UnorderedResource };
				commandList->ValidateStates(transitions);
			}
			GPUTimer::EndEvent(commandList);
#else
		Engine::GPUTimer::BeginEvent(commandList, "Down Sample");
			for (uint32 i = 0; i < m_NumberDownSamples; i++)
			{
				Engine::GPUTimer::BeginEvent(commandList, std::to_string(i));
				Engine::Ref<Engine::Texture2D> srcTexture = (i == 0) ? src : m_GaussianSumTextures[i - 1];

				commandList->ValidateStates({
					{ m_GaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget },
					{ srcTexture->GetResource(), Engine::ResourceState::ShaderResource },
				});

				commandList->SetRenderTarget(m_GaussianSumTextures[i]);
				commandList->ClearRenderTarget(m_GaussianSumTextures[i]);
				commandList->SetShader(downSample); 
				commandList->SetTexture(downSample->GetUniformLocation("src"), srcTexture);
				float threshold = i == 0 ? 1 : 0;
				commandList->SetRootConstant(downSample->GetUniformLocation("RC_Threshold"), threshold);
				commandList->DrawMesh(screenMesh);

				Engine::GPUTimer::EndEvent(commandList);
			}

			{ // validate that all sum textures are render targets
				std::vector<Engine::ResourceStateObject> transitions(m_GaussianSumTextures.size());
				for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
					transitions[i] = { m_GaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget };
				commandList->ValidateStates(transitions);
			}
			Engine::GPUTimer::EndEvent(commandList);

			Engine::GPUTimer::BeginEvent(commandList, "Up Sample");
			for (int i = m_NumberDownSamples - 2; i >= 0; i--)
			{
				Engine::GPUTimer::BeginEvent(commandList, std::to_string(i));
				Engine::Ref<Engine::Texture2D> srcTexture = m_GaussianSumTextures[i + 1];

				commandList->ValidateStates({
					{ m_GaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget },
					{ srcTexture->GetResource(), Engine::ResourceState::ShaderResource },
				});

				commandList->SetRenderTarget(m_GaussianSumTextures[i]);
				commandList->SetShader(upSample);
				commandList->SetTexture(upSample->GetUniformLocation("src"), srcTexture);
				commandList->DrawMesh(screenMesh);

				Engine::GPUTimer::EndEvent(commandList);
			}

			Engine::GPUTimer::EndEvent(commandList);

			Engine::GPUTimer::BeginEvent(commandList, "Composite");

			commandList->ValidateStates({
				{ src->GetResource(), Engine::ResourceState::ShaderResource },
				{ m_GaussianSumTextures[0]->GetResource(), Engine::ResourceState::ShaderResource },
			});

			commandList->SetRenderTarget(renderTarget);
			commandList->ClearRenderTarget(renderTarget);
			commandList->SetShader(composite);
			commandList->SetTexture(composite->GetUniformLocation("src"), src);
			commandList->SetTexture(composite->GetUniformLocation("bloomTex"), m_GaussianSumTextures[0]);
			commandList->DrawMesh(screenMesh);

			{ // validate that all sum textures are render targets
				std::vector<Engine::ResourceStateObject> transitions(m_GaussianSumTextures.size());
				for (uint32 i = 0; i < m_GaussianSumTextures.size(); i++)
					transitions[i] = { m_GaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget };
				commandList->ValidateStates(transitions);
			}
			Engine::GPUTimer::EndEvent(commandList);
#endif

			Engine::GPUTimer::EndEvent(commandList);

	}

	void Bloom::OnViewportResize(uint32 width, uint32 height)
	{
		m_NumberDownSamples = (uint32)std::floor(std::log2(std::max(width, height))) - 1;
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
#ifdef USE_BLOOM_COMPUTE
					m_GaussianSumTextures[i] = Engine::RWTexture2D::Create(w, h, 1, Engine::TextureFormat::RGBA16);
#else
					m_GaussianSumTextures[i] = Engine::RenderTarget2D::Create(w, h, Engine::TextureFormat::RGBA16_FLOAT);
#endif
			}
		}
	}

}

