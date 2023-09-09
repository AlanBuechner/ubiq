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
		void Clear();

		std::vector<GPUResource*> m_Resources;
		std::vector<Descriptor*> m_Descriptor;
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		ResourceDeletionPool* CreateNewDeletionPool();

		virtual void UploadData() = 0;

		void ScheduleResourceDeletion(GPUResource* resource) { m_DeletionPool->m_Resources.push_back(resource); }
		void ScheduleHandleDeletion(Descriptor* descriptor) { m_DeletionPool->m_Descriptor.push_back(descriptor); }
	private:

		ResourceDeletionPool* m_DeletionPool;

	};
}
