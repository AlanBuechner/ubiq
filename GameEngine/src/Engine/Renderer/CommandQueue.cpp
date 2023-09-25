#include "pch.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandQueue.h"
#endif

namespace Engine
{

	void CommandQueue::PrependSubmit(Ref<CommandList> commandList)
	{
		m_Commands.insert(m_Commands.begin(), commandList);
	}

	void CommandQueue::PrependSubmit(std::vector<Ref<CommandList>> commandLists)
	{
		m_Commands.reserve(commandLists.size());
		for (uint32 i = 0; i < commandLists.size(); i++)
			m_Commands.insert(m_Commands.begin(), commandLists[i]);
	}

	void CommandQueue::Submit(Ref<CommandList> commandList)
	{
		m_Commands.push_back(commandList);
	}

	void CommandQueue::Submit(std::vector<Ref<CommandList>> commandLists)
	{
		m_Commands.reserve(commandLists.size());
		for(uint32 i = 0; i < commandLists.size(); i++)
			m_Commands.push_back(commandLists[i]);
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
