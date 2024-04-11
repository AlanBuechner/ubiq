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

class Class {
public:
  Class() = default;
  Class(const std::string &name, const std::string &attrib);
  std::string m_Name;
  std::string m_Group;
  std::string m_SemanticName;
  std::vector<Attribute> m_Attributes;

private:
  std::vector<Attribute> ParseAttributeString(const std::string& attribString);
  std::string GetGroup();
  std::string GetName();
};

class ReflectionData {
public:
  std::vector<std::string> m_Headers;
  std::unordered_map<std::string, Class> m_Classes;
};

ReflectionData GetReflectionDataFromFolder(const fs::path& path);

