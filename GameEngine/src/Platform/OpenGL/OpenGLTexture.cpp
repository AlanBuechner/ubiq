#include "pch.h"
#include "OpenGLTexture.h"

#include"stb_image.h"

#include <glad/glad.h>

namespace Engine
{
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
	{
		LoadFromFile(path);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::SetWrapMode(WrapMode U, WrapMode V)
	{
		m_WrapU = U;
		m_WrapV = V;
	}

	void OpenGLTexture2D::SetMinMagFilter(MinMagFilter min, MinMagFilter mag)
	{
		m_MinFilter = min;
		m_MagFilter = mag;
	}

	void OpenGLTexture2D::LoadFromFile(const std::string& path)
	{
		m_Path = path;

		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		CORE_ASSERT(data, "Failed to load image \"{0}\"", path);
		m_Width = width;
		m_Height = height;

		int InternalFormat = 0, DataFormat = 0;
		switch (channels)
		{
		case 1:
			InternalFormat = GL_R8;
			DataFormat = GL_R;
			break;
		case 2:
			InternalFormat = GL_RG8;
			DataFormat = GL_RG;
			break;
		case 3:
			InternalFormat = GL_RGB8;
			DataFormat = GL_RGB;
			break;
		case 4:
			InternalFormat = GL_RGBA8;
			DataFormat = GL_RGBA;
			break;
		default:
			break;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, InternalFormat, m_Width, m_Width);

		int minFilter = 0, magFilter = 0;
		switch (m_MinFilter)
		{
		case MinMagFilter::Linear:
			minFilter = GL_LINEAR;
			break;
		case MinMagFilter::Nearest:
			minFilter = GL_NEAREST;
		default:
			break;
		}
		
		switch (m_MagFilter)
		{
		case MinMagFilter::Linear:
			magFilter = GL_LINEAR;
			break;
		case MinMagFilter::Nearest:
			magFilter = GL_NEAREST;
		default:
			break;
		}

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, minFilter);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, magFilter);

		int u_Wrap = 0, v_Wrap = 0;
		switch (m_WrapU)
		{
		case WrapMode::Repeat:
			u_Wrap = GL_REPEAT;
			break;
		case WrapMode::MirroredRepeat:
			u_Wrap = GL_MIRRORED_REPEAT;
			break;
		case WrapMode::ClampToEdge:
			u_Wrap = GL_CLAMP_TO_EDGE;
			break;
		default:
			break;
		}

		switch (m_WrapV)
		{
		case WrapMode::Repeat:
			v_Wrap = GL_REPEAT;
			break;
		case WrapMode::MirroredRepeat:
			v_Wrap = GL_MIRRORED_REPEAT;
			break;
		case WrapMode::ClampToEdge:
			v_Wrap = GL_CLAMP_TO_EDGE;
			break;
		default:
			break;
		}

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, u_Wrap);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, v_Wrap);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, DataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

}