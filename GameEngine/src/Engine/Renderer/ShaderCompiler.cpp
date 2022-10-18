#include "pch.h"
#include "ShaderCompiler.h"
#include <fstream>

namespace Engine
{

	Engine::ShaderConfig::RenderPass& ShaderConfig::FindPass(const std::string& passName)
	{
		for (RenderPass& pass : passes)
		{
			if (pass.passName == passName)
				return pass;
		}
		return passes[0];
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

		std::string commonSection = ss["common"].str();
		for (auto& section : ss)
		{
			if (section.first != "config" && section.first != "common")
			{
				sorce->m_Sections[section.first] = commonSection + section.second.str();
			}
		}

		sorce->file = file;

		return sorce;
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
		}

		return config;
	}

	std::vector<std::string> ShaderCompiler::Tokenize(const std::string& line)
	{
		const char delimiters[] = {' ', '	', '\n'};
		const char reservedTokens[] = { "={};" };

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

		if (val->isObject)
		{
			val->object = Object::Build(tokenQueue);
		}
		else
		{
			val->string = tokenQueue.front();
			tokenQueue.pop(); // remove value
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

