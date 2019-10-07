#pragma once
#include "Engine/Core/core.h"
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

namespace Engine
{
	class Shader
	{
	public:
		struct ShaderSorce
		{
		public:
			std::string vertexPath;
			std::string vertexShader;
			std::string pixlePath;
			std::string pixleShader;

			void operator<<(const ShaderSorce& other)
			{
				if (this->vertexShader.empty())
				{
					this->vertexShader = other.vertexShader;
					this->vertexPath = other.vertexPath;
				};
				if (this->pixleShader.empty())
				{
					this->pixleShader = other.pixleShader; 
					this->pixlePath = other.pixlePath;
				};
			}
		};

		enum ShaderType
		{
			None = -1, Vertex = BIT(0), Pixle = BIT(1)
		};

	public:
		virtual ~Shader() {};

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;

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

		static Ref<Shader> Create(const std::string& name, Ref<Shader::ShaderSorce> src);

	private:
		virtual uint32_t GetUniformLocation(const std::string& name) const = 0;
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& name, const std::string& path);
		Ref<Shader> Load(const std::string& name, Ref<Shader::ShaderSorce> src);
		Ref<Shader> Get(const std::string& name);

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}