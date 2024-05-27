#include "ReflectionData.h"
#include "Utils.h"
#include <fstream>
#include <iostream>

std::vector<Attribute> ParseAttributeString(const std::string& attribString) {
	std::vector<Attribute> attrs;

	std::vector<char> reservedTokens = { ',', '=' };
	std::queue<std::string> tokens = Tokenize(attribString, reservedTokens);

	while (!tokens.empty()) {
		std::string name = tokens.front();
		tokens.pop(); // remove name

		if (tokens.empty() || tokens.front() == ",") { // flag
			if (!tokens.empty())
				tokens.pop();
			if(name != "")
				attrs.push_back({ Attribute::Flag, name, "" });
		}
		else if (tokens.front() == "=") {
			tokens.pop(); // remove "="
			if(name != "")
				attrs.push_back({ Attribute::Prop, name, tokens.front() });
			tokens.pop(); // remove value
			// remove ","
			if (tokens.front() == ",")
				tokens.pop();
		}
	}

	return attrs;
}

Property::Property(const std::string& name, const std::string& type, const std::string& attrib) :
	m_Name(name), m_Type(type)
{
	m_Attributes = ParseAttributeString(attrib);
}

Function::Function(const std::string& name, const std::vector<std::string>& types, const std::string& attrib) :
	m_Name(name), m_Types(types)
{
	m_Attributes = ParseAttributeString(attrib);
}

// class
Class::Class(const std::string& name, const std::string& attrib)
	: m_SemanticName(name) {
	m_Attributes = ParseAttributeString(attrib);
	m_Name = GetName();
	m_Group = GetGroup();
}

std::string Class::GetGroup() {
	for (auto& a : m_Attributes) {
		if (a.type == Attribute::Prop && a.name == "GROUP") {
			return a.value;
		}
	}
	return "";
}

std::string Class::GetName() {
	uint32_t index = 0;
	for (uint32_t i = m_SemanticName.size() - 1; i >= 0; i--) {
		if (m_SemanticName[i] == ':') {
			index = i + 1;
			break;
		}
	}

	return m_SemanticName.substr(index);
}

// gen reflection data
ReflectionData GetReflectionDataFromFolder(const fs::path& path) {

	std::vector<char> reservedTokens = { '<', '>', ';' };
	ReflectionData data;

	for (auto& p : fs::directory_iterator("generated")) {
		if (p.is_directory())
			continue;

		fs::path path = p.path();
		if (path.extension() != ".reflected")
			continue;

		std::string currentClassName = "";
		std::ifstream ifs(path);

		std::string line = "";
		while (getline(ifs, line)) {
			std::queue<std::string> tokens = Tokenize(line, reservedTokens);
			std::string action = tokens.front();
			std::cout << line << std::endl;
			tokens.pop();
			if (action == "include") {
				tokens.pop(); // remove the "
				std::string header = tokens.front();
				tokens.pop(); // remove the token
				if (!hasEntry(data.m_Headers, header))
					data.m_Headers.push_back(header);
				tokens.pop(); // remove the "
			}
			else if (action == "class") {
				tokens.pop(); // remove the <
				std::string name = tokens.front();
				tokens.pop(); // remove the token
				tokens.pop(); // remove the >
				std::string attrib = tokens.front();
				attrib.erase(remove(attrib.begin(), attrib.end(), ' '), attrib.end());
				tokens.pop(); // remove the token
				data.m_Classes[name] = Class(name, attrib);
				currentClassName = name;
			}
			else if (action == "	prop")
			{
				tokens.pop(); // remove the <
				std::string type = "";
				while (tokens.front() != ";")
				{
					type += tokens.front();
					tokens.pop(); // remove the token
				}
				tokens.pop(); // remove the ;
				std::string name = tokens.front();
				tokens.pop(); // remove the token
				tokens.pop(); // remove the >
				std::string attrib = tokens.front();
				attrib.erase(remove(attrib.begin(), attrib.end(), ' '), attrib.end());
				tokens.pop(); // remove the token
				data.m_Classes[currentClassName].m_Props.push_back(Property(name, type, attrib));
			}
			else if (action == "	func")
			{
				tokens.pop(); // remove the <
				std::string name = tokens.front();
				tokens.pop(); // remove the name
				if(tokens.front() == ";")
					tokens.pop(); // remove the ;
				std::vector<std::string> types;
				int indent = 1;
				while (indent >= 1)
				{
					std::string type = "";
					while (tokens.front() != ";" && indent >= 1)
					{
						if (tokens.front() == "<") indent++;
						if (tokens.front() == ">") indent--;
						if (indent >= 1)
							type += tokens.front();
						tokens.pop(); // remove the token
					}
					if (type != "")
						types.push_back(type);
				}
				std::string attrib = tokens.front();
				attrib.erase(remove(attrib.begin(), attrib.end(), ' '), attrib.end());
				tokens.pop();
				data.m_Classes[currentClassName].m_Functions.push_back(Function(name, types, attrib));
			}
			else
			{
				std::cout << "error unknown token " << action << std::endl;
			}
		}

		ifs.close();
	}

	return data;
}

