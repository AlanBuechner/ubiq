#include "pch.h"
#include "DepthOfField.h"
#include "Engine/Renderer/Camera.h"
#include "Resource.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Game
{

	void DepthOfField::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_DepthOfFieldShader = Engine::Shader::CreateFromEmbeded(DEPTHOFFIELD, "DepthOfField.hlsl");

		m_Scene = &scene;

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

	void DepthOfField::RecordCommands(Engine::Ref<Engine::CommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh)
	{
		Engine::GPUTimer::BeginEvent(commandList, "Depth Of Field");

		Engine::Ref<Engine::ShaderPass> coc = m_DepthOfFieldShader->GetPass("CoC");
		Engine::Ref<Engine::ShaderPass> expandcoc = m_DepthOfFieldShader->GetPass("expandCoC");
		Engine::Ref<Engine::ShaderPass> blurcoc = m_DepthOfFieldShader->GetPass("blurCoC");
		Engine::Ref<Engine::ShaderPass> bokehBlur = m_DepthOfFieldShader->GetPass("bokehBlur");
		Engine::Ref<Engine::ShaderPass> farBokehBlur = m_DepthOfFieldShader->GetPass("farBokehBlur");
		Engine::Ref<Engine::ShaderPass> composit = m_DepthOfFieldShader->GetPass("composit");

		Engine::GPUTimer::BeginEvent(commandList, "COC");
		Engine::GPUTimer::BeginEvent(commandList, "Calculate COC");

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(coc);
		commandList->SetRootConstant(coc->GetUniformLocation("RC_DepthLoc"), input.m_TextureHandles.at("Depth Buffer")->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(coc->GetUniformLocation("RC_Radius"), m_ConfusionRadius);
		commandList->SetRootConstant(coc->GetUniformLocation("RC_FocalPlane"), m_FocalPlane);
		commandList->SetConstantBuffer(coc->GetUniformLocation("camera"), m_Scene->m_MainCamera->GetCameraBuffer());
		commandList->DrawMesh(screenMesh);

		Engine::GPUTimer::EndEvent(commandList); // end calc coc

		Engine::GPUTimer::BeginEvent(commandList, "Expand COC");

		commandList->ValidateState({ m_COCTexture->GetResource(), Engine::ResourceState::ShaderResource });

		commandList->SetRenderTarget(m_TempTexture);
		commandList->ClearRenderTarget(m_TempTexture);
		commandList->SetShader(expandcoc);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 1u);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::RenderTarget },
			{ m_TempTexture->GetResource(), Engine::ResourceState::ShaderResource },
		});

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(expandcoc);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 0u);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::ShaderResource },
			{ m_TempTexture->GetResource(), Engine::ResourceState::RenderTarget },
		});

		Engine::GPUTimer::EndEvent(commandList); // end expand coc

		Engine::GPUTimer::BeginEvent(commandList, "Blur COC");

		commandList->SetRenderTarget(m_TempTexture);
		commandList->ClearRenderTarget(m_TempTexture);
		commandList->SetShader(blurcoc);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 1u);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::RenderTarget },
			{ m_TempTexture->GetResource(), Engine::ResourceState::ShaderResource },
		});

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(blurcoc);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 0u);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), Engine::ResourceState::ShaderResource },
			{ m_TempTexture->GetResource(), Engine::ResourceState::RenderTarget },
		});

		Engine::GPUTimer::EndEvent(commandList); // end blur coc

		Engine::GPUTimer::EndEvent(commandList); // end coc

		Engine::GPUTimer::BeginEvent(commandList, "Bokeh Blur");

		commandList->SetRenderTarget(m_BokehBlur);
		commandList->ClearRenderTarget(m_BokehBlur);
		commandList->SetShader(bokehBlur);
		commandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_Strength"), m_BokehStrangth);
		commandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_COC"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_NearBlur->GetResource(), Engine::ResourceState::ShaderResource },
			{ m_FarBlur->GetResource(), Engine::ResourceState::ShaderResource },
		});

		Engine::GPUTimer::EndEvent(commandList); // end bokeh blur

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composit);
		commandList->SetRootConstant(composit->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_NearLoc"), (uint32)m_NearBlur->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_FarLoc"), (uint32)m_FarBlur->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_COC"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);


		commandList->ValidateStates({
			{m_NearBlur->GetResource(), Engine::ResourceState::RenderTarget},
			{m_FarBlur->GetResource(), Engine::ResourceState::RenderTarget},
		});


		Engine::GPUTimer::EndEvent(commandList);
	}

	void DepthOfField::OnViewportResize(uint32 width, uint32 height)
	{
		m_COCTexture->Resize(width, height);
		m_TempTexture->Resize(width, height);
		m_NearBlur->Resize(width, height);
		m_FarBlur->Resize(width, height);
	}

}

