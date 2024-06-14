#pragma once
#include "Engine/Core/Core.h"
#include "Utils/Vector.h"

namespace Engine
{
	class CommandList;
}

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

		void PrependSubmit(Ref<CommandList> commandList);
		void PrependSubmit(Utils::Vector<Ref<CommandList>> commandLists);

		void Submit(Ref<CommandList> commandList);
		void Submit(Utils::Vector<Ref<CommandList>> commandLists);

		virtual void Build() = 0;

		virtual void Execute() = 0;
		virtual bool InExecution() = 0;
		virtual void Await() = 0;

		static Ref<CommandQueue> Create(Type type);

	protected:
		Utils::Vector<Ref<CommandList>> m_Commands;
	};
}
