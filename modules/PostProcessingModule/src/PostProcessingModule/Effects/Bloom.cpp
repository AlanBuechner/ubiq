#include "pch.h"
#include "Bloom.h"
#include "Resource.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Core/Application.h"

namespace Game
{
	void Bloom::Init(const PostProcessInput& input)
	{
#ifdef USE_BLOOM_COMPUTE
		m_BloomShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(BLOOMCOMPUTE);
#else
		m_BloomShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(BLOOM);
#endif
	}

	void Bloom::RecordCommands(Engine::Ref<Engine::CPUCommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh)
	{
		CREATE_PROFILE_FUNCTIONI();
		Engine::GPUTimer::BeginEvent(commandList, "Bloom");
		BEGIN_EVENT_TRACE_GPU(commandList, "BLoom");
		Engine::Ref<Engine::GraphicsShaderPass> downSample = m_BloomShader->GetGraphicsPass("downSample");
		Engine::Ref<Engine::GraphicsShaderPass> upSample = m_BloomShader->GetGraphicsPass("upSample");
		Engine::Ref<Engine::GraphicsShaderPass> composite = m_BloomShader->GetGraphicsPass("composite");


		// create sum textures
		Utils::Vector<Engine::Ref<Engine::RenderTarget2D>> gaussianSumTextures;
		uint32 width = renderTarget->GetWidth();
		uint32 height = renderTarget->GetHeight();
		uint32 numberDownSamples = (uint32)std::floor(std::log2(std::max(width, height))) - 1;
		if (gaussianSumTextures.Count() != numberDownSamples)
		{
			gaussianSumTextures.Resize(numberDownSamples);
			for (uint32 i = 0; i < numberDownSamples; i++)
			{
				uint32 fac = Math::Pow<float>(2, i + 1);
				uint32 w = width / fac;
				uint32 h = height / fac;

				w = (w == 0) ? 1 : w;
				h = (h == 0) ? 1 : h;

				gaussianSumTextures[i] = Engine::RenderTarget2D::Create(w, h, Engine::TextureFormat::RGBA16_FLOAT, Engine::ResourceCapabilities::Transient);
				commandList->AllocateTransient(gaussianSumTextures[i]);
			}
		}

		Engine::GPUTimer::BeginEvent(commandList, "Down Sample");
		for (uint32 i = 0; i < numberDownSamples; i++)
		{
			CREATE_PROFILE_SCOPEI("Down Sample");
			ANOTATE_PROFILEI(std::to_string(i));
			Engine::GPUTimer::BeginEvent(commandList, std::to_string(i));
			Engine::Ref<Engine::Texture2D> srcTexture = (i == 0) ? src : gaussianSumTextures[i - 1];

			commandList->ValidateStates({
				{ gaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget },
				{ srcTexture->GetResource(), Engine::ResourceState::ShaderResource },
			});

			commandList->SetRenderTarget(gaussianSumTextures[i]);
			commandList->ClearRenderTarget(gaussianSumTextures[i]);
			commandList->SetShader(downSample);
			commandList->SetTexture("u_Src", srcTexture);
			float threshold = i == 0 ? 1 : 0;
			commandList->SetRootConstant("u_Threshold", threshold);
			commandList->DrawMesh(screenMesh);

			Engine::GPUTimer::EndEvent(commandList);
		}

		{ // validate that all sum textures are render targets
			Utils::Vector<Engine::ResourceStateObject> transitions(gaussianSumTextures.Count());
			for (uint32 i = 0; i < gaussianSumTextures.Count(); i++)
				transitions[i] = { gaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget };
			commandList->ValidateStates(transitions);
		}
		Engine::GPUTimer::EndEvent(commandList);

		Engine::GPUTimer::BeginEvent(commandList, "Up Sample");
		for (int i = numberDownSamples - 2; i >= 0; i--)
		{
			CREATE_PROFILE_SCOPEI("Up Sample");
			ANOTATE_PROFILEI(std::to_string(i));
			Engine::GPUTimer::BeginEvent(commandList, std::to_string(i));
			Engine::Ref<Engine::Texture2D> srcTexture = gaussianSumTextures[i + 1];

			commandList->ValidateStates({
				{ gaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget },
				{ srcTexture->GetResource(), Engine::ResourceState::ShaderResource },
			});

			commandList->SetRenderTarget(gaussianSumTextures[i]);
			commandList->SetShader(upSample);
			commandList->SetTexture("u_Src", srcTexture);
			commandList->DrawMesh(screenMesh);

			Engine::GPUTimer::EndEvent(commandList);
		}

		Engine::GPUTimer::EndEvent(commandList);

		Engine::GPUTimer::BeginEvent(commandList, "Composite");

		commandList->ValidateStates({
			{ src->GetResource(), Engine::ResourceState::ShaderResource },
			{ gaussianSumTextures[0]->GetResource(), Engine::ResourceState::ShaderResource },
		});

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composite);
		commandList->SetTexture("u_Src", src);
		commandList->SetTexture("u_BloomTex", gaussianSumTextures[0]);
		commandList->DrawMesh(screenMesh);

		// close transient resources
		for (uint32 i = 0; i < gaussianSumTextures.Count(); i++)
			commandList->CloseTransient(gaussianSumTextures[i]);
		Engine::GPUTimer::EndEvent(commandList);

		END_EVENT_TRACE_GPU(commandList);
		Engine::GPUTimer::EndEvent(commandList);

	}
}

