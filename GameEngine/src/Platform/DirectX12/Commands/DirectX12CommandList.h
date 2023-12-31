#pragma once
#include "Engine/Renderer/Commands/CommandList.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	struct TransitionCommand;
}

namespace Engine
{
	class DirectX12CommandList : public CommandList
	{
	public:
		DirectX12CommandList(CommandListType type) :
			CommandList(type)
		{ Init(); }

	private:
		void Init();

	public:
		virtual void Build(std::unordered_map<GPUResource*, ResourceState>& startingStates) override;

		ID3D12GraphicsCommandList4* GetCommandList() { return m_CommandList; }

	private:
		void RecordPrependCommands(TransitionCommand* tcmd, std::unordered_map<GPUResource*, ResourceState>& startingStates);

	private:
		ID3D12CommandAllocator* m_Allocator;
		ID3D12GraphicsCommandList4* m_CommandList;
	};
}
