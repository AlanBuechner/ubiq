#pragma once
#include "Engine/Renderer/Shader.h"

namespace Engine
{
	class ENGINE_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string pixleSrc);
		virtual ~OpenGLShader();

		virtual void Bind();
		virtual void Unbind();

	private:
		uint32_t m_RendererID;
	};
}