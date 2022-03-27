#pragma once
#include "Texture.h"

namespace Engine
{
	class SubTexture2D
	{
	public:
		SubTexture2D(Ref<Texture2D> texture, Math::Vector2 regions, Math::Vector2 position, Math::Vector2 size = { 1,1 });

		const Ref<Texture2D> GetTexture() { return m_Texture; }

		const Math::Vector2* GetTextCordes() { return m_TexCoords; }

		const static Ref<SubTexture2D> Create(Ref<Texture2D> texture, Math::Vector2 regions, Math::Vector2 position, Math::Vector2 size = { 1,1 });

	private:
		Ref<Texture2D> m_Texture;

		Math::Vector2* m_TexCoords;
	};
}
