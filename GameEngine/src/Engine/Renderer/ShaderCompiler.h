#pragma once
#include "Engine/Core/Core.h"
#include "Shader.h"
#include "TextureAttribute.h"

#include <queue>
#include <sstream>

namespace Engine
{

	struct ShaderInputElement
	{
		std::string semanticName;
		uint32 semanticIndex;
		ShaderPass::Uniform::Type format;
	};

	enum class ShaderType
	{
		None = -1, Vertex = BIT(0), Pixel = BIT(1), Compute = BIT(2)
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

		ShaderType shader; // witch shader is requesting the information
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

		TextureAttribute samplerAttribs; // used for static sampler

		uint32 rootIndex = 0;
	};

	struct MaterialParameter
	{
		enum Type
		{
			TextureID,
			Float,
			Bool
		};

		std::string name;
		Type type;
		std::string defaultValue;

		uint32 GetTypeSize();
	};


	struct ShaderConfig
	{
		enum Topology
		{
			Triangle,
			Line,
			Point,
		};

		struct RenderPass
		{
			enum BlendMode {
				Blend,
				Add
			};

			std::string passName;
			std::string vs;
			std::string ps;
			BlendMode blendMode = Blend;

		};

		Topology topology;

		std::vector<RenderPass> passes;

		std::vector<MaterialParameter> params;

		std::unordered_map<std::string, void*> m_DataLocations;

		RenderPass& FindPass(const std::string& passName);
	};

	struct ShaderSorce
	{
	public:
		fs::path file;

		struct SectionInfo
		{
			struct SamplerInfo
			{
				TextureAttribute m_SamplerConfig;
			};

			std::stringstream m_SectionCode;
			std::unordered_map<std::string, SamplerInfo> m_Samplers;

			SectionInfo() = default;
			SectionInfo(const SectionInfo& other)
			{
				*this = other;
			}

			SectionInfo& operator=(const SectionInfo& other)
			{
				if (this != &other)
				{
					m_SectionCode << other.m_SectionCode.str();
					m_Samplers = other.m_Samplers;
				}
				return *this;
			}
		};

		std::unordered_map<std::string, SectionInfo> m_Sections;

		ShaderConfig config;
	};

	class ShaderCompiler
	{
	public:
		static fs::path FindFilePath(const fs::path& file, const fs::path& parent);
		static Ref<ShaderSorce> LoadFromFile(const fs::path& file);
		static Ref<ShaderSorce> LoadFromSrc(std::istream& src, const fs::path& file = "");
		static void PreProcess(std::string& src, ShaderSorce::SectionInfo& section, const fs::path& fileLocation);
		static ShaderConfig CompileConfig(const std::string& code);
		static std::string GenerateMaterialStruct(std::vector<MaterialParameter>& params);

	private:
		static std::vector<std::string> Tokenize(const std::string& line);
		
		struct Variable;
		struct Value;
		struct Object;

		struct Variable 
		{
			std::string name;
			Ref<Value> value;

			static Ref<Variable> Build(std::queue<std::string>& tokenQueue);
		};

		struct Value
		{
			bool isObject;
			bool isString;

			std::string string;
			std::vector<std::string> paramters;
			Ref<Object> object;

			static Ref<Value> Build(std::queue<std::string>& tokenQueue);
		};

		struct Object 
		{
			std::vector<Ref<Variable>> values;

			static Ref<Object> Build(std::queue<std::string>& tokenQueue);
		};
		

	};
}
