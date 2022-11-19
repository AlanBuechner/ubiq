#pragma once
#include "Engine/Core/Core.h"
#include "Engine/AssetManager/AssetManager.h"
#include "Light.h"
#include "FrameBuffer.h"
#include "ShaderCompiler.h"
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

		enum ShaderType
		{
			None = -1, Vertex = BIT(0), Pixel = BIT(1)
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
		Shader(const fs::path& file);

		Ref<ShaderPass> GetPass(const std::string& passName);
		std::vector<MaterialParameter>& GetParams();

		static Ref<Shader> Create(const fs::path& file);

	private:
		std::unordered_map<std::string, Ref<ShaderPass>> m_Passes;
		std::vector<MaterialParameter> m_Params;
	};



	struct ShaderInputElement
	{
		std::string semanticName;
		uint32 semanticIndex;
		ShaderPass::Uniform::Type format;
	};

	struct ShaderParameter
	{
		enum class PerameterType
		{
			Constants = 0,
			Descriptor = 1,
			DescriptorTable = 2,
			StaticSampler = 4
		};

		enum class DescriptorType
		{
			CBV,
			SRV,
			UAV,
			Sampler
		};

		ShaderPass::ShaderType shader; // witch shader is requesting the information
		PerameterType type;
		DescriptorType descType; // the type of descriptor
		std::string name; // used for reflection queries
		/*
		* shader : Constants
		*	- the number of constant values
		* 
		* shader : DescDescriptorTable
		*	- the number of Descriptors in the table
		*/
		uint32 count;
		uint32 reg;
		uint32 space;

		uint32 rootIndex = 0;
	};

	struct MaterialParameter
	{
		enum Type
		{
			TextureID,
			Bool
		};

		std::string name;
		Type type;
		std::string defaultValue;

		uint32 GetTypeSize();
	};
}
