#pragma once
#include "Engine/Core/Core.h"
#include "CommandList.h"
#include <vector>

namespace Engine
{
	class CommandQueue
	{
	public:
		enum class Type
		{
			Direct = 0,
			Copy = 1,
			Compute = 2,
		};

		void AddCommandList(Ref<CommandList> commanList, std::vector<Ref<CommandList>> dependencys = {});
		void RemoveCommandLIst(Ref<CommandList> commandList); // don't forget to remove all dependence 
		inline void ClearCommandQueue() { m_Commands.clear(); }

		virtual void Execute() = 0;
		virtual void ExecuteImmediate(std::vector<Ref<CommandList>> commandLists) = 0;
		virtual bool InExecution() = 0;

		static Ref<CommandQueue> Create(Type type);

	protected:
		std::vector<std::vector<Ref<CommandList>>> m_Commands;
	};
}
