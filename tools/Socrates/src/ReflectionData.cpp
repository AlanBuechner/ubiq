#include "ReflectionData.h"
#include "Utils.h"
#include <fstream>
#include <iostream>

// class
Class::Class(const std::string& name, const std::string& attrib)
	: m_SemanticName(name) {
	m_Attributes = ParseAttributeString(attrib);
	m_Name = GetName();
	m_Group = GetGroup();
}

std::vector<Attribute> Class::ParseAttributeString(const std::string& attribString) {
	std::vector<Attribute> attrs;

	std::vector<char> reservedTokens = { ',', '=' };
	std::queue<std::string> tokens = Tokenize(attribString, reservedTokens);

	while (!tokens.empty()) {
		std::string name = tokens.front();
		tokens.pop(); // remove name

		if (tokens.empty() || tokens.front() == ",") { // flag
			if(!tokens.empty())
				tokens.pop();
			attrs.push_back({ Attribute::Flag, name, "" });
		}
		else if (tokens.front() == "=") {
			tokens.pop(); // remove "="
			attrs.push_back({ Attribute::Prop, name, tokens.front() });
			tokens.pop(); // remove value
			// remove ","
			if (tokens.front() == ",")
				tokens.pop();
		}
	}


	return attrs;
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

		std::ifstream ifs(path);

		std::string line = "";
		while (getline(ifs, line)) {
			line.erase(remove(line.begin(), line.end(), ' '), line.end());
			std::vector<std::string> tokens = QueueToVector(Tokenize(line, reservedTokens));
			if (tokens[0] == "include") {
				if (!hasEntry(data.m_Headers, tokens[2]))
					data.m_Headers.push_back(tokens[2]);
			}
			else if (tokens[0] == "class") {
				std::string name = tokens[2];
				std::string attrib = tokens[4];
				data.m_Classes[name] = Class(name, attrib);
			}
			else
			{
				std::cout << "error unknown token " << tokens[0] << std::endl;
			}
		}

		ifs.close();
	}

	return data;
}
