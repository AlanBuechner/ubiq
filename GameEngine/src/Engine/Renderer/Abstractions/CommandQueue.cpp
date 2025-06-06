#include "pch.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Engine/Renderer/Renderer.h"
#include "Utils/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandQueue.h"
#endif

namespace Engine
{

	void CommandQueue::Submit(Ref<CommandList> commandList)
	{
		CREATE_PROFILE_SCOPEI("Submit Command List");
		m_Commands.Push(commandList);
	}

	void CommandQueue::Submit(Utils::Vector<Ref<CommandList>> commandLists)
	{
		CREATE_PROFILE_SCOPEI("Submit Command Lists");
		m_Commands.Reserve(commandLists.Count());
		for(uint32 i = 0; i < commandLists.Count(); i++)
			m_Commands.Push(commandLists[i]);
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
