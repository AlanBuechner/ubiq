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

	void OpenGLTexture2D::LoadFromFile(const std::string& path)
	{
		m_Path = path;

		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		CORE_ASSERT(data, "Failed to load image \"{0}\"", path);
		m_Width = width;
		m_Height = height;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Width, m_Width);


		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

		int OpenGLChannels = 0;
		switch (channels)
		{
		case 1:
			OpenGLChannels = GL_R;
			break;
		case 2:
			OpenGLChannels = GL_RG;
			break;
		case 3:
			OpenGLChannels = GL_RGB;
			break;
		case 4:
			OpenGLChannels = GL_RGBA;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		default:
			break;
		}

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, OpenGLChannels, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

}