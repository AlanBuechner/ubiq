#include "pch.h"
#include "CommandList.h"
#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Resources/Buffer.h"
#include "Engine/Renderer/Resources/InstanceBuffer.h"
#include "Engine/Renderer/Resources/ConstantBuffer.h"
#include "Engine/Renderer/Resources/StructuredBuffer.h"
#include "Engine/Renderer/Resources/Texture.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Util/Performance.h"
#include "Commands.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/Commands/DirectX12CommandList.h"
#endif

namespace Engine
{
	Ref<CommandList> Engine::CommandList::Create(CommandListType type)
	{
		CREATE_PROFILE_FUNCTIONI();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12CommandList>(type);
		default:
			break;
		}
		return Ref<CommandList>();
	}

	void CommandList::Transition(std::vector<ResourceTransitionObject> transitions)
	{
		TransitionCommand* cmd = new TransitionCommand();
		cmd->m_Transitions = transitions;
		m_Commands.push_back(cmd);
	}

	void CommandList::StartRecording()
	{
		for (auto cmd : m_Commands)
			delete cmd;
		m_Commands.clear();

		TransitionCommand* tcmd = new TransitionCommand();
		m_Commands.push_back(tcmd);

		m_PendingTransitions.clear();
		m_ResourceStates.clear();

		m_RecordFlag.Wait();
		m_RecordFlag.Clear();
		m_RenderTarget = nullptr;
	}

	void CommandList::Close()
	{
		m_RenderTarget = nullptr;
	}

	void CommandList::Present(Ref<FrameBuffer> fb)
	{
		if (fb == nullptr)
			fb = m_RenderTarget;

		std::vector<ResourceStateObject> transitions(fb->GetAttachments().size());
		for (uint32 i = 0; i < fb->GetAttachments().size(); i++)
			transitions[i] = { fb->GetAttachment(i)->GetResource(), ResourceState::Common };

		ValidateStates(transitions);

		if (fb == m_RenderTarget)
			m_RenderTarget = nullptr;
	}

	void CommandList::ValidateStates(std::vector<ResourceStateObject> resources)
	{
		std::vector<ResourceTransitionObject> transitions;
		transitions.reserve(resources.size());

		for (ResourceStateObject& res : resources)
		{
			const std::unordered_map<GPUResource*, ResourceState>::iterator& foundResouce = m_ResourceStates.find(res.resource);

			if (foundResouce == m_ResourceStates.end())
			{
				m_PendingTransitions.push_back(res);
				m_ResourceStates[res.resource] = res.state;
			}
			else
			{
				ResourceState currState = foundResouce->second;
				if (currState != res.state)
				{
					transitions.push_back({ res.resource, res.state, currState });
					m_ResourceStates[res.resource] = res.state;
				}
			}
		}

		if (!transitions.empty())
			Transition(transitions);
	}

	void CommandList::SetRenderTarget(Ref<FrameBuffer> buffer)
	{
		if (m_Type == CommandList::Bundle)
		{
			CORE_WARN("Command bundle can not set render targets. command will be ignored");
			return;
		}

		std::vector<ResourceStateObject> resourceStateValidation;
		for (uint32 i = 0; i < buffer->GetAttachments().size(); i++)
			resourceStateValidation.push_back({ buffer->GetAttachment(i)->GetResource(), ResourceState::RenderTarget });
		ValidateStates(resourceStateValidation);

		SetRenderTargetCommand* cmd = new SetRenderTargetCommand();
		cmd->m_HasDepthAttachment = buffer->HasDepthAttachment();
		for (uint32 i = 0; i < buffer->GetAttachments().size(); i++)
			cmd->m_AttachmentHandles.push_back(buffer->GetAttachment(i)->GetRTVDSVDescriptor());
		m_Commands.push_back(cmd);

		m_RenderTarget = buffer;
	}

	void CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer)
	{
		for (uint32 i = 0; i < frameBuffer->GetAttachments().size(); i++)
			ClearRenderTarget(frameBuffer, i);
	}

	void CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment)
	{
		Math::Vector4 color = frameBuffer->GetAttachment(attachment)->GetClearColor();
		ClearRenderTarget(frameBuffer, attachment, color);
	}

	void CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color)
	{
		ClearRenderTarget(frameBuffer->GetAttachment(attachment), color);
	}

	void CommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget)
	{
		ClearRenderTarget(renderTarget, renderTarget->GetClearColor());
	}

	void CommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color)
	{
		ClearRenderTargetCommand* cmd = new ClearRenderTargetCommand();
		cmd->m_RenderTargetHandle = renderTarget->GetRTVDSVDescriptor();
		cmd->m_Color = color;
		m_Commands.push_back(cmd);
	}

	void CommandList::SetShader(Ref<ShaderPass> shader)
	{
		if (!shader->IsComputeShader() && !m_RenderTarget)
		{
			CORE_ERROR("Can not set shader without rendertarget");
			return;
		}

		SetShaderCommand* cmd = new SetShaderCommand();
		cmd->m_ShaderPass = shader.get();
		cmd->m_FrameBufferSpecification = m_RenderTarget->GetSpecification();
		m_Commands.push_back(cmd);

		m_BoundShader = shader;
	}

	void CommandList::SetRootConstant(uint32 index, uint32 data)
	{
		if (index == UINT32_MAX)
			return; // invalid bind slot

		SetRootConstantCommand* cmd = new SetRootConstantCommand();
		cmd->m_IsComputeShader = m_BoundShader->IsComputeShader();
		cmd->m_Index = index;
		cmd->m_Data = data;
		m_Commands.push_back(cmd);
	}

	void CommandList::SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer)
	{
		if (index == UINT32_MAX || buffer == nullptr)
			return; // invalid bind slot

		ValidateState(buffer->GetResource(), ResourceState::ShaderResource);

		SetConstantBufferCommand* cmd = new SetConstantBufferCommand();
		cmd->m_IsComputeShader = m_BoundShader->IsComputeShader();
		cmd->m_Index = index;
		cmd->m_CBVHandle = buffer->GetCBVDescriptor();
		m_Commands.push_back(cmd);
	}

	void CommandList::SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer)
	{
		if (index == UINT32_MAX || buffer == nullptr)
			return; // invalid bind slot

		ValidateState(buffer->GetResource(), ResourceState::ShaderResource);

		SetStructuredBufferCommand* cmd = new SetStructuredBufferCommand();
		cmd->m_IsComputeShader = m_BoundShader->IsComputeShader();
		cmd->m_Index = index;
		cmd->m_SRVHandle = buffer->GetSRVDescriptor();
		m_Commands.push_back(cmd);
	}

	void CommandList::SetTexture(uint32 index, Ref<Texture2D> texture)
	{
		if (index == UINT32_MAX || texture == nullptr)
			return; // invalid bind slot

		ValidateState(texture->GetResource(), ResourceState::ShaderResource);

		SetTextureCommand* cmd = new SetTextureCommand();
		cmd->m_IsComputeShader = m_BoundShader->IsComputeShader();
		cmd->m_Index = index;
		cmd->m_SRVHandle = texture->GetSRVDescriptor();
		m_Commands.push_back(cmd);
	}

	void CommandList::SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav)
	{
		if (index == UINT32_MAX || uav == nullptr)
			return; // invalid bind slot

		ValidateState(uav->m_Resource, ResourceState::UnorderedResource);

		SetRWTextureCommand* cmd = new SetRWTextureCommand();
		cmd->m_IsComputeShader = m_BoundShader->IsComputeShader();
		cmd->m_Index = index;
		cmd->m_UAVHandle = uav;
		m_Commands.push_back(cmd);
	}

	void CommandList::DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer /*= nullptr*/, int numInstances /*= -1*/)
	{
		DrawMeshCommand* cmd = new DrawMeshCommand();
		cmd->m_VertexBufferView = mesh->GetVertexBuffer()->GetView();
		cmd->m_IndexBufferView = mesh->GetIndexBuffer()->GetView();
		if (instanceBuffer)
		{
			cmd->m_Instances = instanceBuffer->GetView();
			cmd->m_NumberOfInstances = (numInstances < 0) ? instanceBuffer->GetCount() : (uint32)numInstances;
		}
		else
			cmd->m_Instances = nullptr;

		m_Commands.push_back(cmd);
	}

	void CommandList::DispatchCompute(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGroupsZ)
	{
		DisbatchComputeCommand* cmd = new DisbatchComputeCommand();
		cmd->m_ThreadGroupsX = threadGroupsX;
		cmd->m_ThreadGroupsY = threadGroupsY;
		cmd->m_ThreadGroupsZ = threadGroupsZ;
		m_Commands.push_back(cmd);
	}

}
