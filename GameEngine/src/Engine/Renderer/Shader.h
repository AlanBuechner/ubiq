#pragma once
#include <string>
#include "Engine/core.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
	class Shader
	{
	public:
		struct ShaderSorce
		{
		public:
			std::string vertexShader;
			std::string pixleShader;

			std::vector<std::pair<std::string, int>> path;

			void operator<<(const ShaderSorce& other)
			{
				if (this->vertexShader.empty()) this->vertexShader = other.vertexShader;
				if (this->pixleShader.empty()) this->pixleShader = other.pixleShader;

				path.push_back(other.path[0]);
			}
		};

		enum ShaderType
		{
			None = -1, Vertex = BIT(0), Pixle = BIT(1)
		};

	public:
		virtual ~Shader() {};

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void UploadUniformInt(const std::string& name, const int value) = 0;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& values) = 0;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& values) = 0;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& values) = 0;
		virtual void UploadUniformFloat(const std::string& name, const float value) = 0;
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;
		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void UploadUniformMat2(const std::string& name, const glm::mat2& matrix) = 0;

		static ShaderSorce LoadShader(std::string file);
		static ShaderSorce LoadShader(std::string file, int type);
		static void ReloadShader(ShaderSorce& shaders);

		static Ref<Shader> Create(Ref<Shader::ShaderSorce> src);

	private:
		virtual uint32_t GetUniformLocation(const std::string& name) const = 0;
	};
}