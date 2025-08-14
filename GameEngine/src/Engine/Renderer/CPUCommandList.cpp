#include "CPUCommandList.h"
#include "CPUCommands.h"
#include "Shaders/ShaderPass.h"
#include "Shaders/GraphicsShaderPass.h"
#include "Shaders/ComputeShaderPass.h"
#include "Shaders/WorkGraphShaderPass.h"
#include "Abstractions/Resources/FrameBuffer.h"
#include "Abstractions/Resources/ConstantBuffer.h"
#include "Abstractions/Resources/StructuredBuffer.h"
#include "Abstractions/Resources/InstanceBuffer.h"
#include "Mesh.h"

namespace Engine
{

	CPUCommandAllocator::~CPUCommandAllocator()
	{
		for (uint32 i = 0; i < m_Commands.Count(); i++)
			delete m_Commands[i];
	}

	void CPUCommandAllocator::PrependResourceStateCommands(const ResourceStateMap& resourceStates)
	{
		CREATE_PROFILE_FUNCTIONI();
		CPUResourceTransitionCommand* cmd = (CPUResourceTransitionCommand*)m_Commands[0]; // first command is always resource transition command
		cmd->resourceStateTransitons.ReserveMore(m_PendingTransitions.Count());

		for (ResourceStateObject& state : m_PendingTransitions)
		{
			// create transition
			ResourceTransitionObject transition;
			transition.resource = state.resource;
			transition.to = state.state;

			// default end of frame state to default if unknown
			if (state.resource->m_EOFState == ResourceState::Unknown)
				state.resource->m_EOFState = state.resource->m_DefultState;

			// find old state
			auto loc = resourceStates.find(state.resource);
			if (loc != resourceStates.end())
				transition.from = loc->second;
			else
				transition.from = state.resource->m_EOFState;

			// submit transition
			if (transition.to != transition.from)
				cmd->resourceStateTransitons.Push(transition);

			// set end of frame state
			state.resource->m_EOFState = state.state;
		}
	}

	void CPUCommandAllocator::MergeResourceStatesInto(ResourceStateMap& resourceStates)
	{
		CREATE_PROFILE_FUNCTIONI();
		for (auto state : m_ResourceStates)
			resourceStates[state.first] = state.second;
	}

#define ASSERT_ALLOCATOR CORE_ASSERT(m_CommandAllocator != nullptr, "Can not record commands without command allocator", "")

	CPUCommandList::~CPUCommandList()
	{
		delete m_CommandAllocator; // delete command allocator if one still exists
	}

	CPUCommandAllocator* CPUCommandList::TakeAllocator()
	{
		CPUCommandAllocator* alloc = m_CommandAllocator;
		m_CommandAllocator = nullptr;
		return alloc;
	}

	void CPUCommandList::StartRecording()
	{
		if(m_CommandAllocator)
			delete m_CommandAllocator; // delete old allocator if it was not used
		m_CommandAllocator = new CPUCommandAllocator(m_Name);
		m_CommandAllocator->SubmitCommand(new CPUResourceTransitionCommand()); // create transition command for resource state management

		m_BoundShader = nullptr;
		m_RenderTarget = nullptr;
	}

	void CPUCommandList::StopRecording()
	{
		
	}

	void CPUCommandList::BeginEvent(const char* eventName)
	{
		ASSERT_ALLOCATOR;
		CPUBeginEventStaticCommand* cmd = new CPUBeginEventStaticCommand();
		cmd->eventName = eventName;
		m_CommandAllocator->SubmitCommand(cmd);

		m_EventStack.Push(eventName);
	}

	void CPUCommandList::BeginEvent(const std::string& eventName)
	{
		ASSERT_ALLOCATOR;
		CPUBeginEventDynamicCommand* cmd = new CPUBeginEventDynamicCommand();
		cmd->eventName = eventName;
		m_CommandAllocator->SubmitCommand(cmd);

		m_EventStack.Push(eventName);
	}

	void CPUCommandList::EndEvent()
	{
		ASSERT_ALLOCATOR;
		CPUEndEventCommand* cmd = new CPUEndEventCommand();
		m_CommandAllocator->SubmitCommand(cmd);
		m_EventStack.Pop();
	}

	void CPUCommandList::Present(Ref<FrameBuffer> fb)
	{
		ASSERT_ALLOCATOR;
		Utils::Vector<ResourceStateObject> transitions(fb->GetAttachments().Count());
		for (uint32 i = 0; i < fb->GetAttachments().Count(); i++)
			transitions[i] = { fb->GetAttachment(i)->GetResource(), ResourceState::Common };

		ValidateStates(transitions);

		if (fb == m_RenderTarget)
			m_RenderTarget = nullptr;
	}

