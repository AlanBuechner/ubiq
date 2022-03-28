#include "pch.h"
#include "AssetManager.h"

namespace Engine
{
	void AssetManager::Init(const fs::path& assetDirectory)
	{
		m_RootDirectory = assetDirectory;
		ProcessDirectory(m_RootDirectory);
	}

	void AssetManager::ProcessDirectory(fs::path directory)
	{
		for (auto& p : fs::directory_iterator(directory))
		{
			if (p.is_directory())
				ProcessDirectory(p.path());
			else
			{

			}
		}
	}

}
