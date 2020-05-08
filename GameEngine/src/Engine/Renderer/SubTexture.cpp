#include "pch.h"
#include "SubTexture.h"

namespace Engine
{
	SubTexture2D::SubTexture2D(Ref<Texture2D> texture, glm::vec2 regions, glm::vec2 position, glm::vec2 size) :
		m_Texture(texture)
	{
		glm::vec2 normalizedPosition = { position.x / regions.x, position.y / regions.y };

		m_TexCoords = new glm::vec2[4];

		m_TexCoords[0] = normalizedPosition;
		m_TexCoords[1] = { (size.x / regions.x) + normalizedPosition.x, normalizedPosition.y };
		m_TexCoords[2] = { (size.x / regions.x) + normalizedPosition.x, (size.y / regions.y) + normalizedPosition.y };
		m_TexCoords[3] = { normalizedPosition.x, (size.y / regions.y) + normalizedPosition.y };
	}

	const Ref<SubTexture2D> SubTexture2D::Create(Ref<Texture2D> texture, glm::vec2 regions, glm::vec2 position, glm::vec2 size)
	{
		return CreateSharedPtr<SubTexture2D>(texture, regions, position, size);
	}

}