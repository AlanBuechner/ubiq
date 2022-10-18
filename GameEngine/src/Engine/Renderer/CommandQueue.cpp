#include "pch.h"
#include "CommandQueue.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandQueue.h"
#endif

namespace Engine
{

	void CommandQueue::AddCommandList(Ref<CommandList> commandList, std::vector<Ref<CommandList>> dependencys)
	{
		// find the command list's dependency count
		uint32 dcount = 0;
		for (uint32 i = 0; i < m_Commands.size(); i++)
		{
			for (uint32 j = 0; j < m_Commands[i].size(); j++)
			{
				for (uint32 k = 0; k < dependencys.size(); k++)
				{
					if (m_Commands[i][j] == dependencys[k])
					{
						dcount = (uint32)Math::Max((float)m_Commands[i][j]->m_DependencyCount+1, (float)dcount); // TODO: make an int version of max
					}
				}
			}
		}

		// add new dependency list
		if (dcount >= m_Commands.size())
			m_Commands.push_back(std::vector<Ref<CommandList>>());

		commandList->m_DependencyCount = dcount;

		m_Commands[dcount].push_back(commandList);
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
