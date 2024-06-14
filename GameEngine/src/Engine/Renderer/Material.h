#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Abstractions/Shader.h"
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
		Ref<Shader> shader;

		inline void* GetData(const std::string& name) { return m_Data->GetDatalocation(name); }

		void Apply();
		Ref<ConstantBuffer> GetBuffer() { return m_Buffer; }

		static Ref<Material> Create(const fs::path& path = "");
		static bool ValidExtention(const fs::path& ext);

	private:
		Utils::Vector<Ref<Texture2D>> m_ReferensedTextures;

		Ref<MaterialData> m_Data;

		Ref<ConstantBuffer> m_Buffer;
	};
}
