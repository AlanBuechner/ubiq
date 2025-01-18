#pragma once
#include "Engine/Renderer/Abstractions/CommandQueue.h"
#include "Utils/Performance.h"
#include "DX.h"
#include <thread>

namespace Engine
{
	class DirectX12CommandQueue : public CommandQueue
	{
	public:
		DirectX12CommandQueue(Type type);

		virtual void Build() override;
		virtual void Execute() override;
		virtual bool InExecution() override;
		virtual void Await() override;

		inline ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }

	private:
		ID3D12CommandQueue* m_CommandQueue;
		ID3D12Fence1* m_Fence;
		Utils::Vector<ID3D12CommandList*> m_DXCommandLists;
		HANDLE m_EventHandle = nullptr;
		uint32 m_SignalCount = 0;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		Profiler::InstrumentationTimer m_Timer = CREATE_PROFILEI();
	};
}
