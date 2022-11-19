#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Shader.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include <unordered_map>

namespace Engine
{
	class MaterialData
	{
	public:
		MaterialData(std::vector<MaterialParameter>& params);

		std::vector<MaterialParameter>& GetParams() { return m_Params; }
		void* GetDatalocation(const std::string& name) { return m_DataLocations[name]; }
		uint32 GetSize() { return m_Size; }
		const void* GetData() { return m_Data; }

	private:
		std::vector<MaterialParameter> m_Params;

		std::unordered_map<std::string, void*> m_DataLocations;

		uint32 m_Size = 0;
		void* m_Data = nullptr;
	};

	class Material : public Asset
	{
	public:
		Ref<Shader> shader;
		Ref<Texture2D> diffuse;
		Ref<Texture2D> normal;
		Ref<Texture2D> roughness;
		Ref<Texture2D> ao;
		Ref<Texture2D> metal;
		Ref<Texture2D> disp;
		Ref<Texture2D> parallax;
		bool invertParallax;

		void Apply();
		Ref<ConstantBuffer> GetBuffer() { return m_Buffer; }

		static Ref<Material> Create(const fs::path& path = "");
		static bool ValidExtention(const fs::path& ext);

	private:
		struct CBuffData
		{
			uint32 diffuseLoc;
			uint32 normalLoc;
			uint32 roughnessLoc;
			uint32 aoLoc;
			uint32 metalLoc;
			uint32 dispLoc;
			uint32 parallaxLoc;
			BOOL useParallax = FALSE;
			BOOL invertParallax = FALSE;
		};

		std::vector<Ref<Texture>> m_ReferensedTextures;

		Ref<MaterialData> m_Data;

		Ref<ConstantBuffer> m_Buffer;
	};
}
