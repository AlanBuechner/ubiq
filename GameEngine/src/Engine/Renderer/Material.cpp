#include "pch.h"
#include "Material.h"
#include "Renderer.h"
#include "Shaders/ShaderCompiler.h"
#include "Abstractions/Resources/Texture.h"
#include "Abstractions/Resources/ConstantBuffer.h"
#include "Engine/Core/Application.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "Engine/Core/ObjectDescription/JsonObjectDescription.h"

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
		CREATE_PROFILE_FUNCTIONI();
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

			ObjectDescription desc = LoadObjectDescriptionFromJson(f);

			AssetManager& assetManager = Application::Get().GetAssetManager();
			ProjectManager::Project& project = Engine::Application::Get().GetProject();

			// get shader
			CORE_ASSERT(desc.HasEntery("shader"), "Material does not have a shader");
			auto& node = desc["shader"];
			if (node.IsString())
			{
				std::string path = node.Get<std::string>();
				mat->m_Shader = assetManager.GetEmbededAsset<Shader>(path);
				if (mat->GetShader() == nullptr) // check if getting as embedded was successful
				{
					fs::path paths[] = {
						assetManager.GetAssetDirectory() / path,
						project.GetRootDirectory() / path
					};
					for (const fs::path& fullPath : paths)
					{
						if(fs::exists(fullPath))
							mat->m_Shader = assetManager.GetAsset<Shader>(fullPath);
					}
				}
			}
			else if(desc.IsInt())
				mat->m_Shader = assetManager.GetAsset<Shader>((Engine::UUID)node.Get<uint64>()); // get asset from uuid
			if (mat->m_Shader == nullptr)
				return nullptr;
			
			// initialize material
			mat->DefultInitalize();

			// set shader data
			for (auto& p : mat->m_Shader->GetParams())
			{
				void* location = mat->m_Data->GetDatalocation(p.name);
				if (desc.HasEntery(p.name))
				{
					auto& fp = desc[p.name];
					if (p.type == MaterialParameterType::TextureID)
					{
						if (fp.IsInt())
							mat->SetTexture(p.name, assetManager.GetAsset<Texture2D>(fp.Get<uint64>()));
						else if(fp.IsString())
							mat->SetTexture(p.name, assetManager.GetAsset<Texture2D>(fp.Get<std::string>()));

					}
					else if (p.type == MaterialParameterType::Float && fp.IsNumber())
						*(float*)location = fp.Get<float>();
					else if (p.type == MaterialParameterType::Float4 && fp.IsArray())
					{
						Utils::Vector<float> data = fp.Get<Utils::Vector<float>>();

						uint32 i = 0;
						for (; i < std::min(data.Count(), (uint32)4); i++)
							((float*)location)[i] = data[i];

						// fill any missing data with 0
						for (; i < 4; i++)
							((float*)location)[i] = 0;
					}
					else if (p.type == MaterialParameterType::Bool)
						*(BOOL*)location = fp.Get<bool>();
				}
			}
		}
		mat->Apply();

		return mat;
	}

	Engine::Ref<Material> Material::Create(Ref<Shader> shader)
	{
		CREATE_PROFILE_FUNCTIONI();
		ANOTATE_PROFILEI("Shader ID: " + std::to_string(shader->GetAssetID()));
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
		CREATE_PROFILE_FUNCTIONI();
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
