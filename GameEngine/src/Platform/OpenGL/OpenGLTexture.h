#pragma once
#include "Engine/Renderer/Texture.h"

namespace Engine
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D() = default;
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void Bind(uint32_t slot) const override;
		virtual void SetWrapMode(WrapMode U, WrapMode V) override;
		virtual void LoadFromFile(const std::string& path);

	private:
		std::string m_Path;

		uint32_t m_Width, m_Height;
		WrapMode m_WrapU, m_WrapV;
		uint32_t m_RendererID;
	};
}