#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/UUID.h"
#include <filesystem>
#include <string>
#include <map>

namespace fs = std::filesystem;

namespace Engine
{
	class Asset
	{};

	class AssetManager
	{
	private:
		fs::path m_RootDirectory;

		std::map<UUID, Ref<Asset>> m_CashedAssets;
		std::map<UUID, fs::path> m_AssetPaths;

	public:
		AssetManager() = default;

		void Init(const fs::path& assetDirectory);

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
		void ProcessDirectory(fs::path directory);


	};
}
