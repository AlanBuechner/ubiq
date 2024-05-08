#include "CodeGen.h"
#include <fstream>


void WriteFlags(std::ofstream& ofs, const std::vector<Attribute>& attribs)
{
	std::vector<std::string> flags;
	for (const Attribute& attrib : attribs)
	{
		if (attrib.type == Attribute::Type::Flag)
			flags.push_back(attrib.name);
	}
	ofs << "{";
	for (uint32_t i = 0; i < flags.size(); i++)
	{
		const std::string& flag = flags[i];
		ofs << "\"" << flag << "\"";
		if (i != flags.size() - 1)
			ofs << ",";
	}
	ofs << "}";
}

void WriteProps(std::ofstream& ofs, const std::vector<Property> props, const std::string& parentTypeName)
{
	ofs << "{";
	for (uint32_t i = 0; i < props.size(); i++)
	{
		const Property& prop = props[i];
		ofs << "Reflect::Property(\"" << prop.m_Name << "\"," <<
			"typeid(" << prop.m_Type << ").hash_code()," <<
			"sizeof(" << prop.m_Type << ")," <<
			"offsetof(" << parentTypeName << "," << prop.m_Name << "),";
		WriteFlags(ofs, prop.m_Attributes);
		ofs << ")";
		if (i != props.size() - 1)
			ofs << ",";
	}
	ofs << "}";
}

void WriteCode(const fs::path& path, const std::string& projectName, const ReflectionData& data) {

	fs::remove(path);
	std::ofstream ofs(path);

	// generate reflection code
	ofs << "#define _XKEYCHECK_H" << std::endl;
	ofs << "#include <pch.h>" << std::endl;

	ofs << "#include \"Reflection.h\"" << std::endl;
	ofs << "#define private public" << std::endl;
	for (auto& h : data.m_Headers)
		ofs << "#include \"" << h << "\"" << std::endl;

	for (auto& c : data.m_Classes) {

		const std::string& sname = c.first;
		const std::string& name = c.second.m_Name;
		const std::string& group = c.second.m_Group;
		const std::vector<Attribute> attribs = c.second.m_Attributes;
		const std::vector<Property>& props = c.second.m_Props;

		ofs << "static Reflect::Registry::Add<" << sname << "> Class" << name
			<< "(\"" << name << "\", \"" << sname << "\", \"" << group << "\",";
		
		WriteFlags(ofs, attribs);
		ofs << ", ";
		WriteProps(ofs, props, sname);
		ofs <<");" << std::endl;
	}

	ofs << "void DeadLink" << projectName << "() {}" << std::endl;
	ofs.close();
}
