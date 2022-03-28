#include "pch.h"
#include "AssetManager.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine
{
	void AssetManager::Init(const fs::path& assetDirectory)
	{
		m_RootDirectory = assetDirectory;

		// get all assets in filesystem
		std::vector<fs::path> foundAssets, foundMetas;
		ProcessDirectory(m_RootDirectory, foundAssets, foundMetas);

		// match files with meta's
		for (auto asset : foundAssets)
		{
			auto metai = std::find(foundMetas.begin(), foundMetas.end(), asset.string() + ".meta");
			if (metai != foundMetas.end())
			{
				UUID id = GetUUIDFromMeta(asset.string() + ".meta");
				m_AssetPaths[id] = asset;
				foundMetas.erase(metai);
			}
			else
			{
				UUID id = UUID();
				nlohmann::json j = {
					{"UUID", (uint64)id}
				};
				std::ofstream ofs(asset.string() + ".meta");
				if (ofs.is_open())
					ofs << j.dump() << std::endl;

				m_AssetPaths[id] = asset;
			}
		}

		// delete unused meta files
		for (auto metaf : foundMetas)
			fs::remove(metaf);
	}

	void AssetManager::ProcessDirectory(fs::path directory, std::vector<fs::path>& foundAssets, std::vector<fs::path>& foundMetas)
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

	UUID AssetManager::GetUUIDFromMeta(fs::path metaFile)
	{
		std::ifstream ifs(metaFile);
		if (ifs.is_open())
		{
			nlohmann::json f;
			ifs >> f;

			return UUID(f["UUID"].get<uint64>());
		}
		return 0;
	}

}
