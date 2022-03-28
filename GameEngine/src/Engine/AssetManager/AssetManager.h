#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include <filesystem>
#include <string>
#include <map>

namespace fs = std::filesystem;

namespace Engine
{
	class Asset;
	template<class T>
	class AssetRef;
	class AssetManager;

	class Asset
	{};

	template<class T>
	class AssetRef
	{
	private:
		UUID m_AssetID;
		std::weak_ptr<T> m_Asset;
		AssetManager* m_AssetManager;
	public:

		AssetRef(UUID assetID, AssetManager* assetManager) :
			m_AssetID(assetID), m_AssetManager(assetManager)
		{}

		T* GetAsset()
		{
			if (!m_Asset.expired())
				return m_Asset.data();

			// load asset from asset manager
		}

	};

	class AssetManager
	{
	private:
		fs::path m_RootDirectory;

		std::map<UUID, Ref<Asset>> m_CashedAssets;
		std::map<UUID, fs::path> m_AssetPaths;

	public:
		AssetManager() = default;

		void Init(const fs::path& assetDirectory);

		void DeleteAsset(const fs::path& assetPath);
		void RenameAsset(const fs::path& oldPath, const fs::path& newName);
		void MoveAsset(const fs::path& oldPath, const fs::path& newDir);

		template<class T>
		Ref<T> GetAsset(UUID assetID)
		{
			// find cashed asset
			const auto assetLocation = m_CashedAssets.find(assetID);
			if (assetLocation != m_CashedAssets.end())
				return std::static_pointer_cast<T>(assetLocation->second);

			// load asset
			const auto pathLocation = m_AssetPaths.find(assetID);
			if (pathLocation != m_AssetPaths.end())
				return T::Create(pathLocation->second.string());

			return Ref<T>();
		}

	private:
		void UpdateDirectory(const fs::path& dir);

		fs::path GetValidName(const fs::path& name);
		void ChangePath(const fs::path& oldPath, const fs::path& newPath);

		void ProcessDirectory(const fs::path& directory, std::vector<fs::path>& foundAssets, std::vector<fs::path>& foundMetas);
		UUID GetAssetUUIDFromPath(const fs::path& metaFile);

	};
}
