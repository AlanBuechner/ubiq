#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <thread>

namespace Engine
{
	class Asset;
	class AssetManager;

	class Asset
	{
	public:
		inline UUID GetAssetID() { return m_AssetID; }

	private:
		UUID m_AssetID = 0; // 0 means that it was created programmability or was an embedded asset

		friend AssetManager;
	};

	class AssetManager
	{
	private:
		struct AssetPool
		{
			std::unordered_map<fs::path, Ref<Asset>> m_CashedEmbededAssets;
			std::unordered_map<UUID, Ref<Asset>> m_CashedAssets;
		};

		std::unordered_map<uint64, AssetPool> m_CashedAssetPools;
		std::unordered_map<UUID, fs::path> m_AssetPaths;

		std::vector<fs::path> m_AssetDirectories;

		std::thread m_DirectoryWatchThread;

		float m_CleanDelay = 120.0f;
		float m_NextCleanTime = 0.0f;

	public:
		AssetManager() = default;

		void Init();
		void Destroy();

		void Clean();

		void AddAssetDirectory(const fs::path& directory);
		void RemoveAssetDirectory(const fs::path& directory);
		inline const std::vector<fs::path>& GetAssetDirectories() { return m_AssetDirectories; }

		void DeleteAsset(const fs::path& assetPath);
		void RenameAsset(const fs::path& oldPath, const fs::path& newName);
		void MoveAsset(const fs::path& oldPath, const fs::path& newDir);
		fs::path CreateFolder(const fs::path& path);
		UUID GetAssetUUIDFromPath(const fs::path& metaFile);

		template<class T>
		Ref<T> GetAsset(UUID assetID)
		{
			Ref<Asset> asset;

			AssetPool& pool = m_CashedAssetPools[typeid(T).hash_code()];

			// find cashed asset
			const auto assetLocation = pool.m_CashedAssets.find(assetID);
			if (assetLocation != pool.m_CashedAssets.end())
				asset = assetLocation->second;
			else
			{
				// load asset
				const auto pathLocation = m_AssetPaths.find(assetID);
				if (pathLocation != m_AssetPaths.end())
				{
					Ref<T> asset = T::Create(pathLocation->second.string());
					asset->m_AssetID = assetID;
					auto data = std::make_pair(assetID, asset);
					pool.m_CashedAssets.insert(data);
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

		template<class T>
		Ref<T> GetEmbededAsset(const fs::path& path)
		{
			Ref<Asset> asset;

			AssetPool& pool = m_CashedAssetPools[typeid(T).hash_code()];

			const auto AssetLocation = pool.m_CashedEmbededAssets.find(path);
			if (AssetLocation != pool.m_CashedEmbededAssets.end())
				asset = AssetLocation->second;
			else
			{
				fs::path resourcePath = GetEmbededFolder() / path;
				if (!fs::exists(resourcePath))
					return nullptr;

				Ref<T> asset = T::Create(resourcePath.string());
				asset->m_AssetID = 0; // 0 is for a embedded asset
				auto data = std::make_pair(path, asset);
				pool.m_CashedEmbededAssets.insert(data);
				return asset;
			}

			return std::static_pointer_cast<T>(asset);
		}

		inline fs::path GetAssetPath(UUID id) { return m_AssetPaths[id]; }

		fs::path GetRelitiveAssetPath(UUID id);

		void OpenAsset(UUID id);
		void OpenAsset(const fs::path& path);

		fs::path FindAssetDirectory(const fs::path& assetPath);

		fs::path GetEmbededFolder();

	private:
		void UpdateDirectory(const fs::path& dir);

		fs::path GetValidName(const fs::path& name);
		void ChangePath(const fs::path& oldPath, const fs::path& newPath);

		void ProcessDirectory(const fs::path& directory, std::vector<fs::path>& foundAssets, std::vector<fs::path>& foundMetas);

		static void WatchDirectory(AssetManager* assetManager);
	};
}