	void CPUCommandList::ValidateStates(const Utils::Vector<ResourceStateObject>& resources)
	{
		ASSERT_ALLOCATOR;

		// get resource state info from command allocator
		Utils::Vector<ResourceStateObject>& pendingTransitions = m_CommandAllocator->m_PendingTransitions;
		std::unordered_map<GPUResource*, ResourceState>& resourceStates = m_CommandAllocator->m_ResourceStates;

		// reserve space in transitions array
		Utils::Vector<ResourceTransitionObject> transitions;
		transitions.Reserve(resources.Count());

		// create transitions
		for (const ResourceStateObject& res : resources)
		{
			const std::unordered_map<GPUResource*, ResourceState>::iterator& foundResouce = resourceStates.find(res.resource);

			if (foundResouce == resourceStates.end())
			{
				pendingTransitions.Push(res);
				resourceStates[res.resource] = res.state;
			}
			else
			{
				ResourceState currState = foundResouce->second;
				if (currState != res.state)
				{
					transitions.Push({ res.resource, res.state, currState });
					resourceStates[res.resource] = res.state;
				}
			}
		}

		// create transition command
		if (!transitions.Empty())
			Transition(transitions);
	}

	void CPUCommandList::ValidateState(Ref<FrameBuffer> frameBuffer)
	{
		Utils::Vector<ResourceStateObject> transitions;
		transitions.Reserve(frameBuffer->GetAttachments().Count());
		for (uint32 i = 0; i < frameBuffer->GetAttachments().Count(); i++)
			transitions.Push({ frameBuffer->GetAttachment(i)->GetResource(), ResourceState::RenderTarget });
		ValidateStates(transitions);
	}

	void CPUCommandList::AwaitUAVs(Utils::Vector<GPUResource*> uavs)
	{
		ASSERT_ALLOCATOR;
		// get last or create new UAV command
		CPUAwaitUAVCommand* cmd = nullptr;
		CPUCommand* lastCMD = m_CommandAllocator->PeekCommand();
		if (lastCMD->GetCommandID() == CPUAwaitUAVCommand::GetStaticCommandID())
			cmd = (CPUAwaitUAVCommand*)lastCMD;
		else
			cmd = new CPUAwaitUAVCommand();

		// add UAVs to command
		for (uint32 i = 0; i < uavs.Count(); i++)
			cmd->UAVs.Push(uavs[i]);

		// submit command
		if (cmd != lastCMD)
			m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::CopyBuffer(GPUResource* dest, uint64 destOffset, GPUResource* src, uint64 srcOffset, uint64 size)
	{
		ASSERT_ALLOCATOR;
		CPUCopyBufferCommand* cmd = new CPUCopyBufferCommand();

		cmd->dest = dest;
		cmd->destOffset = destOffset;
		cmd->src = src;
		cmd->srcOffset = srcOffset;
		cmd->size = size;

		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::UploadTexture(GPUResource* dest, UploadTextureResource* src)
	{
		ASSERT_ALLOCATOR;
		CPUUploadTextureCommand* cmd = new CPUUploadTextureCommand();

		cmd->dest = dest;
		cmd->src = src;

		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetViewport(Math::Vector2 pos, Math::Vector2 size, Math::Vector2 depths)
	{
		ASSERT_ALLOCATOR;
		// get last or create new set viewport command
		CPUSetViewportCommand* cmd = nullptr;
		CPUCommand* lastCMD = m_CommandAllocator->PeekCommand();
		if (lastCMD->GetCommandID() == CPUSetViewportCommand::GetStaticCommandID())
			cmd = (CPUSetViewportCommand*)lastCMD;
		else
			cmd = new CPUSetViewportCommand();

		cmd->pos = pos;
		cmd->size = size;
		cmd->depths = depths;

		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetRenderTarget(Ref<FrameBuffer> buffer)
	{
		ASSERT_ALLOCATOR;

		if (buffer == m_RenderTarget)
			return;

		m_RenderTarget = buffer;

		// get last or create new set render target command
		CPUSetRenderTargetCommand* cmd = new CPUSetRenderTargetCommand();

		// reset data if overwriting last command
		cmd->depthStencil = nullptr; // always default to nullptr
		cmd->renderTargetHandles.Clear();

		uint32 numRenderTargets = buffer->GetAttachments().Count() - (size_t)m_RenderTarget->HasDepthAttachment();
		Utils::Vector<ResourceStateObject> resourceStateValidation;

		// collect resource states and build command
		for (uint32 i = 0; i < buffer->GetAttachments().Count(); i++)
		{
			Texture2DResource* res = buffer->GetAttachment(i)->GetResource();
			Texture2DRTVDSVDescriptorHandle* handle = buffer->GetAttachment(i)->GetRTVDSVDescriptor();

			resourceStateValidation.Push({ buffer->GetAttachment(i)->GetResource(), ResourceState::RenderTarget });

			if (IsTextureFormatDepthStencil(res->GetFormat()))
				cmd->depthStencil = handle;
			else
				cmd->renderTargetHandles.Push(handle);
		}
		
		// set resource states
		ValidateStates(resourceStateValidation);

		// set viewport
		SetViewport(buffer);

		// submit command
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer)
	{
		ValidateState(frameBuffer);
		for (uint32 i = 0; i < frameBuffer->GetAttachments().Count(); i++)
			ClearRenderTarget(frameBuffer, i);
	}

	void CPUCommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, const Math::Vector4& color)
	{
		ValidateState(frameBuffer);
		for (uint32 i = 0; i < frameBuffer->GetAttachments().Count(); i++)
			ClearRenderTarget(frameBuffer, i, color);
	}

	void CPUCommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment)
	{
		Math::Vector4 color = frameBuffer->GetAttachment(attachment)->GetClearColor();
		ClearRenderTarget(frameBuffer, attachment, color);
	}

	void CPUCommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color)
	{
		ClearRenderTarget(frameBuffer->GetAttachment(attachment), color);
	}

	void CPUCommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget)
	{
		ClearRenderTarget(renderTarget, renderTarget->GetClearColor());
	}

