#include "pch.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Engine/Util/Performance.h"

#include "Platform/DirectX12/DirectX12ResourceManager.h"

namespace Engine
{

	void ResourceDeletionPool::Clear()
	{
		for (GPUResource* res : m_Resources)
			delete res;

		for (Descriptor* desc : m_Descriptor)
			delete desc;

		m_Resources.clear();
		m_Resources.clear();
	}

	ResourceManager::ResourceManager()
	{
		m_DeletionPool = new ResourceDeletionPool();
	}

	ResourceManager::~ResourceManager()
	{
		delete m_DeletionPool;
	}


	ResourceDeletionPool* ResourceManager::CreateNewDeletionPool()
	{
		ResourceDeletionPool* pool = m_DeletionPool;
		m_DeletionPool = new ResourceDeletionPool();
		return pool;
	}

}
