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

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		uint32_t m_RendererID;
	};
}