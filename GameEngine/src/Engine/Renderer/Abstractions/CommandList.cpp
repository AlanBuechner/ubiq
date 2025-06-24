#include "pch.h"
#include "CommandList.h"
#include "Engine/Renderer/Renderer.h"

#include "Utils/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandList.h"
#endif


std::unordered_map<Engine::CommandListType, Utils::Vector<Engine::Ref<Engine::CommandList>>> Engine::CommandList::s_CommandListPools;
std::mutex Engine::CommandList::s_GetCommandListMutex;

namespace Engine
{
	Ref<CommandList> CommandList::Create(CommandListType commandListType)
	{
		CREATE_PROFILE_FUNCTIONI();
		std::lock_guard l(s_GetCommandListMutex);
		Utils::Vector<Ref<CommandList>>& pool = s_CommandListPools[commandListType];

		if (pool.Empty())
			return CreateInternal(commandListType);
		return pool.Pop();
	}

	void CommandList::Free(Ref<CommandList> commandList)
	{
		std::lock_guard l(s_GetCommandListMutex);
		Utils::Vector<Ref<CommandList>>& pool = s_CommandListPools[commandList->m_CommandListType];
		pool.Push(commandList);
	}

	Ref<CommandList> CommandList::CreateInternal(CommandListType commandListType)
	{
		CREATE_PROFILE_FUNCTIONI();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12CommandList>(commandListType);
		default:
			break;
		}
		return Ref<CommandList>();
	}
}
