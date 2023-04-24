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

	void Bloom::RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		GPUTimer::BeginEvent(commandList, "Bloom");

		Ref<ShaderPass> downSample = m_BloomShader->GetPass("downSample");
		Ref<ShaderPass> upSample = m_BloomShader->GetPass("upSample");
		Ref<ShaderPass> composit = m_BloomShader->GetPass("composit");

		uint32 numDownSamples = 6;

		if (m_GaussianSumBuffers.size() != numDownSamples)
		{
			m_GaussianSumBuffers.resize(numDownSamples);
			for (uint32 i = 0; i < numDownSamples; i++)
			{
				uint32 w = renderTarget->GetSpecification().Width;
				uint32 h = renderTarget->GetSpecification().Height;
				uint32 fac = Math::Pow(2, i+1);

				if(m_GaussianSumBuffers[i])
					m_GaussianSumBuffers[i]->Resize(w / fac, h / fac);
				else
				{
					FrameBufferSpecification spec;
					spec.Attachments = {
						{ FrameBufferTextureFormat::RGBA16, {0.1f,0.1f,0.1f,1} },
						{ FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
					};
					spec.InitalState = FrameBufferState::RenderTarget;
					spec.Width = w / fac;
					spec.Height = h / fac;
					m_GaussianSumBuffers[i] = FrameBuffer::Create(spec);
				}
			}
		}

		for (uint32 i = 0; i < numDownSamples; i++)
		{
			uint32 srcLoc = (i==0) ? srcDescriptorLocation : m_GaussianSumBuffers[i - 1]->GetAttachmentShaderDescriptoLocation(0);
			commandList->SetRenderTarget(m_GaussianSumBuffers[i]);
			commandList->ClearRenderTarget(m_GaussianSumBuffers[i]);
			commandList->SetShader(downSample);
			commandList->SetRootConstant(downSample->GetUniformLocation("RC_SrcLoc"), srcLoc);
			float threshold = i == 0 ? 1 : 0;
			commandList->SetRootConstant(downSample->GetUniformLocation("RC_Threshold"), *(uint32*)(float*)&threshold);
			commandList->DrawMesh(screenMesh);
			commandList->Transition({ m_GaussianSumBuffers[i] }, FrameBufferState::SRV, FrameBufferState::RenderTarget);
		}

		{
			std::vector<CommandList::FBTransitionObject> transitions(m_GaussianSumBuffers.size() - 1);
			for (uint32 i = 0; i < m_GaussianSumBuffers.size() - 1; i++) // dont change the last frame buffer
				transitions[i] = { m_GaussianSumBuffers[i], FrameBufferState::RenderTarget, FrameBufferState::SRV };
			commandList->Transition(transitions);
		}


		for (int i = numDownSamples-2; i >= 0; i--)
		{
			uint32 srcLoc = m_GaussianSumBuffers[i + 1]->GetAttachmentShaderDescriptoLocation(0);
			commandList->SetRenderTarget(m_GaussianSumBuffers[i]);
			commandList->SetShader(upSample);
			commandList->SetRootConstant(upSample->GetUniformLocation("RC_SrcLoc"), srcLoc);
			commandList->DrawMesh(screenMesh);
			commandList->Transition({ m_GaussianSumBuffers[i] }, FrameBufferState::SRV, FrameBufferState::RenderTarget);
		}

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composit);
		commandList->SetRootConstant(composit->GetUniformLocation("RC_SrcLoc"), srcDescriptorLocation);
		commandList->SetRootConstant(composit->GetUniformLocation("RC_Bloom"), m_GaussianSumBuffers[0]->GetAttachmentShaderDescriptoLocation(0));
		commandList->DrawMesh(screenMesh);

		{
			std::vector<CommandList::FBTransitionObject> transitions(m_GaussianSumBuffers.size());
			for (uint32 i = 0; i < m_GaussianSumBuffers.size(); i++)
				transitions[i] = { m_GaussianSumBuffers[i], FrameBufferState::RenderTarget, FrameBufferState::SRV };
			commandList->Transition(transitions);
		}

		GPUTimer::EndEvent(commandList);

	}

}

