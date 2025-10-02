#include "pch.h"
#include "DepthOfField.h"
#include "Engine/Renderer/Camera.h"
#include "Resource.h"
#include "Engine/Core/Application.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Game
{

	void DepthOfField::Init()
	{
		m_DepthOfFieldShader = Engine::Application::Get().GetAssetManager().GetEmbededAsset<Engine::Shader>(DEPTHOFFIELD);

		uint32 width = 100;
		uint32 height = 100;

		m_TempTexture	= Engine::RenderTarget2D::Create(width, height, 1, Engine::TextureFormat::RGBA16_FLOAT);
		m_COCTexture	= Engine::RenderTarget2D::Create(width, height, 1, Engine::TextureFormat::RGBA16_FLOAT);
		m_NearBlur		= Engine::RenderTarget2D::Create(width, height, 1, Engine::TextureFormat::RGBA16_FLOAT);
		m_FarBlur		= Engine::RenderTarget2D::Create(width, height, 1, Engine::TextureFormat::RGBA16_FLOAT);

		m_BokehBlur = Engine::FrameBuffer::Create({
			m_NearBlur, m_FarBlur
		});
	}

	void DepthOfField::RecordCommands(Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src)
	{
		Engine::GPUTimer::BeginEvent(m_CommandList, "Depth Of Field");
		BEGIN_EVENT_TRACE_GPU(m_CommandList, "Depth Of Field");

		Engine::Ref<Engine::GraphicsShaderPass> coc = m_DepthOfFieldShader->GetGraphicsPass("CoC");
		Engine::Ref<Engine::GraphicsShaderPass> expandcoc = m_DepthOfFieldShader->GetGraphicsPass("expandCoC");
		Engine::Ref<Engine::GraphicsShaderPass> blurcoc = m_DepthOfFieldShader->GetGraphicsPass("blurCoC");
		Engine::Ref<Engine::GraphicsShaderPass> bokehBlur = m_DepthOfFieldShader->GetGraphicsPass("bokehBlur");
		Engine::Ref<Engine::GraphicsShaderPass> farBokehBlur = m_DepthOfFieldShader->GetGraphicsPass("farBokehBlur");
		Engine::Ref<Engine::GraphicsShaderPass> composit = m_DepthOfFieldShader->GetGraphicsPass("composit");

		Engine::GPUTimer::BeginEvent(m_CommandList, "COC");
		Engine::GPUTimer::BeginEvent(m_CommandList, "Calculate COC");

		m_CommandList->SetRenderTarget(m_COCTexture);
		m_CommandList->ClearRenderTarget(m_COCTexture);
		m_CommandList->SetShader(coc);
		m_CommandList->SetRootConstant(coc->GetUniformLocation("RC_DepthLoc"), m_Input->m_TextureHandles.at("Depth Buffer")->GetSRVDescriptor()->GetIndex());
		m_CommandList->SetRootConstant(coc->GetUniformLocation("RC_Radius"), m_ConfusionRadius);
		m_CommandList->SetRootConstant(coc->GetUniformLocation("RC_FocalPlane"), m_FocalPlane);
		//m_CommandList->SetConstantBuffer(coc->GetUniformLocation("camera"), m_Scene->m_MainCamera->GetCameraBuffer());
		m_CommandList->DrawMesh(m_ScreenMesh);

		Engine::GPUTimer::EndEvent(m_CommandList); // end calc coc

		Engine::GPUTimer::BeginEvent(m_CommandList, "Expand COC");

		m_CommandList->ValidateState({ m_COCTexture->GetResource(), Engine::ResourceState::ShaderResource });

		m_CommandList->SetRenderTarget(m_TempTexture);
		m_CommandList->ClearRenderTarget(m_TempTexture);
		m_CommandList->SetShader(expandcoc);
		m_CommandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		m_CommandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 1u);
		m_CommandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		m_CommandList->DrawMesh(m_ScreenMesh);

		m_CommandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::RenderTarget },
			{ m_TempTexture->GetResource(), Engine::ResourceState::ShaderResource },
		});

		m_CommandList->SetRenderTarget(m_COCTexture);
		m_CommandList->ClearRenderTarget(m_COCTexture);
		m_CommandList->SetShader(expandcoc);
		m_CommandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		m_CommandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 0u);
		m_CommandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetSRVDescriptor()->GetIndex());
		m_CommandList->DrawMesh(m_ScreenMesh);

		m_CommandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::ShaderResource },
			{ m_TempTexture->GetResource(), Engine::ResourceState::RenderTarget },
		});

		Engine::GPUTimer::EndEvent(m_CommandList); // end expand coc

		Engine::GPUTimer::BeginEvent(m_CommandList, "Blur COC");

		m_CommandList->SetRenderTarget(m_TempTexture);
		m_CommandList->ClearRenderTarget(m_TempTexture);
		m_CommandList->SetShader(blurcoc);
		m_CommandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		m_CommandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 1u);
		m_CommandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		m_CommandList->DrawMesh(m_ScreenMesh);

		m_CommandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::RenderTarget },
			{ m_TempTexture->GetResource(), Engine::ResourceState::ShaderResource },
		});

		m_CommandList->SetRenderTarget(m_COCTexture);
		m_CommandList->ClearRenderTarget(m_COCTexture);
		m_CommandList->SetShader(blurcoc);
		m_CommandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		m_CommandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 0u);
		m_CommandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetSRVDescriptor()->GetIndex());
		m_CommandList->DrawMesh(m_ScreenMesh);

		m_CommandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::ShaderResource },
			{ m_TempTexture->GetResource(), Engine::ResourceState::RenderTarget },
		});

		Engine::GPUTimer::EndEvent(m_CommandList); // end blur coc

		Engine::GPUTimer::EndEvent(m_CommandList); // end coc

		Engine::GPUTimer::BeginEvent(m_CommandList, "Bokeh Blur");

		m_CommandList->SetRenderTarget(m_BokehBlur);
		m_CommandList->ClearRenderTarget(m_BokehBlur);
		m_CommandList->SetShader(bokehBlur);
		m_CommandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		m_CommandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_Strength"), m_BokehStrangth);
		m_CommandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_COC"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		m_CommandList->DrawMesh(m_ScreenMesh);

		m_CommandList->ValidateStates({
			{ m_NearBlur->GetResource(), Engine::ResourceState::ShaderResource },
			{ m_FarBlur->GetResource(), Engine::ResourceState::ShaderResource },
		});

		Engine::GPUTimer::EndEvent(m_CommandList); // end bokeh blur

		m_CommandList->SetRenderTarget(renderTarget);
		m_CommandList->ClearRenderTarget(renderTarget);
		m_CommandList->SetShader(composit);
		m_CommandList->SetRootConstant(composit->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		m_CommandList->SetRootConstant(composit->GetUniformLocation("RC_NearLoc"), (uint32)m_NearBlur->GetSRVDescriptor()->GetIndex());
		m_CommandList->SetRootConstant(composit->GetUniformLocation("RC_FarLoc"), (uint32)m_FarBlur->GetSRVDescriptor()->GetIndex());
		m_CommandList->SetRootConstant(composit->GetUniformLocation("RC_COC"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		m_CommandList->DrawMesh(m_ScreenMesh);


		m_CommandList->ValidateStates({
			{m_NearBlur->GetResource(), Engine::ResourceState::RenderTarget},
			{m_FarBlur->GetResource(), Engine::ResourceState::RenderTarget},
		});

		END_EVENT_TRACE_GPU(m_CommandList);
		Engine::GPUTimer::EndEvent(m_CommandList);
	}

	void DepthOfField::OnViewportResize(uint32 width, uint32 height)
	{
		m_COCTexture->Resize(width, height);
		m_TempTexture->Resize(width, height);
		m_NearBlur->Resize(width, height);
		m_FarBlur->Resize(width, height);
	}

}

