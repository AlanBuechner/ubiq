#pragma once
#include "Engine/Renderer/Texture.h"
#include <glad/glad.h>

namespace Engine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path, Ref<TextureAttribute> attrib);
		OpenGLTexture2D(const uint32, const uint32 height, Ref<TextureAttribute> attrib);
		virtual ~OpenGLTexture2D();

		virtual uint32 GetWidth() const override { return m_Width; }
		virtual uint32 GetHeight() const override { return m_Height; }
		virtual uint32 GetRendererID() const override { return m_RendererID; }

		virtual void SetData(void* data, uint32 size) override;
		virtual void Bind(uint32 slot) const override;
		virtual void LoadFromFile(const std::string& path);
		virtual Ref<TextureAttribute> GetAttributes() const override { return m_Attribute; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		std::string m_Path;

		uint32 m_Width, m_Height;
		Ref<TextureAttribute> m_Attribute;
		uint32 m_RendererID;
		GLenum m_InternalFormat, m_Format;
	};
}
