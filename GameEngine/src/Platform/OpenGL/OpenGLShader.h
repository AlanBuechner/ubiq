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

		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		virtual uint32_t GetUniformLocation(const std::string& name) const;

	private:
		uint32_t m_RendererID;
		mutable std::unordered_map<std::string, uint32_t> m_UniformLocationCache;
	};
}