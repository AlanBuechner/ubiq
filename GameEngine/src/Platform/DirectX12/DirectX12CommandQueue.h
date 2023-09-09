#pragma once
#include "Engine/Renderer/CommandQueue.h"
#include "Engine/Util/Performance.h"
#include "DX.h"
#include <thread>

namespace Engine
{
	class DirectX12CommandQueue : public CommandQueue
	{
	public:
		DirectX12CommandQueue(Type type);

		virtual void Execute() override;
		virtual void ExecuteImmediate(std::vector<Ref<CommandList>> commandLists) override;
		virtual bool InExecution() override;

		inline wrl::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_CommandQueue; }

	private:
		wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		wrl::ComPtr<ID3D12Fence1> m_Fence;
		std::thread m_ExcutionThread;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		InstrumentationTimer m_Timer = CREATE_PROFILEI();
	};
}
