#include "pch.h"
#include "ShaderCompiler.h"
#include <fstream>

namespace Engine
{

	uint32 MaterialParameter::GetTypeSize()
	{
		switch (type)
		{
		case MaterialParameter::TextureID:
		case MaterialParameter::Bool:
		case MaterialParameter::Float:
			return 4;
		default:
			return 4;
		}
	}

	SamplerInfo::WrapMode GetWrapMode(const std::string& str)
	{
		if (str == "repeat")
			return SamplerInfo::WrapMode::Repeat;
		else if (str == "repeatMiror")
			return SamplerInfo::WrapMode::MirroredRepeat;
		else if (str == "clamp")
			return SamplerInfo::WrapMode::Clamp;
		return SamplerInfo::WrapMode::Clamp;
	}

	SamplerInfo::MinMagFilter GetFilter(const std::string& str)
	{
		if (str == "point") 
			return SamplerInfo::MinMagFilter::Point;
		else if (str == "linear") 
			return SamplerInfo::MinMagFilter::Linear;
		else if (str == "anisotropic") 
			return SamplerInfo::MinMagFilter::Anisotropic;
		return SamplerInfo::MinMagFilter::Linear;
	}


	ShaderConfig::RenderPass& ShaderConfig::FindPass(const std::string& passName)
	{
		for (RenderPass& pass : passes)
		{
			if (pass.passName == passName)
				return pass;
		}
		return passes[0];
	}


	fs::path ShaderCompiler::FindFilePath(const fs::path& file, const fs::path& parent)
	{
		if (fs::exists(file))
			return file;

		fs::path parentfolder = parent.parent_path();
		if (fs::exists(parentfolder / file))
			return parentfolder / file;

		if(fs::exists(fs::path("Assets") / file))
			return fs::path("Assets") / file;

		return fs::path("");
	}

	Ref<ShaderSorce> ShaderCompiler::LoadFromFile(const fs::path& file)
	{
		std::ifstream shaderFile;
		shaderFile.open(file);
		if (shaderFile.fail())
		{
			CORE_ERROR("Cant open file {0}", file.string());
			return CreateRef<ShaderSorce>();
		}

		return LoadFromSrc(shaderFile, file);
	}

	Ref<ShaderSorce> ShaderCompiler::LoadFromSrc(std::istream& src, const fs::path& file)
	{
		Ref<ShaderSorce> sorce = CreateRef<ShaderSorce>();

		std::unordered_map<std::string, std::stringstream> ss;
		std::stringstream* currSS = &ss["config"];

		std::string line;
		while (getline(src, line))
		{
			std::vector<std::string> tokens = Tokenize(line);

			if (!tokens.empty() && tokens[0] == "#section")
				currSS = &ss[tokens[1]];
			else
				*currSS << line << '\n';
		}

		std::string configSection = ss["config"].str();
		sorce->config = CompileConfig(configSection);

		std::string materialCode = GenerateMaterialStruct(sorce->config.params);

		ShaderSorce::SectionInfo commonSection;
		std::string commonSrc = materialCode + ss["common"].str();
		PreProcess(commonSrc, commonSection, file);
		for (auto& sectionSource : ss)
		{
			if (sectionSource.first != "config" && sectionSource.first != "common")
			{
				ShaderSorce::SectionInfo section = commonSection;
				std::string sectionSrc = sectionSource.second.str();
				PreProcess(sectionSrc, section, file);
				sorce->m_Sections[sectionSource.first] = section;
			}
		}

		sorce->file = file;

		return sorce;
	}

