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
		Ref<Texture2D> specular;

		void Apply();
		Ref<ConstantBuffer> GetBuffer() { return m_Buffer; }

		static Ref<Material> Create(const fs::path& path = "");
		static bool ValidExtention(const fs::path& ext);

	private:
		struct CBuffData
		{
			uint32 diffuseLoc;
			uint32 normalLoc;
			uint32 specularLoc;
		};

		Ref<ConstantBuffer> m_Buffer;
	};
}
