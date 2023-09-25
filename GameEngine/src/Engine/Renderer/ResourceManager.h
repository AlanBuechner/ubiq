#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/CommandQueue.h"
#include "Engine/Core/Flag.h"
#include "Engine/Renderer/Resources/Descriptor.h"
#include "Engine/Renderer/Resources/ResourceState.h"
#include <thread>

namespace Engine
{
	class ResourceDeletionPool
	{
	public:
		~ResourceDeletionPool();
		void Clear();

		std::vector<GPUResource*> m_Resources;
		std::vector<Descriptor*> m_Descriptors;
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		ResourceDeletionPool* CreateNewDeletionPool();

		virtual void UploadData() = 0;
		virtual void Clean() = 0;

		virtual std::vector<Ref<CommandList>> GetUploadCommandLists() = 0;

		void ScheduleResourceDeletion(GPUResource* resource) { m_DeletionPool->m_Resources.push_back(resource); }
		void ScheduleHandleDeletion(Descriptor* descriptor) { m_DeletionPool->m_Descriptors.push_back(descriptor); }

	protected:
		ResourceDeletionPool* m_DeletionPool;

	};
}
