#include "pch.h"
#include "DepthOfField.h"
#include "EngineResource.h"

#include "Engine/Renderer/GPUProfiler.h"

namespace Engine
{

	void DepthOfField::Init(const PostProcessInput& input, SceneData& scene)
	{
		m_DepthOfFieldShader = Shader::CreateFromEmbeded(DEPTHOFFIELD, "DepthOfField.hlsl");

		m_Scene = &scene;

		FrameBufferSpecification spec;
		spec.Attachments = {
			{ FrameBufferTextureFormat::RGBA16, {0.1f,0.1f,0.1f,1} },
			{ FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
		};
		spec.InitalState = FrameBufferState::RenderTarget;
		spec.Width = 100;
		spec.Height = 100;

		m_COCTexture = FrameBuffer::Create(spec);
		m_TempTexture = FrameBuffer::Create(spec);
		m_NearBlur = FrameBuffer::Create(spec);
		m_FarBlur = FrameBuffer::Create(spec);
	}

	void DepthOfField::RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh)
	{
		if (m_COCTexture->GetSpecification().Height != renderTarget->GetSpecification().Height || m_COCTexture->GetSpecification().Width != renderTarget->GetSpecification().Width)
		{
			m_COCTexture->Resize(renderTarget->GetSpecification().Width, renderTarget->GetSpecification().Height);
			m_TempTexture->Resize(renderTarget->GetSpecification().Width, renderTarget->GetSpecification().Height);
		}

		GPUTimer::BeginEvent(commandList, "Depth Of Field");

		Ref<ShaderPass> coc = m_DepthOfFieldShader->GetPass("CoC");
		Ref<ShaderPass> expandcoc = m_DepthOfFieldShader->GetPass("expandCoC");
		Ref<ShaderPass> blurcoc = m_DepthOfFieldShader->GetPass("blurCoC");
		Ref<ShaderPass> composit = m_DepthOfFieldShader->GetPass("composit");

		GPUTimer::BeginEvent(commandList, "COC");
		GPUTimer::BeginEvent(commandList, "calculate COC");

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(coc);
		commandList->SetRootConstant(coc->GetUniformLocation("RC_DepthLoc"), (uint32)input.m_TextureHandles.at("Depth Buffer"));
		commandList->SetRootConstant(coc->GetUniformLocation("RC_Radius"), m_ConfusionRadius);
		commandList->SetRootConstant(coc->GetUniformLocation("RC_FocalPlane"), m_FocalPlane);
		commandList->SetConstantBuffer(coc->GetUniformLocation("camera"), m_Scene->m_MainCamera->GetCameraBuffer());
		commandList->DrawMesh(screenMesh);

		GPUTimer::EndEvent(commandList);

		GPUTimer::BeginEvent(commandList, "expand COC");

		commandList->Transition({ m_COCTexture }, FrameBufferState::SRV, FrameBufferState::RenderTarget);

		commandList->SetRenderTarget(m_TempTexture);
		commandList->ClearRenderTarget(m_TempTexture);
		commandList->SetShader(expandcoc);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 1u);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetAttachmentShaderDescriptoLocation(0));
		commandList->DrawMesh(screenMesh);

		commandList->Transition({
			{m_COCTexture, FrameBufferState::RenderTarget, FrameBufferState::SRV},
			{m_TempTexture, FrameBufferState::SRV, FrameBufferState::RenderTarget},
		});

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(expandcoc);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_X"), 0u);
		commandList->SetRootConstant(expandcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetAttachmentShaderDescriptoLocation(0));
		commandList->DrawMesh(screenMesh);

		commandList->Transition({
			{m_COCTexture, FrameBufferState::SRV, FrameBufferState::RenderTarget},
			{m_TempTexture, FrameBufferState::RenderTarget, FrameBufferState::SRV},
		});

		GPUTimer::EndEvent(commandList);

		GPUTimer::BeginEvent(commandList, "blur COC");

		commandList->SetRenderTarget(m_TempTexture);
		commandList->ClearRenderTarget(m_TempTexture);
		commandList->SetShader(blurcoc);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 1u);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_COCTexture->GetAttachmentShaderDescriptoLocation(0));
		commandList->DrawMesh(screenMesh);

		commandList->Transition({
			{m_COCTexture, FrameBufferState::RenderTarget, FrameBufferState::SRV},
			{m_TempTexture, FrameBufferState::SRV, FrameBufferState::RenderTarget},
		});

		commandList->SetRenderTarget(m_COCTexture);
		commandList->ClearRenderTarget(m_COCTexture);
		commandList->SetShader(blurcoc);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_Radius"), m_COCBlurRadius);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_X"), 0u);
		commandList->SetRootConstant(blurcoc->GetUniformLocation("RC_SrcLoc"), (uint32)m_TempTexture->GetAttachmentShaderDescriptoLocation(0));
		commandList->DrawMesh(screenMesh);

		commandList->Transition({ m_TempTexture }, FrameBufferState::RenderTarget, FrameBufferState::SRV);

		GPUTimer::EndEvent(commandList);

		GPUTimer::EndEvent(commandList);

		commandList->SetRenderTarget(renderTarget);
		commandList->ClearRenderTarget(renderTarget);
		commandList->SetShader(composit);
		commandList->SetRootConstant(composit->GetUniformLocation("RC_SrcLoc"), (uint32)srcDescriptorLocation);
		commandList->DrawMesh(screenMesh);


		GPUTimer::EndEvent(commandList);
	}

}

