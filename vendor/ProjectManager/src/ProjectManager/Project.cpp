#include "Project.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML
{

	template<>
	struct convert<ProjectManager::Project::Version>
	{
		static Node encode(const ProjectManager::Project::Version& rhs)
		{
			Node node;
			node.push_back(rhs.major);
			node.push_back(rhs.minor);
			node.push_back(rhs.patch);

			return node;
		}

		static bool decode(const Node& node, ProjectManager::Project::Version& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.major = node[0].as<uint32_t>();
			rhs.minor = node[1].as<uint32_t>();
			rhs.patch = node[2].as<uint32_t>();
			return true;
		}
	};
}

namespace ProjectManager
{


	YAML::Emitter& operator<<(YAML::Emitter& out, const Project::Version& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.major << v.minor << v.patch << YAML::EndSeq;
		return out;
	}


	Project::Project(const fs::path& projectFile)
	{
		Open(projectFile);
	}

	bool Project::Open(const fs::path& projectFile)
	{
		m_ProjectFile = projectFile;

		std::ifstream ifs(projectFile);
		std::stringstream ss;
		ss << ifs.rdbuf();

		YAML::Node data = YAML::Load(ss);
		m_Version = data["Version"].as<Version>();

		if (data["DefultMaterialID"])
			m_DefultMaterialID = data["DefultMaterialID"].as<uint64_t>();

		ifs.close();
		return true;
	}

	bool Project::Save(const fs::path& path)
	{

		// write project file
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Version" << YAML::Value << m_Version;
		out << YAML::Key << "DefultMaterialID" << YAML::Value << m_DefultMaterialID;

		out << YAML::EndMap;

		fs::create_directories(path.parent_path());
		std::ofstream fout(path);
		fout << out.c_str();
		fout.flush();
		fout.close();

		return true;
	}

}
