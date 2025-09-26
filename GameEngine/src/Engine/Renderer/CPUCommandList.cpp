#include "CPUCommandList.h"
#include "CPUCommands.h"
#include "Shaders/ShaderPass.h"
#include "Shaders/GraphicsShaderPass.h"
#include "Shaders/ComputeShaderPass.h"
#include "Shaders/WorkGraphShaderPass.h"
#include "Abstractions/Resources/FrameBuffer.h"
#include "Abstractions/Resources/ConstantBuffer.h"
#include "Abstractions/Resources/StructuredBuffer.h"
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
		}
	}

	void CPUCommandAllocator::MergeResourceStatesInto(ResourceStateMap& resourceStates)
	{
		CREATE_PROFILE_FUNCTIONI();
		for (auto state : m_ResourceStates)
			resourceStates[state.first] = state.second;
	}

	void CPUCommandAllocator::SetEndOfFrameStates()
	{
		CREATE_PROFILE_FUNCTIONI();
		for (auto state : m_ResourceStates)
			state.first->m_EOFState = state.second;
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

	void CPUCommandList::BeginGPUEvent(const tracy::SourceLocationData* data)
	{
		ASSERT_ALLOCATOR;
		CPUBeginGPUEventCommand* cmd = new CPUBeginGPUEventCommand();
		cmd->data = data;
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::EndGPUEvent()
	{
		ASSERT_ALLOCATOR;
		CPUEndGPUEventCommand* cmd = new CPUEndGPUEventCommand();
		m_CommandAllocator->SubmitCommand(cmd);
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

	void CPUCommandList::ValidateState(Ref<FrameBuffer> frameBuffer, ResourceState state)
	{
		Utils::Vector<ResourceStateObject> transitions;
		transitions.Reserve(frameBuffer->GetAttachments().Count());
		for (uint32 i = 0; i < frameBuffer->GetAttachments().Count(); i++)
			transitions.Push({ frameBuffer->GetAttachment(i)->GetResource(), state });
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

	void CPUCommandList::AllocateTransient(Ref<RenderTarget2D> renderTarget)
	{
		ASSERT_ALLOCATOR;

		CPUOpenTransientCommand* cmd = new CPUOpenTransientCommand();
		cmd->res = renderTarget->GetResource();

		cmd->descriptors.Push(renderTarget->GetRTVDSVDescriptor());
		cmd->descriptors.Push(renderTarget->GetSRVDescriptor());

		Ref<RWTexture2D> rwTexture = renderTarget->GetRWTexture2D();
		if (rwTexture)
		{
			for (uint32 i = 0; i < rwTexture->GetMips(); i++)
				cmd->descriptors.Push(rwTexture->GetUAVDescriptor(i));
		}

		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::AllocateTransient(Ref<FrameBuffer> buffer)
	{
		for (Ref<RenderTarget2D> rt : buffer->GetAttachments())
			AllocateTransient(rt);
	}

	void CPUCommandList::CloseTransient(Ref<RenderTarget2D> renderTarget)
	{
		ASSERT_ALLOCATOR;
		CPUCloseTransientCommand* cmd = new CPUCloseTransientCommand();
		cmd->res = renderTarget->GetResource();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::CloseTransient(Ref<FrameBuffer> buffer)
	{
		for (Ref<RenderTarget2D> rt : buffer->GetAttachments())
			CloseTransient(rt);
	}

	void CPUCommandList::ResolveMSAA(Ref<FrameBuffer> texture, Ref<FrameBuffer> msaaTexture)
	{
		ASSERT_ALLOCATOR;
		CORE_ASSERT(texture->GetFormats() == msaaTexture->GetFormats(), "resolveing msaa frame buffer requires both buffers to have the same formats", "");

		ValidateState(texture, ResourceState::ResolveDestination);
		ValidateState(msaaTexture, ResourceState::ResolveSource);

		CPUResolveMSAACommand* cmd = new CPUResolveMSAACommand();
		for (uint32 i = 0; i < texture->GetAttachments().Count(); i++)
		{
			cmd->resolves.Push({
				texture->GetAttachment(i)->GetResource(),
				msaaTexture->GetAttachment(i)->GetResource()
			});
		}
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
		cmd->fbDesc = m_RenderTarget->GetDescription();
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

		GPUDataBinding& binding = m_Bindings.Push({});
		binding.index = index;
		binding.isCompute = m_BoundShader->IsComputeShader();
		binding.type = GPUDataBinding::Type::RootConstant;
		binding.data = data;
	}

	void CPUCommandList::SetConstantBuffer(uint32 index, Ref<ConstantBuffer> buffer)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(buffer->GetResource(), ResourceState::ShaderResource);

		GPUDataBinding& binding = m_Bindings.Push({});
		binding.index = index;
		binding.isCompute = m_BoundShader->IsComputeShader();
		binding.type = GPUDataBinding::Type::ConstantBuffer;
		binding.data = (uint64)buffer->GetResource();
	}

	void CPUCommandList::SetStructuredBuffer(uint32 index, Ref<StructuredBuffer> buffer)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(buffer->GetResource(), ResourceState::ShaderResource);

		GPUDataBinding& binding = m_Bindings.Push({});
		binding.index = index;
		binding.isCompute = m_BoundShader->IsComputeShader();
		binding.type = GPUDataBinding::Type::StructuredBuffer;
		binding.data = (uint64)buffer->GetSRVDescriptor();
	}

	void CPUCommandList::SetRWStructuredBuffer(uint32 index, Ref<RWStructuredBuffer> buffer)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(buffer->GetResource(), ResourceState::UnorderedResource);

		GPUDataBinding& binding = m_Bindings.Push({});
		binding.index = index;
		binding.isCompute = m_BoundShader->IsComputeShader();
		binding.type = GPUDataBinding::Type::RWStructuredBuffer;
		binding.data = (uint64)buffer->GetUAVDescriptor();
	}

	void CPUCommandList::SetTexture(uint32 index, Ref<Texture2D> texture)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(texture->GetResource(), ResourceState::ShaderResource);

		GPUDataBinding& binding = m_Bindings.Push({});
		binding.index = index;
		binding.isCompute = m_BoundShader->IsComputeShader();
		binding.type = GPUDataBinding::Type::Texture2D;
		binding.data = (uint64)texture->GetSRVDescriptor();
	}

	void CPUCommandList::SetRWTexture(uint32 index, Texture2DUAVDescriptorHandle* uav)
	{
		if (index == UINT32_MAX) return;
		ASSERT_ALLOCATOR;

		ValidateState(uav->GetResource(), ResourceState::UnorderedResource);

		GPUDataBinding& binding = m_Bindings.Push({});
		binding.index = index;
		binding.isCompute = m_BoundShader->IsComputeShader();
		binding.type = GPUDataBinding::Type::RWTexture2D;
		binding.data = (uint64)uav;
	}

	void CPUCommandList::SetRWTexture(uint32 index, Ref<RWTexture2D> texture, uint32 mip)
	{
		SetRWTexture(index, texture->GetUAVDescriptor(mip));
	}

	void CPUCommandList::SetVertexBuffer(uint32 stream, Ref<VertexBuffer> vb)
	{
		if (stream >= m_BoundVertexBuffers.Count())
			m_BoundVertexBuffers.Resize(stream + 1);

		m_BoundVertexBuffers[stream] = vb;
	}

	void CPUCommandList::SetIndexBuffer(Ref<IndexBuffer> ib)
	{
		m_BoundIndexBuffer = ib;
	}



	void CPUCommandList::DrawInstanced(uint32 numInstances)
	{
		ASSERT_ALLOCATOR;
		FlushBindings();

		for (Ref<VertexBuffer> vb : m_BoundVertexBuffers)
		{
			CORE_ASSERT(vb != nullptr, "Missing Vertex Stream", "");
			ValidateState(vb->GetResource(), ResourceState::PiplineInput);
		}
		ValidateState(m_BoundIndexBuffer->GetResource(), ResourceState::PiplineInput);

		CPUDrawMeshCommand* cmd = new CPUDrawMeshCommand();
		cmd->vertexBufferViews.Reserve(m_BoundVertexBuffers.Count());
		for (Ref<VertexBuffer> vb : m_BoundVertexBuffers)
		{
			CORE_ASSERT(vb != nullptr, "Missing Vertex Stream", "");
			cmd->vertexBufferViews.Push(vb->GetView());
		}
		cmd->indexBufferView = m_BoundIndexBuffer->GetView();
		cmd->numIndices = m_BoundIndexBuffer->GetCount();
		cmd->numInstances = numInstances;

		m_CommandAllocator->SubmitCommand(cmd);

		m_BoundVertexBuffers.Clear();
		m_BoundIndexBuffer = nullptr;
	}

	void CPUCommandList::DrawMesh(Ref<Mesh> mesh)
	{
		DrawMesh(mesh->GetVertexBuffers(), mesh->GetIndexBuffer());
	}

	void CPUCommandList::DrawMesh(Ref<Mesh> mesh, Ref<VertexBuffer> instanceBuffer, uint32 numInstances)
	{
		DrawMesh(mesh, Utils::Vector{ instanceBuffer }, numInstances);
	}

	void CPUCommandList::DrawMesh(Ref<Mesh> mesh, Utils::Vector<Ref<VertexBuffer>> instanceBuffers, uint32 numInstances)
	{
		Utils::Vector<Ref<VertexBuffer>> vertexBuffers;
		vertexBuffers = mesh->GetVertexBuffers();
		vertexBuffers.Append(instanceBuffers);

		if (numInstances == UINT32_MAX && !instanceBuffers.Empty())
			numInstances = instanceBuffers[0]->GetCount();

		DrawMesh(vertexBuffers, mesh->GetIndexBuffer(), numInstances);
	}

	void CPUCommandList::DrawMesh(Utils::Vector<Ref<VertexBuffer>> vertexBuffers, Ref<IndexBuffer> indexBuffer, uint32 numInstances)
	{
		m_BoundVertexBuffers = vertexBuffers;
		m_BoundIndexBuffer = indexBuffer;

		DrawInstanced(numInstances);
	}

	void CPUCommandList::DispatchGroups(uint32 threadGroupsX, uint32 threadGroupsY, uint32 threadGroupsZ)
	{
		ASSERT_ALLOCATOR;
		FlushBindings();

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
		FlushBindings();

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
		FlushBindings();

		CPUDispatchGraphGPUDataCommand* cmd = new CPUDispatchGraphGPUDataCommand();
		cmd->res = buffer->GetResource();
		m_CommandAllocator->SubmitCommand(cmd);
	}

	void CPUCommandList::FlushBindings()
	{
		ASSERT_ALLOCATOR;
		CPUBindDataCommand* cmd = new CPUBindDataCommand();
		cmd->bindings = m_Bindings;
		m_CommandAllocator->SubmitCommand(cmd);

		m_Bindings.Clear();
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

