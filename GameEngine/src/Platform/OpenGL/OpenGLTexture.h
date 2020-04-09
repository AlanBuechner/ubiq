#pragma once
#include "Engine/Renderer/Texture.h"
#include <glad/glad.h>

namespace Engine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path, Ref<TextureAttribute> attrib);
		OpenGLTexture2D(const uint32_t, const uint32_t height, Ref<TextureAttribute> attrib);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Bind(uint32_t slot) const override;
		virtual void LoadFromFile(const std::string& path);
		virtual Ref<TextureAttribute> GetAttributes() const override { return m_Attribute; }
		virtual glm::vec2 AtlasIndexToPosition(int index) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		std::string m_Path;

		uint32_t m_Width, m_Height;
		Ref<TextureAttribute> m_Attribute;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_Format;
	};
}