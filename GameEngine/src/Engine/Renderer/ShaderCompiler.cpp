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

	Ref<ShaderSorce> ShaderCompiler::LoadFile(const fs::path& file)
	{
		std::ifstream shaderFile;
		shaderFile.open(file);
		if (shaderFile.fail())
		{
			CORE_ERROR("Cant open file {0}", file);
			return CreateRef<ShaderSorce>();
		}

		Ref<ShaderSorce> sorce = CreateRef<ShaderSorce>();

		std::unordered_map<std::string, std::stringstream> ss;
		std::stringstream* currSS = &ss["config"];

		std::string line;
		while (getline(shaderFile, line))
		{
			std::vector<std::string> tokens = Tokenize(line);

			if (!tokens.empty() && tokens[0] == "#section")
				currSS = &ss[tokens[1]];
			else
				*currSS << line << '\n';
		}

		shaderFile.close();

		std::string configSection = ss["config"].str();
		sorce->config = CompileConfig(configSection);
		
		std::string materialCode = GenerateMaterialStruct(sorce->config.params);

		std::string commonSection = materialCode + ss["common"].str();
		PreProcess(commonSection, file);
		for (auto& section : ss)
		{
			if (section.first != "config" && section.first != "common")
			{
				std::string code = section.second.str();
				PreProcess(code, file);
				sorce->m_Sections[section.first] = commonSection + "\n" + code;
			}
		}

		sorce->file = file;

		return sorce;
	}

	void ShaderCompiler::PreProcess(std::string& str, const fs::path& path)
	{
		std::stringstream out;
		std::stringstream ss(str);

		std::string line;
		while (getline(ss, line))
		{
			std::vector<std::string> tokens = Tokenize(line);
			if (!tokens.empty() && tokens[0] == "#include")
			{
				CORE_ASSERT( tokens.size() == 4 && tokens[1] == "\"" && tokens[3] == "\"", "failed to include header on Line \"{0}\"", line);
				fs::path headerPath = FindFilePath(tokens[2], path);

				// load header
				std::ifstream ifs(headerPath);
				std::string headerCode((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

				// process header
				PreProcess(headerCode, headerPath);

				out << headerCode << "\n";
			}
			else
			{
				out << line << "\n"; // append the line if there is no preprocessing needed
			}
		}

		str = out.str();
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

		for (Ref<Variable> var : vars)
		{
			if (var->name == "topology")
			{
				if (var->value->string == "triangle")
					config.topology = ShaderConfig::Triangle;
				else if (var->value->string == "line")
					config.topology = ShaderConfig::Line;
				else if (var->value->string == "point")
					config.topology = ShaderConfig::Point;
			}
			else if (var->name == "passes")
			{
				for (Ref<Variable> pass : var->value->object->values)
				{
					ShaderConfig::RenderPass rpass;
					rpass.passName = pass->name;
					for (Ref<Variable> var : pass->value->object->values)
					{
						if (var->name == "VS")
							rpass.vs = var->value->string;
						else if (var->name == "PS")
							rpass.ps = var->value->string;
					}
					config.passes.push_back(rpass);
				}
			}
			else if (var->name == "material")
			{
				for (Ref<Variable> param : var->value->object->values)
				{
					MaterialParameter::Type type = MaterialParameter::TextureID;
					if (param->value->string == "textureID")
						type = MaterialParameter::TextureID;
					else if (param->value->string == "float")
						type = MaterialParameter::Float;
					else if (param->value->string == "bool")
						type = MaterialParameter::Bool;

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
		const char delimiters[] = {' ', '	', '\n'};
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
}

