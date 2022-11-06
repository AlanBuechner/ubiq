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

		if (roughness) data.roughnessLoc = roughness->GetDescriptorLocation();
		else data.roughnessLoc = Renderer::GetBlackexture()->GetDescriptorLocation();

		if (ao) data.aoLoc = ao->GetDescriptorLocation();
		else data.aoLoc = Renderer::GetWhiteTexture()->GetDescriptorLocation();

		if (metal) data.metalLoc = metal->GetDescriptorLocation();
		else Renderer::GetBlackexture()->GetDescriptorLocation();

		if (disp) data.dispLoc = disp->GetDescriptorLocation();
		else data.dispLoc = Renderer::GetBlackexture()->GetDescriptorLocation();

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

#define GET_TEXTURE_ATTRIB(name) if(f.contains(#name)) mat->name = assetManager.GetAsset<Texture2D>(f[#name]);

			GET_TEXTURE_ATTRIB(diffuse);
			GET_TEXTURE_ATTRIB(normal);
			GET_TEXTURE_ATTRIB(roughness);
			GET_TEXTURE_ATTRIB(ao);
			GET_TEXTURE_ATTRIB(metal);
			GET_TEXTURE_ATTRIB(disp);

#undef GET_TEXTURE_ATTRIB
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
