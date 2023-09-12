#include "pch.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Engine/Util/Performance.h"

#include "Platform/DirectX12/DirectX12ResourceManager.h"

namespace Engine
{

	ResourceDeletionPool::~ResourceDeletionPool()
	{
		Clear();
	}

	void ResourceDeletionPool::Clear()
	{
		for (uint32 i = 0; i < m_Resources.size(); i++)
			delete m_Resources[i];

		for (uint32 i = 0; i < m_Descriptors.size(); i++)
			delete m_Descriptors[i];

		m_Resources.clear();
		m_Descriptors.clear();
	}

	ResourceManager::ResourceManager()
	{
		m_DeletionPool = new ResourceDeletionPool();
	}

	ResourceManager::~ResourceManager()
	{}


	ResourceDeletionPool* ResourceManager::CreateNewDeletionPool()
	{
		ResourceDeletionPool* pool = m_DeletionPool;
		m_DeletionPool = new ResourceDeletionPool();
		return pool;
	}

}