	void CPUCommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget, const Math::Vector4& color)
	{
		ASSERT_ALLOCATOR;

		ValidateState(renderTarget->GetResource(), ResourceState::RenderTarget);

		CPUClearRenderTargetCommand* cmd = new CPUClearRenderTargetCommand();

		cmd->handle = renderTarget->GetRTVDSVDescriptor();
		cmd->color = color;
		cmd->isDepthStencil = IsTextureFormatDepthStencil(renderTarget->GetResource()->GetFormat());

		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetShader(Ref<GraphicsShaderPass> shader)
	{
		ASSERT_ALLOCATOR;
		if (shader == m_BoundShader)
			return;

		CPUSetGraphicsShaderCommand* cmd = new CPUSetGraphicsShaderCommand();
		cmd->shaderPass = shader;
		cmd->format = m_RenderTarget->GetFormats();
		m_CommandAllocator->SubmitCommand(cmd);

		m_BoundShader = shader;
	}

	void CPUCommandList::SetShader(Ref<ComputeShaderPass> shader)
	{
		ASSERT_ALLOCATOR;
		CPUSetComputeShaderCommand* cmd = new CPUSetComputeShaderCommand();
		cmd->shaderPass = shader;
		m_CommandAllocator->SubmitCommand(cmd);

		m_BoundShader = shader;
	}

	void CPUCommandList::SetShader(Ref<WorkGraphShaderPass> shader)
	{
		ASSERT_ALLOCATOR;
		CPUSetWorkGraphShaderCommand* cmd = new CPUSetWorkGraphShaderCommand();
		cmd->shaderPass = shader;
		m_CommandAllocator->SubmitCommand(cmd);

		m_BoundShader = shader;
	}

	void CPUCommandList::SetRootConstant(uint32 index, uint32 data)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		CPUSetRootConstantCommand* cmd = new CPUSetRootConstantCommand();
		cmd->index = index;
		cmd->data = data;
		cmd->isCompute = m_BoundShader->IsComputeShader();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(buffer->GetResource(), ResourceState::ShaderResource);

		CPUSetConstantBufferCommand* cmd = new CPUSetConstantBufferCommand();
		cmd->index = index;
		cmd->res = buffer->GetResource();
		cmd->isCompute = m_BoundShader->IsComputeShader();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(buffer->GetResource(), ResourceState::ShaderResource);

		CPUSetStructuredBufferCommand* cmd = new CPUSetStructuredBufferCommand();
		cmd->index = index;
		cmd->handle = buffer->GetSRVDescriptor();
		cmd->isCompute = m_BoundShader->IsComputeShader();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetRWStructuredBuffer(uint32 index, Ref<RWStructuredBuffer> buffer)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(buffer->GetResource(), ResourceState::UnorderedResource);

		CPUSetRWStructuredBufferCommand* cmd = new CPUSetRWStructuredBufferCommand();
		cmd->index = index;
		cmd->handle = buffer->GetUAVDescriptor();
		cmd->isCompute = m_BoundShader->IsComputeShader();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetTexture(uint32 index, Ref<Texture2D> texture)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(texture->GetResource(), ResourceState::ShaderResource);

		CPUSetTextureCommand* cmd = new CPUSetTextureCommand();
		cmd->index = index;
		cmd->handle = texture->GetSRVDescriptor();
		cmd->isCompute = m_BoundShader->IsComputeShader();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(uav->m_Resource, ResourceState::UnorderedResource);

		CPUSetRWTextureCommand* cmd = new CPUSetRWTextureCommand();
		cmd->index = index;
		cmd->handle = uav;
		cmd->isCompute = m_BoundShader->IsComputeShader();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::SetRWTexture(uint32 index, Ref<RWTexture2D> texture, uint32 mip)
	{
		SetRWTexture(index, texture->GetUAVDescriptor(mip));
	}

	void CPUCommandList::DrawMesh(Ref<Mesh> mesh, Ref<InstanceBuffer> instanceBuffer, uint32 numInstances)
	{
		ASSERT_ALLOCATOR;

		ValidateState(mesh->GetVertexBuffer()->GetResource(), ResourceState::PiplineInput);
		ValidateState(mesh->GetIndexBuffer()->GetResource(), ResourceState::PiplineInput);
		if(instanceBuffer)
			ValidateState(instanceBuffer->GetResource(), ResourceState::PiplineInput);

		CPUDrawMeshCommand* cmd = new CPUDrawMeshCommand();
		cmd->vertexBufferView = mesh->GetVertexBuffer()->GetView();
		cmd->indexBufferView = mesh->GetIndexBuffer()->GetView();
		cmd->numIndices = mesh->GetIndexBuffer()->GetCount();

		if (instanceBuffer)
		{
			cmd->instanceBufferView = instanceBuffer->GetView();
			cmd->numInstances = numInstances;
			if (numInstances == UINT32_MAX)
				cmd->numInstances = instanceBuffer->GetCount();
		}
		else
		{
			cmd->instanceBufferView = nullptr;
			cmd->numInstances = 0;
		}

		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::DispatchGroups(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGroupsZ)
	{
		ASSERT_ALLOCATOR;
		CPUDispatchCommand* cmd = new CPUDispatchCommand();
		cmd->threadGroupsX = threadGroupsX;
		cmd->threadGroupsY = threadGroupsY;
		cmd->threadGroupsZ = threadGroupsZ;
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::DispatchThreads(uint32 threadsX, uint32 threadsY, uint32 threadsZ)
	{
		Math::UVector3 groupSize = std::dynamic_pointer_cast<ComputeShaderPass>(m_BoundShader)->GetThreadGroupSize();

		uint32 groupsX = Math::Ceil((float)threadsX / (float)groupSize.x);
		uint32 groupsY = Math::Ceil((float)threadsY / (float)groupSize.y);
		uint32 groupsZ = Math::Ceil((float)threadsZ / (float)groupSize.z);
		DispatchGroups(groupsX, groupsY, groupsZ);
	}

	void CPUCommandList::DispatchGraph(void* data, uint32 stride, uint32 count)
	{
		ASSERT_ALLOCATOR;
		CPUDispatchGraphCPUDataCommand* cmd = new CPUDispatchGraphCPUDataCommand();
		cmd->stride = stride;
		cmd->count = count;
		if (data)
		{
			cmd->data.Resize(count * stride);
			memcpy(cmd->data.Data(), data, stride * count);
		}
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::DispatchGraph(Ref<StructuredBuffer> buffer)
	{
		ASSERT_ALLOCATOR;
		CPUDispatchGraphGPUDataCommand* cmd = new CPUDispatchGraphGPUDataCommand();
		cmd->res = buffer->GetResource();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::Transition(const Utils::Vector<ResourceTransitionObject>& transitions)
	{
		ASSERT_ALLOCATOR;

		// get last or create new transition command
		CPUResourceTransitionCommand* cmd = nullptr;
		CPUCommand* lastCMD = m_CommandAllocator->PeekCommand();
		if (lastCMD->GetCommandID() == CPUResourceTransitionCommand::GetStaticCommandID())
			cmd = (CPUResourceTransitionCommand*)lastCMD;
		else
			cmd = new CPUResourceTransitionCommand();

		// add transitions to command
		for (uint32 i = 0; i < transitions.Count(); i++)
		{
			ResourceState to = transitions[i].to;
			GPUResource* resource = transitions[i].resource;
			CORE_ASSERT(resource->SupportState(to), "resouce does not support state: {0}", to);
			cmd->resourceStateTransitons.Push(transitions[i]);
		}

		// submit command
		if(cmd != lastCMD)
			m_CommandAllocator->SubmitCommand(cmd);
	}

#undef ASSERT_ALLOCATOR
}

