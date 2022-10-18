#include "pch.h"
#include "CommandList.h"
#include "Renderer.h"

#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandList.h"
#endif

namespace Engine
{
	Ref<CommandList> Engine::CommandList::Create(CommandListType type)
	{
		CREATE_PROFILE_FUNCTIONI();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12CommandList>(type);
		default:
			break;
		}
		return Ref<CommandList>();
	}

}
