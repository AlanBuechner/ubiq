#pragma once
#include "Engine/Core/Core.h"
#include "Shader.h"

#include <queue>


namespace Engine
{
	struct MaterialParameter;


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
			std::string passName;
			std::string vs;
			std::string ps;
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

		std::unordered_map<std::string, std::string> m_Sections;

		ShaderConfig config;
	};

	class ShaderCompiler
	{
	public:
		static fs::path FindFilePath(const fs::path& file, const fs::path& parent);
		static Ref<ShaderSorce> LoadFile(const fs::path& file);
		static void PreProcess(std::string& str, const fs::path& path);
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
