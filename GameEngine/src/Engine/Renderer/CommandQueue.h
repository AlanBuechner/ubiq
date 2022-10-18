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
		inline void ClearCommandQueue() { m_Commands.clear(); }

		virtual void Execute() = 0;
		virtual bool InExecution() = 0;

		static Ref<CommandQueue> Create(Type type);

	protected:
		std::vector<std::vector<Ref<CommandList>>> m_Commands;
	};
}
