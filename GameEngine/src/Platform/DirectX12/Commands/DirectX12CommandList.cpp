#include "pch.h"
#include "Engine/Math/Math.h"
#include "Engine/Renderer/Renderer.h"

#include "DirectX12CommandList.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Platform/DirectX12/DirectX12Shader.h"
#include "Platform/DirectX12/Resources/DirectX12Buffer.h"
#include "Platform/DirectX12/Resources/DirectX12ConstantBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12StructuredBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12Texture.h"
#include "Platform/DirectX12/Resources/DirectX12InstanceBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12ResourceManager.h"

#include "Engine/Renderer/Commands/Commands.h"
#include "Engine/Renderer/GPUProfiler.h"


#include "Engine/Util/Performance.h"
#include "Engine/Renderer/Mesh.h"

namespace Engine
{
	static inline D3D12_RESOURCE_BARRIER TransitionResource(
		_In_ ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
	{
		D3D12_RESOURCE_BARRIER result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = flags;
		result.Transition.pResource = pResource;
		result.Transition.StateBefore = stateBefore;
		result.Transition.StateAfter = stateAfter;
		result.Transition.Subresource = subresource;
		return result;
	}

	static inline D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandList::CommandListType type)
	{
		switch (type)
		{
		case Engine::CommandList::Direct: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case Engine::CommandList::Bundle: return D3D12_COMMAND_LIST_TYPE_BUNDLE;
		}
		return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}

	void DirectX12CommandList::Init()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		D3D12_COMMAND_LIST_TYPE type = GetD3D12CommandListType(m_Type);
	
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_Allocator)),
			"Failed to Create Command List Allocator");
		m_Allocator->SetName(L"Command Allocator");
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandList(0, type, m_Allocator, nullptr, IID_PPV_ARGS(&m_CommandList)),
			"Failed to Create Commnd List");
		m_CommandList->SetName(L"Command List");
		m_CommandList->Close();

		m_RecordFlag.Signal();
	}

	void DirectX12CommandList::Build(std::unordered_map<GPUResource*, ResourceState>& startingStates)
	{
		// init allocator and command list
		CORE_ASSERT_HRESULT(m_Allocator->Reset(), "Failed to reset command allocator");
		CORE_ASSERT_HRESULT(m_CommandList->Reset(m_Allocator, nullptr), "Failed to reset command list");

		ID3D12DescriptorHeap* heaps[]{ DirectX12ResourceManager::s_SRVHeap->GetHeap().Get(), DirectX12ResourceManager::s_SamplerHeap->GetHeap().Get() };
		m_CommandList->SetDescriptorHeaps(2, heaps);

		// get prepend transitions
		RecordPrependCommands((TransitionCommand*)m_Commands[0], startingStates);

		// record commands
		for (uint32 i = 0; i < m_Commands.size(); i++)
			m_Commands[i]->RecordCommand(*this, *m_Commands[i]);

		// close
		m_CommandList->Close();
	}

	void DirectX12CommandList::RecordPrependCommands(TransitionCommand* tcmd, std::unordered_map<GPUResource*, ResourceState>& startingStates)
	{
		std::vector<ResourceStateObject>& pendingTransitions = GetPendingTransitions();
		std::unordered_map<GPUResource*, ResourceState> endingStates = GetEndingResourceStates();

		// get list off all resources that need to be changed
		for (uint32 i = 0; i < pendingTransitions.size(); i++)
		{
			ResourceStateObject& rso = pendingTransitions[i];

			GPUResource* resource = rso.resource;
			ResourceState to = rso.state;
			ResourceState from = startingStates[resource];
			if(from == ResourceState::Unknown)
				from = resource->GetDefultState();

			CORE_ASSERT(resource->SupportState(to), "resouce does not support state");
			if (rso.resource->GetDefultState() != rso.state)
				tcmd->m_Transitions.push_back({ resource, to, from });
		}
	}

}
