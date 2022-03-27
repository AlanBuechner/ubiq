#include "pch.h"
#include "SubTexture.h"

namespace Engine
{
	SubTexture2D::SubTexture2D(Ref<Texture2D> texture, Math::Vector2 regions, Math::Vector2 position, Math::Vector2 size) :
		m_Texture(texture)
	{
		Math::Vector2 normalizedPosition = { position.x / regions.x, position.y / regions.y };

		m_TexCoords = new Math::Vector2[4];

		m_TexCoords[0] = normalizedPosition;
		m_TexCoords[1] = { (size.x / regions.x) + normalizedPosition.x, normalizedPosition.y };
		m_TexCoords[2] = { (size.x / regions.x) + normalizedPosition.x, (size.y / regions.y) + normalizedPosition.y };
		m_TexCoords[3] = { normalizedPosition.x, (size.y / regions.y) + normalizedPosition.y };
	}

	const Ref<SubTexture2D> SubTexture2D::Create(Ref<Texture2D> texture, Math::Vector2 regions, Math::Vector2 position, Math::Vector2 size)
	{
		return CreateRef<SubTexture2D>(texture, regions, position, size);
	}

}
