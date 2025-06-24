#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/CPUCommandList.h"

namespace Engine
{
	class CommandList
	{
	protected:
		CommandList(CommandListType commandListType) :
			m_CommandListType(commandListType)
		{}

	public:

		virtual void StartRecording() = 0;
		virtual void RecoredCommands(CPUCommandAllocator* commandAllocator) = 0;
		virtual void Close() = 0;

		static Ref<CommandList> Create(CommandListType commandListType);
		static void Free(Ref<CommandList> commandList);

	private:
		static Ref<CommandList> CreateInternal(CommandListType commandListType);

	protected:
		CommandListType m_CommandListType;


		static std::unordered_map<CommandListType, Utils::Vector<Ref<CommandList>>> s_CommandListPools;
		static std::mutex s_GetCommandListMutex;

		friend class CommandQueue;
	};
}
