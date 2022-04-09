#include "pch.h"
#include "AssetManager.h"
#include "Engine/Core/Application.h"
#include "Engine/Util/Utils.h"
#include "Engine/Core/Time.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <Windows.h>
#include <shlobj_core.h>

namespace Engine
{
	void AssetManager::Init(const fs::path& assetDirectory)
	{
		m_RootDirectory = assetDirectory;
		UpdateDirectory(m_RootDirectory);
		m_DirectoryWatchThread = std::thread(WatchDirectory, this);
	}

	void AssetManager::Destroy()
	{
		m_DirectoryWatchThread.detach();
	}

	void AssetManager::Clean()
	{
		if (Time::GetTime() > m_NextCleanTime)
		{
			CORE_INFO("cleaning cashed assets");
			std::vector<std::unordered_map<UUID, Ref<Asset>>::iterator> unusedAssets;

			for (std::unordered_map<UUID, Ref<Asset>>::iterator asset = m_CashedAssets.begin(); asset != m_CashedAssets.end(); asset++)
			{
				if (asset->second.use_count() == 1)
					unusedAssets.push_back(asset);
			}

			for (auto& asset : unusedAssets)
				m_CashedAssets.erase(asset);

			m_NextCleanTime = Time::GetTime() + m_CleanDelay;
		}
	}

	void AssetManager::DeleteAsset(const fs::path& assetPath)
	{
		// check if is asset or directory
		if (assetPath.extension().string() != "")
		{
			fs::remove(assetPath);
			UUID id = GetAssetUUIDFromPath(assetPath);
			fs::remove(assetPath.string() + ".meta");
			m_AssetPaths.erase(m_AssetPaths.find(id));
		}
		else
		{
			fs::remove_all(assetPath);
			m_AssetPaths.clear();
			UpdateDirectory(m_RootDirectory);
		}
	}

	void AssetManager::RenameAsset(const fs::path& oldPath, const fs::path& newName)
	{
		ChangePath(oldPath, GetValidName(oldPath.parent_path() / newName));
	}

	void AssetManager::MoveAsset(const fs::path& oldPath, const fs::path& newDir)
	{
		ChangePath(oldPath, GetValidName(newDir / oldPath.filename()));
	}

	fs::path AssetManager::CreateFolder(const fs::path& path)
	{
		fs::path newPath = GetValidName(path);
		fs::create_directories(newPath);
		return newPath;
	}

	void AssetManager::UpdateDirectory(const fs::path& dir)
	{
		if (!fs::exists(dir))
		{
			fs::create_directory(dir);
			return;
		}

		CORE_INFO("starting directory update");
		// get all assets in filesystem
		std::vector<fs::path> foundAssets, foundMetas;
		ProcessDirectory(dir, foundAssets, foundMetas);

		// match files with meta files
		for (auto asset : foundAssets)
		{
			auto metai = std::find(foundMetas.begin(), foundMetas.end(), asset.string() + ".meta");
			if (metai != foundMetas.end())
			{
				UUID id = GetAssetUUIDFromPath(asset);
				m_AssetPaths[id] = asset;
				foundMetas.erase(metai);
			}
			else
			{
				UUID id = UUID();
				nlohmann::json j = {
					{ "UUID", (uint64)id }
				};
				std::ofstream ofs(asset.string() + ".meta");
				if (ofs.is_open())
					ofs << j.dump() << std::endl;

				m_AssetPaths[id] = asset;
			}
			CORE_INFO(asset.string());
		}

		// delete unused meta files
		for (auto metaf : foundMetas)
			fs::remove(metaf);
	}

	fs::path AssetManager::GetValidName(const fs::path& file)
	{
		fs::path name = file.filename().stem();
		fs::path ext = file.filename().extension();
		fs::path dir = file.parent_path();

		fs::path newPath = dir / (name.string() + ext.string());
		for (uint32 i = 1; fs::exists(newPath); i++)
			newPath = dir / (name.string() + " (" + std::to_string(i) + ")" + ext.string());

		return newPath;
	}

	void AssetManager::ChangePath(const fs::path& oldPath, const fs::path& newPath)
	{
		fs::rename(oldPath, newPath);

		// check if is asset or directory
		if (oldPath.extension().string() != "")
		{
			UUID id = GetAssetUUIDFromPath(oldPath);
			fs::rename(oldPath.string() + ".meta", newPath.string() + ".meta");
			m_AssetPaths[id] = newPath;
		}
		else
			UpdateDirectory(newPath);
	}

	void AssetManager::ProcessDirectory(const fs::path& directory, std::vector<fs::path>& foundAssets, std::vector<fs::path>& foundMetas)
	{
		for (auto& p : fs::directory_iterator(directory))
		{
			if (p.is_directory())
				ProcessDirectory(p.path(), foundAssets, foundMetas);
			else if (p.path().extension().string() != ".meta")
				foundAssets.push_back(p.path());
			else if (p.path().extension().string() == ".meta")
				foundMetas.push_back(p.path());
		}
	}

	UUID AssetManager::GetAssetUUIDFromPath(const fs::path& metaFile)
	{
		std::ifstream ifs(metaFile.string() + ".meta");
		if (ifs.is_open())
		{
			nlohmann::json f;
			ifs >> f;

			return UUID(f["UUID"].get<uint64>());
		}
		return 0;
	}

	// TODO
	void AssetManager::WatchDirectory(AssetManager* assetManager)
	{
		return;

#if defined(PLATFORM_WINDOWS)
		FILE_NOTIFY_INFORMATION strFileNotifyInfo[1024];
		DWORD dwBytesReturned = 0;

		HANDLE hDir = CreateFile(
			assetManager->m_RootDirectory.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL
		);

		if (hDir == INVALID_HANDLE_VALUE)
		{
			CORE_ERROR("Failed to get handle to root directorym {0}", GetLastError());
			return;
		}

		DWORD changeFlags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE;

		while (true)
		{
			if (ReadDirectoryChangesW(hDir, (LPVOID)&strFileNotifyInfo, sizeof(strFileNotifyInfo), 
				TRUE, changeFlags, &dwBytesReturned, NULL, NULL) != 0)
			{
				FILE_NOTIFY_INFORMATION& info = strFileNotifyInfo[0];
				switch (info.Action)
				{
				case FILE_ACTION_ADDED:
				case FILE_ACTION_MODIFIED:
					assetManager->UpdateDirectory(assetManager->m_RootDirectory);
					break;
				default:
					break;
				}
			}
		}
#endif
	}

}
