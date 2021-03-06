#pragma once
#include "Engine/Renderer/Shader.h"

namespace Engine
{
	class ENGINE_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& name, Ref<ShaderSorce> src);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override;

		virtual void UploadUniformInt(const std::string& name, const int value) override;
		virtual void UploadUniformIntArray(const std::string& name, const int* values, uint32_t count) override;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& values)override;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& values)override;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& values)override;
		virtual void UploadUniformFloat(const std::string& name, const float value)override;
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix)override;
		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix)override;
		virtual void UploadUniformMat2(const std::string& name, const glm::mat2& matrix)override;

	private:
		virtual uint32_t GetUniformLocation(const std::string& name) const;

		void Compile();

	private:
		uint32_t m_RendererID;
		std::string m_Name;
		mutable std::unordered_map<std::string, uint32_t> m_UniformLocationCache;

		Ref<ShaderSorce> m_Src;
	};
}