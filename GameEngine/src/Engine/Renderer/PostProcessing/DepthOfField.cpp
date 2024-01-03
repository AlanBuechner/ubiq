#include "pch.h"
#include "DepthOfField.h"
#include "EngineResource.h"
#include "Engine/Renderer/Camera.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

namespace Engine
{

	void DepthOfField::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_DepthOfFieldShader = Shader::CreateFromEmbeded(DEPTHOFFIELD, "DepthOfField.hlsl");

		m_Scene = &scene;

		uint32 width = 100;
		uint32 height = 100;

		m_TempTexture	= RenderTarget2D::Create(width, height, 1, TextureFormat::RGBA16_FLOAT);
		m_COCTexture	= RenderTarget2D::Create(width, height, 1, TextureFormat::RGBA16_FLOAT);
		m_NearBlur		= RenderTarget2D::Create(width, height, 1, TextureFormat::RGBA16_FLOAT);
		m_FarBlur		= RenderTarget2D::Create(width, height, 1, TextureFormat::RGBA16_FLOAT);

		m_BokehBlur = FrameBuffer::Create({
			m_NearBlur, m_FarBlur
		});
	}

	void DepthOfField::RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		GPUTimer::BeginEvent(commandList, "Depth Of Field");

		Ref<ShaderPass> coc = m_DepthOfFieldShader->GetPass("CoC");
		Ref<ShaderPass> expandcoc = m_DepthOfFieldShader->GetPass("expandCoC");
		Ref<ShaderPass> blurcoc = m_DepthOfFieldShader->GetPass("blurCoC");
		Ref<ShaderPass> bokehBlur = m_DepthOfFieldShader->GetPass("bokehBlur");
		Ref<ShaderPass> farBokehBlur = m_DepthOfFieldShader->GetPass("farBokehBlur");
		Ref<ShaderPass> composit = m_DepthOfFieldShader->GetPass("composit");

		GPUTimer::BeginEvent(commandList, "COC");
		GPUTimer::BeginEvent(commandList, "Calculate COC");

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(coc);
		commandList->SetRootConstant(coc->GetUniformLocation("RC_DepthLoc"), input.m_TextureHandles.at("Depth Buffer")->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(coc->GetUniformLocation("RC_Radius"), m_ConfusionRadius);
		commandList->SetRootConstant(coc->GetUniformLocation("RC_FocalPlane"), m_FocalPlane);
		commandList->SetConstantBuffer(coc->GetUniformLocation("camera"), m_Scene->m_MainCamera->GetCameraBuffer());
		commandList->DrawMesh(screenMesh);

		GPUTimer::EndEvent(commandList); // end calc coc

		GPUTimer::BeginEvent(commandList, "Expand COC");

		commandList->ValidateState({ m_COCTexture->GetResource(), ResourceState::ShaderResource });

		commandList->SetRenderTarget(m_TempTexture);
		commandList->ClearRenderTarget(m_TempTexture);
		commandList->SetShader(expandcoc);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 1u);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), ResourceState::RenderTarget },
			{ m_TempTexture->GetResource(), ResourceState::ShaderResource },
		});

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(expandcoc);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 0u);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), ResourceState::ShaderResource },
			{ m_TempTexture->GetResource(), ResourceState::RenderTarget },
		});

		GPUTimer::EndEvent(commandList); // end expand coc

		GPUTimer::BeginEvent(commandList, "Blur COC");

		commandList->SetRenderTarget(m_TempTexture);
		commandList->ClearRenderTarget(m_TempTexture);
		commandList->SetShader(blurcoc);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 1u);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), ResourceState::RenderTarget },
			{ m_TempTexture->GetResource(), ResourceState::ShaderResource },
		});

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(blurcoc);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 0u);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_COCTexture->GetResource(), ResourceState::ShaderResource },
			{ m_TempTexture->GetResource(), ResourceState::RenderTarget },
		});

		GPUTimer::EndEvent(commandList); // end blur coc

		GPUTimer::EndEvent(commandList); // end coc

		GPUTimer::BeginEvent(commandList, "Bokeh Blur");

		commandList->SetRenderTarget(m_BokehBlur);
		commandList->ClearRenderTarget(m_BokehBlur);
		commandList->SetShader(bokehBlur);
		commandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_Strength"), m_BokehStrangth);
		commandList->SetRootConstant(bokehBlur->GetUniformLocation("RC_COC"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);

		commandList->ValidateStates({
			{ m_NearBlur->GetResource(), ResourceState::ShaderResource },
			{ m_FarBlur->GetResource(), ResourceState::ShaderResource },
		});

		GPUTimer::EndEvent(commandList); // end bokeh blur

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composit);
		commandList->SetRootConstant(composit->GetUniformLocation("RC_SrcLoc"), src->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_NearLoc"), (uint32)m_NearBlur->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_FarLoc"), (uint32)m_FarBlur->GetSRVDescriptor()->GetIndex());
		commandList->SetRootConstant(composit->GetUniformLocation("RC_COC"), (uint32)m_COCTexture->GetSRVDescriptor()->GetIndex());
		commandList->DrawMesh(screenMesh);


		commandList->ValidateStates({
			{m_NearBlur->GetResource(), ResourceState::RenderTarget},
			{m_FarBlur->GetResource(), ResourceState::RenderTarget},
		});


		GPUTimer::EndEvent(commandList);
	}

	void DepthOfField::OnViewportResize(uint32 width, uint32 height)
	{
		m_COCTexture->Resize(width, height);
		m_TempTexture->Resize(width, height);
		m_NearBlur->Resize(width, height);
		m_FarBlur->Resize(width, height);
	}

}