	void ShaderCompiler::PreProcess(std::string& src, ShaderSorce::SectionInfo& section, const fs::path& fileLocation)
	{
		std::stringstream ss(src);

		std::string line;
		while (getline(ss, line))
		{
			const std::string delims(" \t");
			if (line.rfind("//", line.find_first_not_of(delims)) == 0)
				continue;

			std::vector<std::string> tokens = Tokenize(line);
			if (!tokens.empty())
			{

				if (tokens[0] == "#include")
				{
					CORE_ASSERT(tokens.size() == 4 && tokens[1] == "\"" && tokens[3] == "\"", "failed to include header on line \"{0}\"", line);
					fs::path headerPath = FindFilePath(tokens[2], fileLocation);

					// load header
					std::ifstream ifs(headerPath);
					std::string headerCode((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

					// process header
					PreProcess(headerCode, section, headerPath);
				}
				else if (tokens[0] == "StaticSampler")
				{
					// 0			 1	  2 3			 4 5 6 7 8 9   10 11  12 13
					// StaticSampler name = StaticSampler( U , V , Min ,  Mag )  ;

					CORE_ASSERT(tokens.size() == 14, "error on line \"{0}\"", line);

					std::string& name = tokens[1];
					CORE_ASSERT(tokens[2] == "=", "expected \"=\"");
					CORE_ASSERT(tokens[3] == "StaticSampler", "expected \"StaticSampler\"");
					CORE_ASSERT(tokens[4] == "(", "expected \"(\"");
					std::string& u = tokens[5];
					CORE_ASSERT(tokens[6] == ",", "expected \",\"");
					std::string& v = tokens[7];
					CORE_ASSERT(tokens[8] == ",", "expected \",\"");
					std::string& min = tokens[9];
					CORE_ASSERT(tokens[10] == ",", "expected \",\"");
					std::string& mag = tokens[11];
					CORE_ASSERT(tokens[12] == ")", "expected \")\"");
					CORE_ASSERT(tokens[13] == ";", "expected \";\"");

					SamplerInfo info;
					info.U = GetWrapMode(u);
					info.V = GetWrapMode(v);

					info.Min = GetFilter(min);
					info.Mag = GetFilter(mag);

					section.m_Samplers[name] = info;

					section.m_SectionCode << "sampler " << name << ";";
				}
				else
				{
					section.m_SectionCode << line << "\n"; // append the line if there is no preprocessing needed
				}
			}
		}
	}

	ShaderConfig ShaderCompiler::CompileConfig(const std::string& code)
	{
		ShaderConfig config{};
		std::istringstream iss(code);

		std::vector<std::string> tokens = Tokenize(code);
		std::queue<std::string> tokenQueue;
		for (uint32 i = 0; i < tokens.size(); i++)
			tokenQueue.push(tokens[i]);
		
		std::vector<Ref<Variable>> vars;
		while (!tokenQueue.empty())
			vars.push_back(Variable::Build(tokenQueue));
		ShaderConfig::RenderPass::Topology baseTopo = ShaderConfig::RenderPass::Topology::Triangle;

		for (Ref<Variable> var : vars)
		{
			if (var->name == "topology")
				baseTopo = ParseTopology(var);
			else if (var->name == "passes")
			{
				for (Ref<Variable> pass : var->value->object->values)
				{
					ShaderConfig::RenderPass rpass;
					rpass.passName = pass->name;
					rpass.topology = baseTopo;
					for (Ref<Variable> var : pass->value->object->values)
					{
						if (var->name == "VS")				rpass.vs = var->value->string;
						else if (var->name == "PS")			rpass.ps = var->value->string;
						else if (var->name == "CS")			rpass.cs = var->value->string;
						else if (var->name == "blendMode")	rpass.blendMode = ParseBlendMode(var);
						else if (var->name == "cullMode")	rpass.cullMode = ParseCullMode(var);
						else if (var->name == "depthTest")	rpass.depthTest = ParseDepthTest(var);
						else if (var->name == "topology")	rpass.topology = ParseTopology(var);
					}
					config.passes.push_back(rpass);
				}
			}
			else if (var->name == "material")
			{
				for (Ref<Variable> param : var->value->object->values)
				{
					MaterialParameter::Type type = MaterialParameter::TextureID;
					if (param->value->string == "textureID")	type = MaterialParameter::TextureID;
					else if (param->value->string == "float")	type = MaterialParameter::Float;
					else if (param->value->string == "bool")	type = MaterialParameter::Bool;

					std::string defaultValue = "";
					if (param->value->paramters.size() > 0)
						defaultValue = param->value->paramters[0];

					config.params.push_back({ param->name, type, defaultValue }); 
				}
			}
		}

		return config;
	}

	std::string ShaderCompiler::GenerateMaterialStruct(std::vector<MaterialParameter>& params)
	{
		if (params.empty())
			return "";

		std::stringstream ss;
		ss << "struct Material {" << std::endl;

		for (auto& p : params)
		{
			// add type
			switch (p.type)
			{
			case MaterialParameter::TextureID:
				ss << "uint ";
				break;
			case MaterialParameter::Float:
				ss << "float ";
				break;
			case MaterialParameter::Bool:
				ss << "bool ";
				break;
			}

			ss << p.name << ";" << std::endl;
		}

		ss << "};" << std::endl;
		return ss.str();
	}

	std::vector<std::string> ShaderCompiler::Tokenize(const std::string& line)
	{
		const char delimiters[] = {' ', '	', '\r', '\n'};
		const char reservedTokens[] = { "={};\"()," };

		std::vector<std::string> tokens;

		bool lastCharReserved = false;
		std::string token;
		for (const char& c : line)
		{
			bool endToken = false;
			// check if it is a delimiter
			bool delimiter = false;
			for (uint32 i = 0; i < _countof(delimiters); i++)
			{
				if (c == delimiters[i])
				{
					endToken = true;
					delimiter = true;
					break;
				}
			}

			// check if reserved token
			bool reserved = false;
			for (uint32 i = 0; i < _countof(reservedTokens); i++)
			{
				if (c == reservedTokens[i])
				{
					reserved = true;
					break;
				}
			}

			// combine char with last
			if (reserved && lastCharReserved && !endToken)
			{
				endToken = true;
				//if (token.back() == '=')
			}

			if (lastCharReserved != reserved)
				endToken = true;

			if (endToken)
			{
				if (!token.empty())
				{
					tokens.push_back(token);
					token.clear();
				}
				if (!delimiter)
					token.push_back(c);
				lastCharReserved = reserved;
			}
			else
				token.push_back(c);
		}

		if(!token.empty())
			tokens.push_back(token); // add the last token

		return tokens;
	}


	Ref<ShaderCompiler::Variable> ShaderCompiler::Variable::Build(std::queue<std::string>& tokenQueue)
	{
		Ref<Variable> var = CreateRef<Variable>();
		var->name = tokenQueue.front();
		tokenQueue.pop();

		CORE_ASSERT(tokenQueue.front() == "=", "Invalid token expected '='");
		tokenQueue.pop(); // remove '='

		var->value = Value::Build(tokenQueue);
		CORE_ASSERT(tokenQueue.front() == ";", "Incalid token expected ';'");
		tokenQueue.pop();

		return var;
	}

	Ref<ShaderCompiler::Value> ShaderCompiler::Value::Build(std::queue<std::string>& tokenQueue)
	{
		Ref<Value> val = CreateRef<Value>();
		val->isObject = tokenQueue.front() == "{";
		val->isString = tokenQueue.front() == "\"";

		if (val->isObject)
			val->object = Object::Build(tokenQueue);
		else if (val->isString)
		{
			tokenQueue.pop(); // remove first "
			val->string = tokenQueue.front();
			tokenQueue.pop(); // remove value
			tokenQueue.pop(); // remove last "
		}
		else
		{
			val->string = tokenQueue.front();
			tokenQueue.pop(); // remove value
		}

		if (tokenQueue.front() == "(")
		{
			tokenQueue.pop(); // remove the "("
			while (tokenQueue.front() != ")")
			{
				val->paramters.push_back(tokenQueue.front());
				tokenQueue.pop(); // remove the parameter
				if (tokenQueue.front() == ",")
					tokenQueue.pop(); // remove the ","
			}
			tokenQueue.pop(); // remove the ")"
		}

		return val;
	}

	Ref<ShaderCompiler::Object> ShaderCompiler::Object::Build(std::queue<std::string>& tokenQueue)
	{
		Ref<Object> object = CreateRef<Object>();

		CORE_ASSERT(tokenQueue.front() == "{", "Invalid token expected '{'");
		tokenQueue.pop(); // remove '{'

		while (tokenQueue.front() != "}")
		{
			object->values.push_back(Variable::Build(tokenQueue));
		}

		tokenQueue.pop(); // remove '}'

		return object;
	}

	ShaderConfig::RenderPass::Topology ShaderCompiler::ParseTopology(Ref<Variable> var)
	{
		if (var->value->string == "triangle")	return ShaderConfig::RenderPass::Topology::Triangle;
		else if (var->value->string == "line")	return ShaderConfig::RenderPass::Topology::Line;
		else if (var->value->string == "point")	return ShaderConfig::RenderPass::Topology::Point;
		return ShaderConfig::RenderPass::Topology::Triangle;
	}


	ShaderConfig::RenderPass::BlendMode ShaderCompiler::ParseBlendMode(Ref<Variable> var)
	{
		if (var->value->string == "blend")		return ShaderConfig::RenderPass::BlendMode::Blend;
		else if (var->value->string == "add")	return ShaderConfig::RenderPass::BlendMode::Add;
		else if (var->value->string == "none")	return ShaderConfig::RenderPass::BlendMode::None;
		return ShaderConfig::RenderPass::BlendMode::None;
	}

	ShaderConfig::RenderPass::CullMode ShaderCompiler::ParseCullMode(Ref<Variable> var)
	{
		if (var->value->string == "back")		return ShaderConfig::RenderPass::CullMode::Back;
		else if (var->value->string == "front")	return ShaderConfig::RenderPass::CullMode::Front;
		else if (var->value->string == "none")	return ShaderConfig::RenderPass::CullMode::None;
		return ShaderConfig::RenderPass::CullMode::None;
	}

	ShaderConfig::RenderPass::DepthTest ShaderCompiler::ParseDepthTest(Ref<Variable> var)
	{
		if (var->value->string == "less")					return ShaderConfig::RenderPass::DepthTest::Less;
		else if (var->value->string == "lessOrEqual")		return ShaderConfig::RenderPass::DepthTest::LessOrEqual;
		else if (var->value->string == "greater")			return ShaderConfig::RenderPass::DepthTest::Greater;
		else if (var->value->string == "greaterOrEqual")	return ShaderConfig::RenderPass::DepthTest::GreaterOrEqual;
		else if (var->value->string == "none")				return ShaderConfig::RenderPass::DepthTest::None;
		return ShaderConfig::RenderPass::DepthTest::None;
	}

}

