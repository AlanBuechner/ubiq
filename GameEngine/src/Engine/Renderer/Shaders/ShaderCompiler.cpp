#include "pch.h"
#include "ShaderCompiler.h"
#include "Engine/Core/Application.h"
#include <fstream>

namespace Engine
{
#pragma region Utilitis


	Utils::Vector<std::string> Tokenize(const std::string& line)
	{
		const char delimiters[] = { ' ', '	', '\r', '\n' };
		const char reservedTokens[] = { "={};\"(),<>" };

		Utils::Vector<std::string> tokens;

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
					tokens.Push(token);
					token.clear();
				}
				if (!delimiter)
					token.push_back(c);
				lastCharReserved = reserved;
			}
			else
				token.push_back(c);
		}

		if (!token.empty())
			tokens.Push(token); // add the last token

		return tokens;
	}


	uint32 MaterialParameter::GetTypeSize()
	{
		switch (type)
		{
		case MaterialParameterType::TextureID:	return 4;
		case MaterialParameterType::Bool:		return 4;
		case MaterialParameterType::Float:		return 4;
		case MaterialParameterType::Float4:		return 4*4;
		default:								return 4;
		}
	}

	WrapMode GetWrapMode(const std::string& str)
	{
		if		(str == "repeat")		return WrapMode::Repeat;
		else if (str == "repeatMiror")	return WrapMode::MirroredRepeat;
		else if (str == "clamp")		return WrapMode::Clamp;
		else							return WrapMode::Clamp;
	}

	MinMagFilter GetFilter(const std::string& str)
	{
		if		(str == "point")		return MinMagFilter::Point;
		else if (str == "linear")		return MinMagFilter::Linear;
		else if (str == "anisotropic")	return MinMagFilter::Anisotropic;
		else							return MinMagFilter::Linear;
	}
