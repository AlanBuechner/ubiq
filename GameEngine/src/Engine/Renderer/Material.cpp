#include "pch.h"
#include "Material.h"
#include "Renderer.h"
#include "Engine/Core/Application.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine
{

	MaterialData::MaterialData(std::vector<MaterialParameter>& params) :
		m_Params(params)
	{
		// calculate size of buffer
		for (auto& p : params)
			m_Size += p.GetTypeSize();

		// create buffer
		m_Data = malloc(m_Size);

		uint64 offset = 0;
		for (auto& p : params)
		{
			m_DataLocations[p.name] = (char*)m_Data + offset;
			offset += p.GetTypeSize();
		}

	}


	void Material::Apply()
	{
		m_Buffer->SetData(m_Data->GetData());
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
			mat->m_Data = CreateRef<MaterialData>(mat->shader->GetParams());

			for (auto& p : mat->shader->GetParams())
			{
				void* location = mat->m_Data->GetDatalocation(p.name);
				if (f.contains(p.name))
				{
					if (p.type == MaterialParameter::TextureID)
					{
						mat->m_ReferensedTextures.push_back(assetManager.GetAsset<Texture2D>(f[p.name])); // get asset
						*(uint32*)location = mat->m_ReferensedTextures.back()->GetDescriptorLocation(); // set asset value
					}
					else if (p.type == MaterialParameter::Bool)
						*(BOOL*)location = f[p.name].get<bool>();
				}
				else
				{
					if (p.type == MaterialParameter::TextureID)
					{
						if (p.defaultValue == "white")
							*(uint32*)location = Renderer::GetWhiteTexture()->GetDescriptorLocation();
						else if (p.defaultValue == "black")
							*(uint32*)location = Renderer::GetBlackTexture()->GetDescriptorLocation();
						else if (p.defaultValue == "blue")
							*(uint32*)location = Renderer::GetBlueTexture()->GetDescriptorLocation();
						else
							*(uint32*)location = Renderer::GetWhiteTexture()->GetDescriptorLocation();
					}
					else if (p.type == MaterialParameter::Bool)
					{
						if (p.defaultValue == "true")
							*(BOOL*)location = TRUE;
						else
							*(BOOL*)location = FALSE;
					}
				}
			}
		}

		mat->m_Buffer = ConstantBuffer::Create(mat->m_Data->GetSize());
		mat->Apply();

		return mat;
	}

	bool Material::ValidExtention(const fs::path& ext)
	{
		return (ext == ".mat");
	}

}
