#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

struct Attribute {
	enum Type {
		Flag,
		Prop,
	} type;
	std::string name;
	std::string value;
};

std::vector<Attribute> ParseAttributeString(const std::string& attribString);

class Property
{
public:
	Property(const std::string& name, const std::string& type, const std::string& attrib);
	std::string m_Name;
	std::string m_Type;
	std::vector<Attribute> m_Attributes;
};

class Function
{
public:
	Function(const std::string& name, const std::vector<std::string>& types, const std::string& attrib);
	std::string m_Name;
	std::vector<std::string> m_Types;
	std::vector<Attribute> m_Attributes;
};

class Class {
public:
	Class() = default;
	Class(const std::string& name, const std::string& attrib);
	std::string m_Name;
	std::string m_Group;
	std::string m_SemanticName;
	std::vector<Attribute> m_Attributes;
	std::vector<Property> m_Props;
	std::vector<Function> m_Functions;

private:
	std::string GetGroup();
	std::string GetName();
};

class ReflectionData {
public:
	std::vector<std::string> m_Headers;
	std::unordered_map<std::string, Class> m_Classes;
};

ReflectionData GetReflectionDataFromFolder(const fs::path& path);