#pragma endregion


	GraphicsPassConfig* ShaderConfig::FindGraphicsPass(const std::string& passName)
	{
		for (GraphicsPassConfig& pass : graphicsPasses)
		{
			if (pass.passName == passName)
				return &pass;
		}
		return nullptr;
	}


	ComputePassConfig* ShaderConfig::FindComputePass(const std::string& passName)
	{
		for (ComputePassConfig& pass : computePasses)
		{
			if (pass.passName == passName)
				return &pass;
		}
		return nullptr;
	}

	WorkGraphPassConfig* ShaderConfig::FindWorkGraphPass(const std::string& passName)
	{
		for (WorkGraphPassConfig& pass : workGraphPasses)
		{
			if (pass.passName == passName)
				return &pass;
		}
		return nullptr;
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

		fs::path p = Application::Get().GetAssetManager().GetEmbededFolder() / file;
		if (fs::exists(p)) return p;

		return fs::path("");
	}

	Ref<ShaderSorce> ShaderCompiler::LoadFromSrc(std::istream& src, const fs::path& file)
	{
		Ref<ShaderSorce> source = CreateRef<ShaderSorce>();

		std::unordered_map<std::string, std::stringstream> ss;
		std::stringstream* currSS = &ss["config"];

		std::string line;
		while (getline(src, line))
		{
			Utils::Vector<std::string> tokens = Tokenize(line);

			if (!tokens.Empty() && tokens[0] == "#section")
				currSS = &ss[tokens[1]];
			else
				*currSS << line << '\n';
		}

		std::string configSection = ss["config"].str();
		source->config = CompileConfig(configSection);

		std::string materialCode = GenerateMaterialStruct(source->config.params);

		SectionInfo commonSection;
		std::string commonSrc = materialCode + ss["common"].str();
		PreProcess(commonSrc, commonSection, file);
		for (auto& sectionSource : ss)
		{
			if (sectionSource.first != "config" && sectionSource.first != "common")
			{
				SectionInfo section = commonSection;
				std::string sectionSrc = sectionSource.second.str();
				PreProcess(sectionSrc, section, file);
				source->m_Sections[sectionSource.first] = section;
			}
		}

		source->file = file;

		return source;
	}

	void ShaderCompiler::PreProcess(std::string& src, SectionInfo& section, const fs::path& fileLocation)
	{
		std::stringstream ss(src);

		std::string line;
		while (getline(ss, line))
		{
			const std::string delims(" \t");
			if (line.rfind("//", line.find_first_not_of(delims)) == 0)
				continue;

			Utils::Vector<std::string> tokens = Tokenize(line);
			if (!tokens.Empty())
			{

				if (tokens[0] == "#include")
				{
					CORE_ASSERT(tokens.Count() == 4 && tokens[1] == "\"" && tokens[3] == "\"", "failed to include header on line \"{0}\"", line);
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

					CORE_ASSERT(tokens.Count() == 14, "error on line \"{0}\"", line);

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

					section.m_SectionCode << "sampler " << name << ";\n";
				}
				else if (tokens[0] == "RootConstant")
				{
					// 0			1 2    3 4    5 6-?(op)
					// RootConstant < type > name : register ;
					
					CORE_ASSERT(tokens.Count() >= 6, "error on line \"{0}\"", line);

					CORE_ASSERT(tokens[1] == "<", "expected \"<\"");
					std::string& type = tokens[2];
					CORE_ASSERT(tokens[3] == ">", "expected \">\"");
					std::string& name = tokens[4];
					std::string registerCode = "";
					if (tokens[5] == ":")
					{
						uint32 i = 6;
						while (i < tokens.Count() && tokens[i] != ";")
							registerCode += tokens[i++];
						CORE_ASSERT(i != tokens.Count(), "expected \";\"");
					}
					else
						CORE_ASSERT(tokens[5] == ";", "expected \";\"");

					section.m_RootConstants.insert(name);

					section.m_SectionCode << "cbuffer " << name;
					if (registerCode != "")
						section.m_SectionCode << " : " << registerCode;
					section.m_SectionCode << " { " << type << " " << name << "; };\n";
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

		// get token queue
		Utils::Vector<std::string> tokens = Tokenize(code);
		std::queue<std::string> tokenQueue;
		tokenQueue.push("{");
		for (uint32 i = 0; i < tokens.Count(); i++)
			tokenQueue.push(tokens[i]);
		tokenQueue.push("}");

		// load description
		ObjectDescription description = BuildObject(tokenQueue);

		// load base topology
		Topology baseTopo = Topology::Triangle;
		if (description.HasEntery("topology"))
			baseTopo = ParseTopology(description["topology"]);

		// load passes
		if (description.HasEntery("passes"))
		{
			for (auto [passName, pass] : description["passes"].GetAsDescriptionMap())
			{
				bool hasGeo = pass.HasEntery("VS") || pass.HasEntery("MS");
				bool hasPixel = pass.HasEntery("PS");
				bool isGraphics = hasGeo && hasPixel;
				bool isCompute = pass.HasEntery("CS");
				bool isWorkGraph = pass.HasEntery("WG");

				if (isGraphics)
				{
					// graphics pass config
					GraphicsPassConfig rpass;
					rpass.passName = passName;
					rpass.topology = baseTopo;
					for (auto [param, val] : pass.GetAsDescriptionMap())
					{
						if		(param == "VS")				rpass.vs = val.GetAsString();
						else if (param == "PS")				rpass.ps = val.GetAsString();
						else if (param == "blendMode")		rpass.blendMode = ParseBlendMode(val);
						else if (param == "cullMode")		rpass.cullMode = ParseCullMode(val);
						else if (param == "depthTest")		rpass.depthTest = ParseDepthTest(val);
						else if (param == "topology")		rpass.topology = ParseTopology(val);
						else if (param == "conservativeRasterization")	rpass.enableConservativeRasterization = ParseConservativeRasterization(val);
					}
					config.graphicsPasses.Push(rpass);
				}
				if (isCompute)
				{
					// compute pass config
					ComputePassConfig rpass;
					rpass.passName = passName;
					for (auto [param, val] : pass.GetAsDescriptionMap())
						if (param == "CS")	rpass.cs = val.GetAsString();
					config.computePasses.Push(rpass);
				}
				if (isWorkGraph)
				{
					// work graph config
					WorkGraphPassConfig rpass;
					rpass.passName = passName;
					for (auto [param, val] : pass.GetAsDescriptionMap())
						if (param == "WG")	rpass.wg = val.GetAsString();
					config.workGraphPasses.Push(rpass);
				}
			}
		}

		// load material
		if (description.HasEntery("material"))
		{
			for (auto [param, val] : description["material"].GetAsDescriptionMap())
			{
				MaterialParameterType type = MaterialParameterType::TextureID;
				const std::string& typeName = val["value"].GetAsString();
				if (typeName == "textureID")	type = MaterialParameterType::TextureID;
				else if (typeName == "float")		type = MaterialParameterType::Float;
				else if (typeName == "float4")		type = MaterialParameterType::Float4;
				else if (typeName == "bool")		type = MaterialParameterType::Bool;

				Utils::Vector<std::string> params = val["params"].Get<Utils::Vector<std::string>>();
				std::string defaultValue = "";
				if (params.Count() > 0)
					defaultValue = params[0];

				config.params.Push({ param, type, defaultValue });
			}
		}

		return config;
	}

	std::string ShaderCompiler::GenerateMaterialStruct(Utils::Vector<MaterialParameter>& params)
	{
		if (params.Empty())
			return "";

		std::stringstream ss;
		ss << "struct Material {" << std::endl;

		for (auto& p : params)
		{
			// add type
			switch (p.type)
			{
			case MaterialParameterType::TextureID:	ss << "uint";	break;
			case MaterialParameterType::Float:		ss << "float";	break;
			case MaterialParameterType::Float4:		ss << "float4";	break;
			case MaterialParameterType::Bool:		ss << "bool";	break;
			}

			ss << " " << p.name << ";" << std::endl;
		}

		ss << "};" << std::endl;
		return ss.str();
	}

	ObjectDescription ShaderCompiler::BuildObject(std::queue<std::string>& tokenQueue)
	{
		if (tokenQueue.front() == "{") // build object
		{
			tokenQueue.pop(); // remove '{'

			ObjectDescription description(ObjectDescription::Type::Object);
			while (tokenQueue.front() != "}")
			{
				std::string name = tokenQueue.front(); // get name
				tokenQueue.pop(); // remove name
				CORE_ASSERT(tokenQueue.front() == "=", "Invalid token expected '='");
				tokenQueue.pop(); // remove '='

				description[name] = BuildObject(tokenQueue);

				CORE_ASSERT(tokenQueue.front() == ";", "Incalid token expected ';'");
				tokenQueue.pop();
			}
			tokenQueue.pop(); // remove '}'

			return description;
		}
		else // value
		{
			// get value
			std::string value = tokenQueue.front();
			tokenQueue.pop();

			// get params
			if (tokenQueue.front() == "(")
			{
				// load params
				Utils::Vector<std::string> params;
				tokenQueue.pop(); // remove the "("
				while (tokenQueue.front() != ")")
				{
					params.Push(tokenQueue.front());
					tokenQueue.pop(); // remove the parameter
					if (tokenQueue.front() == ",")
						tokenQueue.pop(); // remove the ","
				}
				tokenQueue.pop(); // remove the ")"

				// create object description
				ObjectDescription description(ObjectDescription::Type::Object);
				description["value"] = ObjectDescription::CreateFrom(value);
				description["params"] = ObjectDescription::CreateFrom(params);
				return description;
			}
			else
				return ObjectDescription::CreateFrom(value);
		}
	}

	Topology ShaderCompiler::ParseTopology(const ObjectDescription& var)
	{
		CORE_ASSERT(var.GetType() == ObjectDescription::Type::String, "Description is not of type string");

		const std::string& str = var.GetAsString();
		if		(str == "triangle")	return Topology::Triangle;
		else if (str == "line")		return Topology::Line;
		else if (str == "point")	return Topology::Point;
		return Topology::Triangle;
	}


	BlendMode ShaderCompiler::ParseBlendMode(const ObjectDescription& var)
	{
		CORE_ASSERT(var.GetType() == ObjectDescription::Type::String, "Description is not of type string");

		const std::string& str = var.GetAsString();
		if (str == "blend")	return BlendMode::Blend;
		else if (str == "add")		return BlendMode::Add;
		else if (str == "none")		return BlendMode::None;
		return BlendMode::None;
	}

	CullMode ShaderCompiler::ParseCullMode(const ObjectDescription& var)
	{
		CORE_ASSERT(var.GetType() == ObjectDescription::Type::String, "Description is not of type string");

		const std::string& str = var.GetAsString();
		if		(str == "back")		return CullMode::Back;
		else if (str == "front")	return CullMode::Front;
		else if (str == "none")		return CullMode::None;
		return CullMode::None;
	}

	DepthTest ShaderCompiler::ParseDepthTest(const ObjectDescription& var)
	{
		CORE_ASSERT(var.GetType() == ObjectDescription::Type::String, "Description is not of type string");

		const std::string& str = var.GetAsString();
		if		(str == "less")				return DepthTest::Less;
		else if (str == "lessOrEqual")		return DepthTest::LessOrEqual;
		else if (str == "greater")			return DepthTest::Greater;
		else if (str == "greaterOrEqual")	return DepthTest::GreaterOrEqual;
		else if (str == "none")				return DepthTest::None;
		return DepthTest::None;
	}

	bool ShaderCompiler::ParseConservativeRasterization(const ObjectDescription& var)
	{
		CORE_ASSERT(var.GetType() == ObjectDescription::Type::String, "Description is not of type string");

		const std::string& str = var.GetAsString();
		if		(str == "true")		return true;
		else if (str == "false")	return false;
		return false;
	}

}

