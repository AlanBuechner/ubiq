#include "pch.h"
#include "OpenGLFrameBuffer.h"
#include <glad/glad.h>

namespace Engine
{
	static const uint32_t MaxFramBufferSize = 8192;

	namespace Utils
	{

		static bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:	return true;
			}
			return false;
		}

		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisample, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisample), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static GLenum UbiqFBTextureFormatToGL(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case Engine::FrameBufferTextureFormat::RGBA8:			return GL_RGBA8;
			case Engine::FrameBufferTextureFormat::RED_INTEGER:		return GL_RED_INTEGER;
			case Engine::FrameBufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
			default:
				break;
			}
			
			CORE_ASSERT(false, "unknown format")
			return 0;
		}
	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec) :
		m_Spec(spec)
	{
		for (auto format : m_Spec.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentSpecs.emplace_back(format);
			else
				m_DepthAttachmentSpec = format;
		}

		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
		glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Spec.Samples > 1;

		// Attachments
		if (m_ColorAttachmentSpecs.size()) 
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), (GLsizei)m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); i++)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecs[i].TextureFormat)
				{
				case FrameBufferTextureFormat::RGBA8:
					Utils::AttachColorTexture(m_ColorAttachments[(int)i], m_Spec.Samples, GL_RGBA8, GL_RGBA, m_Spec.Width, m_Spec.Height, (uint32_t)i);
					break;
				case FrameBufferTextureFormat::RED_INTEGER:
					Utils::AttachColorTexture(m_ColorAttachments[(int)i], m_Spec.Samples, GL_R32I, GL_RED_INTEGER, m_Spec.Width, m_Spec.Height, (uint32_t)i);
					break;
				default:
					break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);

			switch (m_DepthAttachmentSpec.TextureFormat)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Spec.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Spec.Width, m_Spec.Height);
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			CORE_ASSERT(m_ColorAttachments.size() <= 4, "");

			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void OpenGLFrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Spec.Width, m_Spec.Height);

		
	}

	void OpenGLFrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > MaxFramBufferSize || height > MaxFramBufferSize)
		{
			CORE_WARN("Attempted to resize frameBuffer to {0}, {1}", width, height);
			return;
		}

		m_Spec.Width = width;
		m_Spec.Height = height;

		Invalidate();
	}

	int OpenGLFrameBuffer::ReadPixle(uint32_t index, int x, int y)
	{
		CORE_ASSERT(index < m_ColorAttachments.size(), "");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		int pixleData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixleData);
		return pixleData;
	}

	void OpenGLFrameBuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "");

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];

		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::UbiqFBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);

	}

}