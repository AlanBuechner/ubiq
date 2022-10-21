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
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())),
			"Faild to Create Fence");
	}

	void DirectX12CommandQueue::Execute()
	{
		CREATE_PROFILE_FUNCTIONI();
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS); // create event for command queue completion
		m_Fence->Signal(0);
		for (uint32 d = 0; d < m_Commands.size(); d++)
		{
			std::vector<ID3D12CommandList*> cmdList(m_Commands[d].size());
			for (uint32 i = 0; i < m_Commands[d].size(); i++)
				cmdList[i] = std::dynamic_pointer_cast<DirectX12CommandList>(m_Commands[d][i])->GetCommandList().Get();

			m_Fence->SetEventOnCompletion(d, eventHandle); // call event when fence val has been reached
			WaitForSingleObject(eventHandle, INFINITE); // wait for event to be triggered

			m_CommandQueue->ExecuteCommandLists((uint32)cmdList.size(), cmdList.data());
			m_CommandQueue->Signal(m_Fence.Get(), d+1); // signal fence when execution has finished

			for (uint32 i = 0; i < m_Commands[d].size(); i++)
				m_Commands[d][i]->SignalRecording();
		}

		m_Fence->SetEventOnCompletion(m_Commands.size(), eventHandle); // call event when fence val has been reached
		WaitForSingleObject(eventHandle, INFINITE); // wait for event to be triggered

	}

	void DirectX12CommandQueue::ExecuteImmediate(std::vector<Ref<CommandList>> commandLists)
	{
		CREATE_PROFILE_FUNCTIONI();
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS); // create event for command queue completion
		m_Fence->Signal(0);

		std::vector<ID3D12CommandList*> cmdList(commandLists.size());
		for (uint32 i = 0; i < commandLists.size(); i++)
			cmdList[i] = std::dynamic_pointer_cast<DirectX12CommandList>(commandLists[i])->GetCommandList().Get();

		m_CommandQueue->ExecuteCommandLists((uint32)cmdList.size(), cmdList.data());
		m_CommandQueue->Signal(m_Fence.Get(), 1);

		for (uint32 i = 0; i < commandLists.size(); i++)
			commandLists[i]->SignalRecording();

		m_Fence->SetEventOnCompletion(1, eventHandle); // call event when fence val has been reached
		WaitForSingleObject(eventHandle, INFINITE); // wait for event to be triggered
	}

	bool DirectX12CommandQueue::InExecution()
	{
		return (bool)m_Fence->GetCompletedValue();
	}

}
