#include "CodeGen.h"
#include <fstream>


void WriteCode(const fs::path& path, const std::string& projectName, const ReflectionData& data) {

	fs::remove(path);
	std::ofstream ofs(path);

	// generate reflection code
	ofs << "#include <pch.h>" << std::endl;

	ofs << "#include \"Reflection.h\"" << std::endl;
	for (auto& h : data.m_Headers)
		ofs << "#include \"" << h << "\"" << std::endl;

	for (auto& c : data.m_Classes) {

		const std::string& sname = c.first;
		const std::string& name = c.second.m_Name;
		const std::string& group = c.second.m_Group;
		const std::vector<Attribute> attribs = c.second.m_Attributes;
		std::vector<std::string> flags;
		for (const Attribute& attrib : attribs)
		{
			if (attrib.type == Attribute::Type::Flag)
				flags.push_back(attrib.name);
		}

		ofs << "static Reflect::Registry::Add<" << sname << "> Class" << name
			<< "(\"" << name << "\", \"" << sname << "\", \"" << group << "\", {";
		
		for (uint32_t i = 0; i < flags.size(); i++)
		{
			const std::string& flag = flags[i];
			ofs << "\"" << flag << "\"";
			if (i != flags.size() - 1)
				ofs << ",";
		}

		ofs <<"});" << std::endl;
	}

	ofs << "void DeadLink" << projectName << "() {}" << std::endl;
	ofs.close();
}
