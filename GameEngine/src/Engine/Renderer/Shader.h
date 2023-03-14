#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Light.h"
#include "FrameBuffer.h"
#include <string>
#include <Engine/Math/Math.h>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <vector>

namespace Engine
{
	struct ShaderParameter;
	struct ShaderSorce;
	struct MaterialParameter;

	class ShaderPass
	{
	public:
		struct Uniform
		{
			std::string name;
			enum Type {
				Float, Int, Uint,
				Float2, Int2, Uint2,
				Float3, Int3, Uint3,
				Float4, Int4, Uint4,
				Mat2, Mat3, Mat4
			} type;
		};

	public:
		virtual ~ShaderPass() {};

		virtual std::vector<ShaderParameter> GetReflectionData() const = 0;

		static Ref<ShaderPass> Create(Ref<ShaderSorce> src, const std::string& passName);

		virtual uint32 GetUniformLocation(const std::string& name) const = 0;
	};

	class Shader : public Asset
	{
	public:
		Shader(const std::string& src, const fs::path& file);

		Ref<ShaderPass> GetPass(const std::string& passName);
		std::vector<MaterialParameter>& GetParams();

		static Ref<Shader> Create(const fs::path& file);
		static Ref<Shader> CreateFromEmbeded(uint32 id, const fs::path& file);
		static Ref<Shader> CreateFromSrc(const std::string& src, const fs::path& file = "");

	private:
		std::unordered_map<std::string, Ref<ShaderPass>> m_Passes;
		std::vector<MaterialParameter> m_Params;
	};


	class ComputeShader : public Asset
	{
	public:

		virtual std::vector<ShaderParameter> GetReflectionData() const = 0;

		virtual uint32 GetUniformLocation(const std::string& name) const = 0;

		static Ref<ComputeShader> Create(const fs::path& file);
		static Ref<ComputeShader> CreateFromEmbeded(uint32 id, const fs::path& file);
		static Ref<ComputeShader> CreateFromSrc(const std::string& src, const fs::path& file = "");
	};
}
