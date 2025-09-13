#pragma once
#include "Engine/Renderer/Abstractions/CommandQueue.h"
#include "Utils/Performance.h"
#include "DX.h"
#include <thread>

#include "tracy/TracyD3D12.hpp"

namespace Engine
{
	class DirectX12CommandQueue : public CommandQueue
	{
	public:
		DirectX12CommandQueue(Type type);
		~DirectX12CommandQueue();

		virtual void Execute() override;
		virtual bool InExecution() override;
		virtual void Await() override;

		inline ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }

		TracyD3D12Ctx GetTracyCtx() { return m_TracyCtx; }

	private:
		ID3D12CommandQueue* m_CommandQueue;
		ID3D12Fence1* m_Fence;
		HANDLE m_EventHandle = nullptr;
		uint32 m_SignalCount = 0;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		TracyD3D12Ctx m_TracyCtx = nullptr;
	};
}
