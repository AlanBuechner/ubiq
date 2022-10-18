#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/CommandQueue.h"
#include "Engine/Core/Flag.h"
#include <thread>

namespace Engine
{
	class ResourceDeletionPool
	{
	public:
		virtual void Clear() = 0;
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		virtual Ref<ResourceDeletionPool> CreateNewDeletionPool() = 0;

		virtual void RecordCommands(Ref<CommandList> commandList) = 0;
	private:
	};
}
