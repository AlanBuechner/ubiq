#include "pch.h"
#include "Material.h"
#include "Renderer.h"
#include "Engine/Core/Application.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine
{

	void Material::Apply()
	{
		CBuffData data;
		data.diffuseLoc = diffuse->GetDescriptorLocation();
		if (normal) data.normalLoc = normal->GetDescriptorLocation();
		else data.normalLoc = Renderer::GetBlueTexture()->GetDescriptorLocation();

		if (specular) data.specularLoc = specular->GetDescriptorLocation();
		else data.specularLoc = Renderer::GetBlackexture()->GetDescriptorLocation();

		m_Buffer->SetData((const void*)&data);
	}

	Ref<Material> Material::Create(const fs::path& path)
	{
		Ref<Material> mat = CreateRef<Material>();

		std::ifstream ifs(path.string());
		if (ifs.is_open())
		{
			nlohmann::json f;
			ifs >> f;

			AssetManager& assetManager = Application::Get().GetAssetManager();

			CORE_ASSERT(f.contains("shader"), "Material does not have a shader");
			mat->shader = assetManager.GetAsset<Shader>(f["shader"].get<fs::path>());

			if (f.contains("diffuse"))
				mat->diffuse = assetManager.GetAsset<Texture2D>(f["diffuse"]);

			if (f.contains("normal"))
				mat->normal = assetManager.GetAsset<Texture2D>(f["normal"]);

			if (f.contains("specular"))
				mat->specular = assetManager.GetAsset<Texture2D>(f["specular"]);
		}

		mat->m_Buffer = ConstantBuffer::Create(sizeof(CBuffData));
		mat->Apply();

		return mat;
	}

	bool Material::ValidExtention(const fs::path& ext)
	{
		return (ext == ".mat");
	}

}
