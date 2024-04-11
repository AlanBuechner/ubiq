#include "CodeGen.h"
#include <fstream>


void WriteCode(const fs::path &path, const std::string &projectName, const ReflectionData& data) {

  fs::remove(path);
  std::ofstream ofs(path);

  // generate reflection code
  ofs << "#include <pch.h>" << std::endl;

  ofs << "#include \"Reflection.h\"" << std::endl;
  for (auto &h : data.m_Headers)
    ofs << "#include \"" << h << "\"" << std::endl;

  for (auto &c : data.m_Classes) {

    const std::string &sname = c.first;
    const std::string &name = c.second.m_Name;
    const std::string &group = c.second.m_Group;
    const std::vector<Attribute> attrib = c.second.m_Attributes;
    ofs << "static Reflect::Registry::Add<" << sname << "> Class" << name
        << "(\"" << name << "\", \"" << sname << "\", \"" << group
        << "\", Reflect::CreateInstance<" << sname << ">);" << std::endl;
  }

  ofs << "void DeadLink" << projectName << "() {}" << std::endl;
  ofs.close();
}
