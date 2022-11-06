#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Shader.h"
#include "Texture.h"
#include "ConstantBuffer.h"

namespace Engine
{
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
		};

		Ref<ConstantBuffer> m_Buffer;
	};
}
