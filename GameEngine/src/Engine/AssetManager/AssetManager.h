#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <thread>

namespace fs = std::filesystem;

namespace Engine
{
	class Asset;
	class AssetManager;

	class Asset
	{};

	class AssetManager
	{
	private:
		fs::path m_RootDirectory;

		std::unordered_map<UUID, Ref<Asset>> m_CashedAssets;
		std::unordered_map<UUID, fs::path> m_AssetPaths;

		std::thread m_DirectoryWatchThread;

		float m_CleanDelay = 120.0f;
		float m_NextCleanTime = 0.0f;

	public:
		AssetManager() = default;

		void Init(const fs::path& assetDirectory);
		void Destroy();

		void Clean();

		void DeleteAsset(const fs::path& assetPath);
		void RenameAsset(const fs::path& oldPath, const fs::path& newName);
		void MoveAsset(const fs::path& oldPath, const fs::path& newDir);
		fs::path CreateFolder(const fs::path& path);
		UUID GetAssetUUIDFromPath(const fs::path& metaFile);

		template<class T>
		Ref<T> GetAsset(UUID assetID)
		{
			Ref<Asset> asset;

			// find cashed asset
			const auto assetLocation = m_CashedAssets.find(assetID);
			if (assetLocation != m_CashedAssets.end())
				asset = assetLocation->second;
			else
			{
				// load asset
				const auto pathLocation = m_AssetPaths.find(assetID);
				if (pathLocation != m_AssetPaths.end())
				{
					auto data = std::make_pair(assetID, T::Create(pathLocation->second.string()));
					m_CashedAssets.insert(data);
					return data.second;
				}
			}

			return std::static_pointer_cast<T>(asset);
		}

		template<class T>
		Ref<T> GetAsset(const fs::path& path)
		{
			return GetAsset<T>(GetAssetUUIDFromPath(path));
		}

	private:
		void UpdateDirectory(const fs::path& dir);

		fs::path GetValidName(const fs::path& name);
		void ChangePath(const fs::path& oldPath, const fs::path& newPath);

		void ProcessDirectory(const fs::path& directory, std::vector<fs::path>& foundAssets, std::vector<fs::path>& foundMetas);

		static void WatchDirectory(AssetManager* assetManager);
	};
}