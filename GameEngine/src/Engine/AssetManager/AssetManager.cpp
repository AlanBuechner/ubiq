#include "pch.h"
#include "AssetManager.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine
{
	void AssetManager::Init(const fs::path& assetDirectory)
	{
		m_RootDirectory = assetDirectory;

		UpdateDirectory(m_RootDirectory);

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

	void AssetManager::UpdateDirectory(const fs::path& dir)
	{
		CORE_INFO("starting directory update");
		// get all assets in filesystem
		std::vector<fs::path> foundAssets, foundMetas;
		ProcessDirectory(dir, foundAssets, foundMetas);

		// match files with meta's
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

}
