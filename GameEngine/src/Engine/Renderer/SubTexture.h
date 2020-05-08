#pragma once
#include "Texture.h"

namespace Engine
{
	class SubTexture2D
	{
	public:
		SubTexture2D(Ref<Texture2D> texture, glm::vec2 regions, glm::vec2 position, glm::vec2 size = { 1,1 });

		const Ref<Texture2D> GetTexture() { return m_Texture; }

		const glm::vec2* GetTextCordes() { return m_TexCoords; }

		const static Ref<SubTexture2D> Create(Ref<Texture2D> texture, glm::vec2 regions, glm::vec2 position, glm::vec2 size = { 1,1 });

	private:
		Ref<Texture2D> m_Texture;

		glm::vec2* m_TexCoords;
	};
}