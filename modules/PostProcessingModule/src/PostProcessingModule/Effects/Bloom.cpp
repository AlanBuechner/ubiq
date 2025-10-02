#include "pch.h"
#include "Bloom.h"
#include "Resource.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Core/Application.h"

namespace Game
{
	void Bloom::Init()
	{
#ifdef USE_BLOOM_COMPUTE
		m_BloomShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(BLOOMCOMPUTE);
#else
		m_BloomShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(BLOOM);
#endif
	}

	void Bloom::RecordCommands(Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src)
	{
		CREATE_PROFILE_FUNCTIONI();
		Engine::GPUTimer::BeginEvent(m_CommandList, "Bloom");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "BLoom");
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
				m_CommandList->AllocateTransient(gaussianSumTextures[i]);
			}
		}

		Engine::GPUTimer::BeginEvent(m_CommandList, "Down Sample");
		for (uint32 i = 0; i < numberDownSamples; i++)
		{
			CREATE_PROFILE_SCOPEI("Down Sample");
			ANOTATE_PROFILEI(std::to_string(i));
			Engine::GPUTimer::BeginEvent(m_CommandList, std::to_string(i));
			Engine::Ref<Engine::Texture2D> srcTexture = (i == 0) ? src : gaussianSumTextures[i - 1];

			m_CommandList->ValidateStates({
				{ gaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget },
				{ srcTexture->GetResource(), Engine::ResourceState::ShaderResource },
			});

			m_CommandList->SetRenderTarget(gaussianSumTextures[i]);
			m_CommandList->ClearRenderTarget(gaussianSumTextures[i]);
			m_CommandList->SetShader(downSample);
			m_CommandList->SetTexture("u_Src", srcTexture);
			float threshold = i == 0 ? 1 : 0;
			m_CommandList->SetRootConstant("u_Threshold", threshold);
			m_CommandList->DrawMesh(m_ScreenMesh);

			Engine::GPUTimer::EndEvent(m_CommandList);
		}

		{ // validate that all sum textures are render targets
			Utils::Vector<Engine::ResourceStateObject> transitions(gaussianSumTextures.Count());
			for (uint32 i = 0; i < gaussianSumTextures.Count(); i++)
				transitions[i] = { gaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget };
			m_CommandList->ValidateStates(transitions);
		}
		Engine::GPUTimer::EndEvent(m_CommandList);

		Engine::GPUTimer::BeginEvent(m_CommandList, "Up Sample");
		for (int i = numberDownSamples - 2; i >= 0; i--)
		{
			CREATE_PROFILE_SCOPEI("Up Sample");
			ANOTATE_PROFILEI(std::to_string(i));
			Engine::GPUTimer::BeginEvent(m_CommandList, std::to_string(i));
			Engine::Ref<Engine::Texture2D> srcTexture = gaussianSumTextures[i + 1];

			m_CommandList->ValidateStates({
				{ gaussianSumTextures[i]->GetResource(), Engine::ResourceState::RenderTarget },
				{ srcTexture->GetResource(), Engine::ResourceState::ShaderResource },
			});

			m_CommandList->SetRenderTarget(gaussianSumTextures[i]);
			m_CommandList->SetShader(upSample);
			m_CommandList->SetTexture("u_Src", srcTexture);
			m_CommandList->DrawMesh(m_ScreenMesh);

			Engine::GPUTimer::EndEvent(m_CommandList);
		}

		Engine::GPUTimer::EndEvent(m_CommandList);

		Engine::GPUTimer::BeginEvent(m_CommandList, "Composite");

		m_CommandList->ValidateStates({
			{ src->GetResource(), Engine::ResourceState::ShaderResource },
			{ gaussianSumTextures[0]->GetResource(), Engine::ResourceState::ShaderResource },
		});

		m_CommandList->SetRenderTarget(renderTarget);
		m_CommandList->ClearRenderTarget(renderTarget);
		m_CommandList->SetShader(composite);
		m_CommandList->SetTexture("u_Src", src);
		m_CommandList->SetTexture("u_BloomTex", gaussianSumTextures[0]);
		m_CommandList->DrawMesh(m_ScreenMesh);

		// close transient resources
		for (uint32 i = 0; i < gaussianSumTextures.Count(); i++)
			m_CommandList->CloseTransient(gaussianSumTextures[i]);
		Engine::GPUTimer::EndEvent(m_CommandList);

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);

	}
}

