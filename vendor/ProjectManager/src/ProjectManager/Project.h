#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace ProjectManager
{
	class Project
	{
	public:
		struct Version
		{
			uint32_t major = 0;
			uint32_t minor = 0;
			uint32_t patch = 0;

			bool operator==(const Version& other) const
			{
				return major == other.major && minor == other.minor && patch == other.patch;
			}
		};

	public:
		Project() = default;
		Project(const fs::path& projectFile);

		bool Open(const fs::path& projectFile);
		inline bool Save() { return Save(m_ProjectFile); }
		bool Save(const fs::path& path);

		inline fs::path GetProjectFile() { return m_ProjectFile; }
		inline void SetProjectFile(fs::path path) { m_ProjectFile = path; }
		inline std::string GetProjectName() { return m_ProjectFile.filename().stem().string(); }

		inline fs::path GetRootDirectory() { return m_ProjectFile.parent_path(); }
		inline fs::path GetAssetsDirectory() { return GetRootDirectory() / "Assets"; }
		inline fs::path GetScriptsDirectory() { return GetRootDirectory() / "Scripts"; }
		inline fs::path GetModulesDirectory() { return GetRootDirectory() / "Modules"; }

		inline Version GetVersion() { return m_Version; }
		inline void SetVersion(Version version) { m_Version = version; }

	private:
		fs::path m_ProjectFile;

		Version m_Version;
	};
}


namespace std
{
	template<>
	struct hash <ProjectManager::Project::Version>
	{
		std::size_t operator()(const ProjectManager::Project::Version& v) const
		{
			std::size_t maj = hash<std::size_t>()(v.major);
			std::size_t min = hash<std::size_t>()(v.minor);
			std::size_t pat = hash<std::size_t>()(v.patch);

			return maj ^ min ^ pat;
		}
	};
}
