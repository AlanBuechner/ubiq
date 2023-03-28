#pragma once
#include "Engine/Core/Core.h"
#include "CommandList.h"
#include <vector>

namespace Engine
{

	class ExecutionOrder
	{
	public:
		void Add(Ref<CommandList> commandList, std::vector<Ref<CommandList>> dependencys = {});
		void Remove(Ref<CommandList> commandList); // don't forget to remove all dependence 
		void Clear() { m_Commands.clear(); }

		std::vector<std::vector<Ref<CommandList>>>& GetCommandLists() { return m_Commands; }

		static Ref<ExecutionOrder> Create() { return CreateRef<ExecutionOrder>(); }

	private:
		std::vector<std::vector<Ref<CommandList>>> m_Commands;
	};

	class CommandQueue
	{
	public:
		enum class Type
		{
			Direct = 0,
			Copy = 1,
			Compute = 2,
		};

		void Submit(Ref<CommandList> commandList, uint32 dcount = 0);
		void Submit(Ref<ExecutionOrder> order, uint32 dcount = 0);

		virtual void Execute() = 0;
		virtual void ExecuteImmediate(std::vector<Ref<CommandList>> commandLists) = 0;
		virtual bool InExecution() = 0;

		static Ref<CommandQueue> Create(Type type);

	protected:
		std::vector<std::vector<Ref<CommandList>>> m_Commands;
	};
}
