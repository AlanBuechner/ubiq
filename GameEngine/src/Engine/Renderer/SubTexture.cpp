#include "pch.h"
#include "SubTexture.h"

namespace Engine
{
	SubTexture2D::SubTexture2D(Ref<Texture2D> texture, Math::Vector2 regions, Math::Vector2 position, Math::Vector2 size) :
		m_Texture(texture)
	{
		m_Position = { position.x / regions.x, position.y / regions.y };
		m_Size = { size.x / regions.x, size.y / regions.y };
	}

	const Ref<SubTexture2D> SubTexture2D::Create(Ref<Texture2D> texture, Math::Vector2 regions, Math::Vector2 position, Math::Vector2 size)
	{
		return CreateRef<SubTexture2D>(texture, regions, position, size);
	}

}
