#include "pch.h"
#include "Material.h"
#include "Renderer.h"
#include "Shaders/ShaderCompiler.h"
#include "Abstractions/Resources/Texture.h"
#include "Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Core/Application.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine
{

	MaterialData::MaterialData(Utils::Vector<MaterialParameter>& params) :
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


	MaterialParameter* MaterialData::GetParam(const std::string& name)
	{
		for (uint32 i = 0; i < m_Params.Count(); i++)
		{
			if (m_Params[i].name == name)
				return &m_Params[i];
		}
		return nullptr;
	}

	void Material::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		if (texture == nullptr)
			return;

		MaterialParameter* param = m_Data->GetParam(name);
		if (param->type == MaterialParameterType::TextureID)
		{
			void* location = m_Data->GetDatalocation(name);
			m_ReferensedTextures[name] = texture;
			uint32 descLoc = texture->GetSRVDescriptor()->GetIndex();
			*(uint32*)location = descLoc; // set asset value
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
			mat->m_Shader = assetManager.GetAsset<Shader>(f["shader"].get<fs::path>());
			mat->DefultInitalize();

			for (auto& p : mat->m_Shader->GetParams())
			{
				void* location = mat->m_Data->GetDatalocation(p.name);
				if (f.contains(p.name))
				{
					if (p.type == MaterialParameterType::TextureID)
						mat->SetTexture(p.name, assetManager.GetAsset<Texture2D>(f[p.name]));
					else if (p.type == MaterialParameterType::Float)
						*(float*)location = f[p.name].get<float>();
					else if (p.type == MaterialParameterType::Float4)
					{
						std::vector<float> data;
						f[p.name].get_to(data);

						uint32 i = 0;
						for (; i < std::min(data.size(), (size_t)4); i++)
							((float*)location)[i] = data[i];

						// fill any missing data with 0
						for (; i < 4; i++)
							((float*)location)[i] = 0;
					}
					else if (p.type == MaterialParameterType::Bool)
						*(BOOL*)location = f[p.name].get<bool>();
				}
			}
		}
		mat->Apply();

		return mat;
	}

	Engine::Ref<Material> Material::Create(Ref<Shader> shader)
	{
		Ref<Material> mat = CreateRef<Material>();
		mat->m_Shader = shader;

		mat->DefultInitalize();
		mat->Apply();

		return mat;
	}

	bool Material::ValidExtention(const fs::path& ext)
	{
		return (ext == ".mat");
	}

	void Material::DefultInitalize()
	{
		m_Data = CreateRef<MaterialData>(m_Shader->GetParams());
		m_Buffer = ConstantBuffer::Create(m_Data->GetSize());
		for (auto& p : m_Shader->GetParams())
		{
			void* location = m_Data->GetDatalocation(p.name);

			if (p.type == MaterialParameterType::TextureID)
			{
				if (p.defaultValue == "white")
					*(uint32*)location = Renderer::GetWhiteTexture()->GetSRVDescriptor()->GetIndex();
				else if (p.defaultValue == "black")
					*(uint32*)location = Renderer::GetBlackTexture()->GetSRVDescriptor()->GetIndex();
				else if (p.defaultValue == "normal")
					*(uint32*)location = Renderer::GetNormalTexture()->GetSRVDescriptor()->GetIndex();
				else
					*(uint32*)location = Renderer::GetWhiteTexture()->GetSRVDescriptor()->GetIndex();
			}
			else if (p.type == MaterialParameterType::Float)
			{
				*(float*)location = std::stof(p.defaultValue);
			}
			else if (p.type == MaterialParameterType::Bool)
			{
				if (p.defaultValue == "true")
					*(BOOL*)location = TRUE;
				else
					*(BOOL*)location = FALSE;
			}
		}

	}

}
