#include "pch.h"
#include "CommandQueue.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandQueue.h"
#endif

namespace Engine
{

	void ExecutionOrder::Add(Ref<CommandList> commandList, std::vector<Ref<CommandList>> dependencys /*= {}*/)
	{
		// find the command list's dependency count
		int dcount = -1;
		for (uint32 i = 0; i < m_Commands.size(); i++)
		{
			for (uint32 j = 0; j < m_Commands[i].size(); j++)
			{
				for (uint32 k = 0; k < dependencys.size(); k++)
				{
					if (m_Commands[i][j] == dependencys[k])
					{
						dcount = (uint32)Math::Max((float)i, (float)dcount); // TODO: make an int version of max
					}
				}
			}
		}

		dcount++;

		// add new dependency list
		if (dcount >= m_Commands.size())
			m_Commands.push_back(std::vector<Ref<CommandList>>());

		m_Commands[dcount].push_back(commandList);
	}

	void ExecutionOrder::Remove(Ref<CommandList> commandList)
	{
		for (uint32 i = 0; i < m_Commands.size(); i++)
		{
			for (uint32 j = 0; j < m_Commands[i].size(); j++)
			{
				if (m_Commands[i][j] == commandList)
				{
					m_Commands[i][j] = m_Commands[i].back();
					m_Commands[i].pop_back();
				}
			}
		}
	}

	void CommandQueue::Submit(Ref<CommandList> commandList, uint32 dcount)
	{
		// add new dependency list
		if (dcount >= m_Commands.size())
			m_Commands.resize(dcount + 1);

		m_Commands[dcount].push_back(commandList);
	}

	void CommandQueue::Submit(Ref<ExecutionOrder> order, uint32 dcount /*= 0*/)
	{
		auto& commands = order->GetCommandLists();
		for (uint32 i = 0; i < commands.size(); i++)
		{
			std::vector<Ref<CommandList>>& list = commands[i];
			for (auto& command : list)
				Submit(command, dcount + i);
		}
	}

	Ref<CommandQueue> CommandQueue::Create(Type type)
	{
		CREATE_PROFILE_FUNCTIONI();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12CommandQueue>(type);
		default:
			break;
		}
		
		return Ref<CommandQueue>();
	}

}
