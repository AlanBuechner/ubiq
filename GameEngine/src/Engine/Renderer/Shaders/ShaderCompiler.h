#pragma once
#include "Engine/Core/Core.h"
#include <queue>
#include <sstream>

namespace Engine
{
	// shader type
	enum class ShaderType
	{
		None = -1, Vertex = BIT(0), Pixel = BIT(1), Compute = BIT(2), WorkGraph = BIT(3)
	};

	/* ---------------------------- sampler info ---------------------------- */

	// texture wrapping mode
	enum class WrapMode
	{
		Repeat,
		MirroredRepeat,
		Clamp
	};

	// texture filtering mode
	enum class MinMagFilter
	{
		Point,
		Linear,
		Anisotropic
	};

	struct SamplerInfo
	{
		WrapMode U = WrapMode::Repeat;
		WrapMode V = WrapMode::Repeat;

		MinMagFilter Min = MinMagFilter::Linear;
		MinMagFilter Mag = MinMagFilter::Linear;

		bool operator==(const SamplerInfo& other)
		{
#define CHECK_PARAM(p) if(p != other.p) return false;
			CHECK_PARAM(U);
			CHECK_PARAM(V);
			CHECK_PARAM(Min);
			CHECK_PARAM(Mag);
			return true;
#undef CHECK_PARAM
		}

		bool operator!=(const SamplerInfo& other) { return !(*this == other); }
	};
	/* ---------------------------- end sampler info ---------------------------- */


	enum class UniformType
	{
		Float, Int, Uint,
		Float2, Int2, Uint2,
		Float3, Int3, Uint3,
		Float4, Int4, Uint4,
		Mat2, Mat3, Mat4
	};

	struct ShaderInputElement
	{
		std::string semanticName;
		uint32 semanticIndex;
		UniformType format;
	};

	/* ---------------------------- binding info ---------------------------- */
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

	struct ShaderParameter
	{
		ShaderType shader; // witch shader is requesting the information (vertex, pixel, etc)
		PerameterType type;
		DescriptorType descType; // the type of descriptor
		std::string name; // used for reflection queries
		uint32 reg;
		uint32 space;
		/*
		* type : Constants
		*	- the number of constant values
		*
		* type : DescriptorTable
		*	- the number of Descriptors in the table
		*/
		uint32 count;

		SamplerInfo samplerAttribs;

		uint32 rootIndex = 0; // index used when binding

		bool operator==(const ShaderParameter& other)
		{
#define CHECK_PARAM(p) if(p != other.p) return false;
			CHECK_PARAM(shader);
			CHECK_PARAM(type);
			CHECK_PARAM(descType);
			CHECK_PARAM(name);
			CHECK_PARAM(reg);
			CHECK_PARAM(space);
			CHECK_PARAM(count);
			CHECK_PARAM(samplerAttribs);
			CHECK_PARAM(rootIndex);
			return true;
#undef CHECK_PARAM
		}

		bool operator!=(const ShaderParameter& other) { return !(*this == other); }
	};
	/* ---------------------------- end binding info ---------------------------- */


	/* ---------------------------- material info ---------------------------- */
	enum class MaterialParameterType
	{
		TextureID,
		Float,
		Bool
	};

	struct MaterialParameter
	{
		std::string name;
		MaterialParameterType type;
		std::string defaultValue;

		uint32 GetTypeSize();
	};
	/* ---------------------------- end material info ---------------------------- */


	/* ---------------------------- render pass info ---------------------------- */
	enum class Topology
	{
		Triangle,
		Line,
		Point,
	};

	enum class BlendMode {
		Blend,
		Add,
		None
	};

	enum class CullMode {
		Back,
		Front,
		None
	};

	enum class DepthTest {
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
		None
	};

	struct GraphicsPassConfig
	{
		std::string passName;
		std::string vs; // vertex shader section name
		std::string ps; // pixel shader section name
		Topology topology;
		BlendMode blendMode = BlendMode::Blend;
		CullMode cullMode = CullMode::Back;
		DepthTest depthTest = DepthTest::LessOrEqual;
	};

	struct ComputePassConfig
	{
		std::string passName;
		std::string cs; // compute shader section name
	};

	struct WorkGraphPassConfig
	{
		std::string passName;
		std::string wg; // work graph section name
	};

	/* ---------------------------- end render pass info ---------------------------- */

	struct ShaderConfig
	{
		std::vector<GraphicsPassConfig> graphicsPasses;
		std::vector<ComputePassConfig> computePasses;
		std::vector<WorkGraphPassConfig> workGraphPasses;

		std::vector<MaterialParameter> params;

		std::unordered_map<std::string, void*> m_DataLocations;

		GraphicsPassConfig* FindGraphicsPass(const std::string& passName);
		ComputePassConfig* FindComputePass(const std::string& passName);
		WorkGraphPassConfig* FindWorkGraphPass(const std::string& passName);
	};

	struct SectionInfo
	{
		std::stringstream m_SectionCode;
		std::unordered_map<std::string, SamplerInfo> m_Samplers;

		SectionInfo() = default;
		SectionInfo(const SectionInfo& other) { *this = other; }

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

	struct ShaderSorce
	{
	public:
		fs::path file;
		std::unordered_map<std::string, SectionInfo> m_Sections;
		ShaderConfig config;
	};

	class ShaderCompiler
	{
	public:
		static fs::path FindFilePath(const fs::path& file, const fs::path& parent);
		static Ref<ShaderSorce> LoadFromSrc(std::istream& src, const fs::path& file = "");
		static void PreProcess(std::string& src, SectionInfo& section, const fs::path& fileLocation);
		static ShaderConfig CompileConfig(const std::string& code);
		static std::string GenerateMaterialStruct(std::vector<MaterialParameter>& params);

	private:
		
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
			bool HasVariable(const std::string& name);

			static Ref<Object> Build(std::queue<std::string>& tokenQueue);
		};
		

		static Topology ParseTopology(Ref<Variable> var);
		static BlendMode ParseBlendMode(Ref<Variable> var);
		static CullMode ParseCullMode(Ref<Variable> var);
		static DepthTest ParseDepthTest(Ref<Variable> var);
	};
}
