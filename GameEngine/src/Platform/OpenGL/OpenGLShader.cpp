#include "pch.h"
#include "OpenGLShader.h"
#include <glad/glad.h>

namespace Engine
{
	OpenGLShader::OpenGLShader(const std::string& name, Ref<Shader::ShaderSorce> src)
		: m_Src(src), m_Name(name)
	{
		Compile();
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const 
	{
		glUseProgram(0);
	}

	const std::string& OpenGLShader::GetName() const
	{
		return m_Name;
	}

	std::vector<Shader::Uniform> OpenGLShader::GetUniforms()
	{
		const GLsizei bufSize = 16; // maximum name length
		GLchar name[bufSize]; // variable name in GLSL
		GLsizei length; // name length

		GLint size; // size of the variable
		GLenum type; // type of the variable (float, vec3 or mat4, etc)

		int count;
		glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &count);
		std::vector< Shader::Uniform> uniforms(count);

		for (uint32 i = 0; i < (uint32)count; i++)
		{
			glGetActiveUniform(m_RendererID, (GLuint)i, bufSize, &length, &size, &type, name);

			Shader::Uniform& uniform = uniforms[i];
			uniform.name = name;
			
			switch (type)
			{
			case GL_FLOAT: uniform.type = Shader::Uniform::Float; break;
			case GL_FLOAT_VEC2: uniform.type = Shader::Uniform::Float2; break;
			case GL_FLOAT_VEC3: uniform.type = Shader::Uniform::Float3; break;
			case GL_FLOAT_VEC4: uniform.type = Shader::Uniform::Float4; break;

			case GL_INT: uniform.type = Shader::Uniform::Int; break;
			case GL_INT_VEC2: uniform.type = Shader::Uniform::Int2; break;
			case GL_INT_VEC3: uniform.type = Shader::Uniform::Int3; break;
			case GL_INT_VEC4: uniform.type = Shader::Uniform::Int4; break;

			case GL_FLOAT_MAT2: uniform.type = Shader::Uniform::Mat2; break;
			case GL_FLOAT_MAT3: uniform.type = Shader::Uniform::Mat3; break;
			case GL_FLOAT_MAT4: uniform.type = Shader::Uniform::Mat4; break;
			default:
				break;
			}
		}

		return uniforms;
	}

	uint32 OpenGLShader::GetUniformLocation(const std::string & name) const
	{
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name];
		uint32 location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			CORE_ERROR("Uniform {0} was not found!", name);
			return location;
		}
		m_UniformLocationCache[name] = location;
		return location;
	}

	void OpenGLShader::Compile()
	{
		// Create an empty vertex shader handle
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const GLchar* source = (const GLchar*)m_Src->vertexShader.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		// Compile the vertex shader
		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			CORE_ERROR("{0}", infoLog.data());
			CORE_ASSERT(false, "Vertex shader comilation failure!");

			// In this simple program, we'll just leave
			return;
		}

		// Create an empty fragment shader handle
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		source = m_Src->pixleShader.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		// Compile the fragment shader
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragmentShader);
			// Either of them. Don't leak shaders.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			CORE_ERROR("{0}", infoLog.data());
			CORE_ASSERT(false, "Fragment shader comilation failure!");
			// In this simple program, we'll just leave
			return;
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
		m_RendererID = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);

		// Link our program
		glLinkProgram(m_RendererID);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(m_RendererID);
			// Don't leak shaders either.
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Use the infoLog as you see fit.
			CORE_ERROR("{0}", infoLog.data());
			CORE_ASSERT(false, "Shader link failure!");

			// In this simple program, we'll just leave
			return;
		}

		// Always detach shaders after a successful link.
		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int value)
	{
		uint32 location = GetUniformLocation(name);
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, uint32 count)
	{
		uint32 location = GetUniformLocation(name);
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string & name, const Math::Vector4 & values)
	{
		uint32 location = GetUniformLocation(name);
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const Math::Vector3& values)
	{
		uint32 location = GetUniformLocation(name);
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const Math::Vector2& values)
	{
		uint32 location = GetUniformLocation(name);
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float value)
	{
		uint32 location = GetUniformLocation(name);
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformMat4(const std::string & name, const Math::Mat4 & matrix)
	{
		uint32 location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const Math::Mat3& matrix)
	{
		uint32 location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat2(const std::string& name, const Math::Mat2& matrix)
	{
		uint32 location = GetUniformLocation(name);
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadPointLight(const std::string& name, uint32 i, const PointLight& light)
	{
		std::string l = name + "[" + std::to_string(i) + "].";
		UploadUniformFloat3(l + "position", light.position);
		UploadUniformFloat3(l + "color", light.color);
		UploadUniformFloat(l + "diffuseIntensity", light.diffuseIntensity);
		UploadUniformFloat(l + "attConst", light.attConst);
		UploadUniformFloat(l + "attLin", light.attLin);
		UploadUniformFloat(l + "attQuad", light.attQuad);
	}

}
