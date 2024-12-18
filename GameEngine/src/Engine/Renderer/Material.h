#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Shaders/Shader.h"
#include <unordered_map>

namespace Engine
{
	class Shader;
	class Texture2D;
	class ConstantBuffer;
}

namespace Engine
{
	class MaterialData
	{
	public:
		MaterialData(Utils::Vector<MaterialParameter>& params);

		Utils::Vector<MaterialParameter>& GetParams() { return m_Params; }
		MaterialParameter* GetParam(const std::string& name);
		void* GetDatalocation(const std::string& name) { return m_DataLocations[name]; }
		uint32 GetSize() { return m_Size; }
		const void* GetData() { return m_Data; }

	private:
		Utils::Vector<MaterialParameter> m_Params;

		std::unordered_map<std::string, void*> m_DataLocations;

		uint32 m_Size = 0;
		void* m_Data = nullptr;
	};

	class Material : public Asset
	{
	public:

		inline void* GetData(const std::string& name) { return m_Data->GetDatalocation(name); }

		Ref<ConstantBuffer> GetBuffer() { return m_Buffer; }
		Ref<Shader> GetShader() { return m_Shader; }

		void SetTexture(const std::string& name, Ref<Texture2D> texture);

		void Apply();

		static Ref<Material> Create(const fs::path& path = "");
		static Ref<Material> Create(Ref<Shader> shader);
		static bool ValidExtention(const fs::path& ext);

	private:
		void DefultInitalize();

	private:
		Ref<Shader> m_Shader;
		std::unordered_map<std::string, Ref<Texture2D>> m_ReferensedTextures;

		Ref<MaterialData> m_Data;

		Ref<ConstantBuffer> m_Buffer;
	};
}
