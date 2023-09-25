#include "pch.h"
#include "DirectX12CommandQueue.h"
#include "Directx12Context.h"
#include "DirectX12CommandList.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	DirectX12CommandQueue::DirectX12CommandQueue(Type type)
	{
		static const D3D12_COMMAND_LIST_TYPE types[] = {
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_LIST_TYPE_COPY,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
		};
		m_Type = types[(int)type];

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_COMMAND_QUEUE_DESC qDesc;
		ZeroMemory(&qDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));

		// Setup descriptor
		qDesc.Type = m_Type;
		qDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
		qDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		qDesc.NodeMask = NULL;

		// Create command queue
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommandQueue(&qDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())),
			"Faild To Create Command Queue");

		// create fence and set initial value to 1 to indicate it is not currently executing
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())),
			"Faild to Create Fence");
	}

	void DirectX12CommandQueue::Build()
	{
		m_DXCommandLists.reserve(m_Commands.size());
		for (uint32 i = 0; i < m_Commands.size(); i++)
		{
			Ref<DirectX12CommandList> dxCmdList = std::dynamic_pointer_cast<DirectX12CommandList>(m_Commands[i]);

			if (dxCmdList->RecordPrependCommands())
				m_DXCommandLists.push_back(dxCmdList->GetPrependCommandList().Get());
			dxCmdList->InternalClose();
			m_DXCommandLists.push_back(dxCmdList->GetCommandList().Get());
		}
	}

	void DirectX12CommandQueue::Execute()
	{
		CREATE_PROFILE_FUNCTIONI();
		
		m_CommandQueue->ExecuteCommandLists((uint32)m_DXCommandLists.size(), m_DXCommandLists.data());
		m_CommandQueue->Signal(m_Fence.Get(), ++m_SignalCount); // signal fence when execution has finished

		for (uint32 i = 0; i < m_Commands.size(); i++)
			std::dynamic_pointer_cast<DirectX12CommandList>(m_Commands[i])->SignalRecording();

		m_Commands.clear();
		m_DXCommandLists.clear();
	}

	// returns wrong value some times
	bool DirectX12CommandQueue::InExecution()
	{
		return m_Fence->GetCompletedValue() != 0;
	}

	void DirectX12CommandQueue::Await()
	{
		m_Fence->SetEventOnCompletion(m_SignalCount, m_EventHandle); // call event when fence val has been reached
		WaitForSingleObject(m_EventHandle, INFINITE); // wait for event to be triggered
		m_EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS); // create event for command queue completion
		m_SignalCount = 0;
		m_Fence->Signal(0);
	}

}
