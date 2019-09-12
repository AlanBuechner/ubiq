#pragma once
#include "Engine/Renderer/Texture.h"

namespace Engine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path, Ref<TextureAttribute> attrib);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void Bind(uint32_t slot) const override;
		virtual void LoadFromFile(const std::string& path);
		virtual Ref<TextureAttribute> GetAttributes() const override { return m_Attribute; }
		virtual glm::vec2 AtlasIndexToPosition(int index) const override;

	private:
		std::string m_Path;

		uint32_t m_Width, m_Height;
		Ref<TextureAttribute> m_Attribute;
		uint32_t m_RendererID;
	};
}