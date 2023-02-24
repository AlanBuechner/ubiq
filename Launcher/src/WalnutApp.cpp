#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "Walnut/Image.h"
#include "Walnut/ImGui/Roboto-Regular.embed"

#include "ProjectManager/Project.h"

#include <windows.h>
#include <iostream>

#include <vector>
#include <fstream>

#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <filesystem>
namespace fs = std::filesystem;

std::string OpenFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)glfwGetWin32Window(Walnut::Application::Get().GetWindowHandle());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return std::string();

}

std::vector<std::string> tokenize(std::string s, std::string del = " ")
{
	std::vector<std::string> tokens;
	int start = 0;
	int end = s.find(del);
	while (end != -1) {
		tokens.push_back(s.substr(start, end - start));
		start = end + del.size();
		end = s.find(del, start);
	}
	tokens.push_back(s.substr(start, end - start));
	return tokens;
}

inline std::string toString(const fs::file_time_type& timePoint)
{
	const size_t Precision = std::numeric_limits<double>::digits10;
	auto seconds = (double)(timePoint.time_since_epoch().count()) * fs::file_time_type::period::num / fs::file_time_type::period::den;
	auto const zeconds = std::modf(seconds, &seconds);
	std::time_t tt(seconds);
	std::ostringstream oss;
	auto const tm = std::localtime(&tt);
	if (!tm) throw std::runtime_error(std::strerror(errno));
	oss << std::put_time(tm, "%Y-%b-%d %H:%M:")
		<< std::setw(Precision + 3) << std::setfill('0')
		<< std::fixed << std::setprecision(Precision)
		<< tm->tm_sec + zeconds;
	if (!oss) throw std::runtime_error("timepoint-to-string");
	return oss.str();
}

class ExampleLayer : public Walnut::Layer
{
	enum Tab
	{
		Projects,
		NewProject,
		Installs,
	};
	
	Tab m_Tab = Projects;

	fs::path m_ProjectListFile = "ProjectList.txt";
	fs::path m_InstallsListFile = "InstallsList.txt";

	std::vector<fs::path> m_ProjectList;
	std::unordered_map<ProjectManager::Project::Version, fs::path> m_Installs;

public:
	virtual void OnAttach() override
	{
		if (fs::exists(m_ProjectListFile))
		{
			std::ifstream ifs(m_ProjectListFile);

			std::string line;
			while (getline(ifs, line))
			{
				fs::path projectPath = line;
				if (fs::exists(projectPath))
					m_ProjectList.push_back(projectPath);
			}

			ifs.close();
		}

		if (fs::exists(m_InstallsListFile))
		{
			std::ifstream ifs(m_InstallsListFile);

			std::string line;
			while (getline(ifs, line))
			{
				std::vector<std::string> tokens = tokenize(line, ",");
				std::vector<std::string> versionTokens = tokenize(tokens[0], ".");

				ProjectManager::Project::Version version;
				version.major = std::stoi(versionTokens[0]);
				version.minor = std::stoi(versionTokens[1]);
				version.patch = std::stoi(versionTokens[2]);

				fs::path installPath = tokens[1];
				if (fs::exists(installPath))
					m_Installs[version] = installPath;
			}
		}
	}

	virtual void OnDetach() override
	{
		{ // projects
			std::ofstream ofs(m_ProjectListFile);

			for (uint32_t i = 0; i < m_ProjectList.size(); i++)
				ofs << m_ProjectList[i].string() << std::endl;

			ofs.flush();
			ofs.close();
		}

		{ // installs
			std::ofstream ofs(m_InstallsListFile);

			for (auto install : m_Installs)
				ofs << install.first.major << "." << install.first.minor << "." << install.first.patch << "," << install.second.string() << std::endl;

			ofs.flush();
			ofs.close();
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 100,20 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0,2 });

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

		ImGui::Begin("##SidePanel", nullptr, flags);
		ImGui::PushFont(Walnut::Application::Get().m_TabButtonFont);
		if (ImGui::Button("Projects"))
			m_Tab = Projects;
		if (ImGui::Button("Installs"))
			m_Tab = Installs;
		ImGui::PopFont();
		ImGui::End();

		ImGui::Begin("##ContentPanel", nullptr, flags);
		switch (m_Tab)
		{
		case ExampleLayer::Projects:
			DrawProjectsTab();
			break;
		case ExampleLayer::NewProject:
			DrawNewProjectTab();
			break;
		case ExampleLayer::Installs:
			DrawInstallsTab();
			break;
		default:
			break;
		}
		ImGui::End();

		ImGui::PopStyleVar(2);
	}

	void DrawProjectsTab()
	{
		ImGui::PushFont(Walnut::Application::Get().m_HeaderFont);
		ImGui::Text("Projects");

		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			fs::path projectPath = OpenFile("Ubiq Project (*.ubiqproj)\0*.ubiqproj\0");
			if (fs::exists(projectPath))
			{
				if(std::find(m_ProjectList.begin(), m_ProjectList.end(), projectPath) == m_ProjectList.end())
					m_ProjectList.push_back(projectPath);
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("New"))
			m_Tab = NewProject;

		ImGui::PopFont();

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::BeginTable("##projectsTable", 2);

		ImGui::PushFont(Walnut::Application::Get().m_ProjectTitleFont);

		ImGui::TableSetupColumn("NAME");
		ImGui::TableSetupColumn("LAST DATE MODIFIED");
		ImGui::TableHeadersRow();

		for (uint32_t i = 0; i < m_ProjectList.size(); i++)
		{
			fs::path projectPath = m_ProjectList[i];

			if (fs::exists(projectPath))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (ImGui::Button(projectPath.string().c_str()))
				{
					ProjectManager::Project project(projectPath);
					fs::path editorLocation = m_Installs[project.GetVersion()];

					STARTUPINFO si;
					PROCESS_INFORMATION pi;

					// set the size of the structures
					ZeroMemory(&si, sizeof(si));
					si.cb = sizeof(si);
					ZeroMemory(&pi, sizeof(pi));

					std::wstring args = editorLocation.wstring() + L" " + project.GetProjectFile().wstring();

					// start the program up
					CreateProcess(editorLocation.wstring().c_str(),   // the path
						(LPWSTR)args.c_str(),        // Command line
						NULL,           // Process handle not inheritable
						NULL,           // Thread handle not inheritable
						FALSE,          // Set handle inheritance to FALSE
						0,              // No creation flags
						NULL,           // Use parent's environment block
						editorLocation.parent_path().wstring().c_str(),           // Use parent's starting directory 
						&si,            // Pointer to STARTUPINFO structure
						&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
					);
					// Close process and thread handles. 
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
				ImGui::TableNextColumn();

				fs::file_time_type lastModifide = fs::last_write_time(projectPath);
				ImGui::Text(toString(lastModifide).c_str());
			}
			else
			{
				m_ProjectList.erase(m_ProjectList.begin() + i);
				i--;
			}
		}
		ImGui::PopFont();

		ImGui::EndTable();
	}

	void DrawNewProjectTab()
	{
		ImGui::PushFont(Walnut::Application::Get().m_HeaderFont);
		ImGui::Text("New Project");
		ImGui::PopFont();

		static std::string projectName = "New Project";
		static std::string projectFolder = (fs::current_path() / "projects").string();

		ImGui::InputText("##ChangeFileName", &projectName);
		ImGui::InputText("project folder", &projectFolder);

		if (ImGui::Button("Create Project"))
		{
			projectName.erase(remove(projectName.begin(), projectName.end(), ' '), projectName.end());
			fs::path projectFile = fs::path(projectFolder) / (projectName + ".ubiqproj");

			if (fs::exists(projectFile)) // TODO : give error
				return;

			ProjectManager::Project project;
			project.SetProjectFile(projectFile);
			project.Save();

			m_ProjectList.push_back(project.GetProjectFile());

			m_Tab = Projects;
		}
	}

	void DrawInstallsTab()
	{
		ImGui::PushFont(Walnut::Application::Get().m_HeaderFont);
		ImGui::Text("Installs");
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			fs::path installPath = OpenFile("Editor (*.exe)\0*.exe\0Editor (*.bat)\0*.bat\0");
			if (fs::exists(installPath))
			{
				ProjectManager::Project::Version version;
				version.major = 0;
				version.minor = 0;
				version.patch = 0;
				m_Installs[version] = installPath;
			}
		}
		ImGui::PopFont();

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::BeginTable("##installsTable", 2);

		ImGui::PushFont(Walnut::Application::Get().m_ProjectTitleFont);

		ImGui::TableSetupColumn("INSTALL LOCATION");
		ImGui::TableSetupColumn("VERSION");
		ImGui::TableHeadersRow();

		for (auto install : m_Installs)
		{
			ProjectManager::Project::Version version = install.first;
			fs::path installPath = install.second;

			if (fs::exists(installPath))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text(installPath.string().c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%i.%i.%i", version.major, version.minor, version.patch);
			}
		}

		ImGui::PopFont();

		ImGui::EndTable();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	int horizontal = desktop.right * 2;
	int vertical = desktop.bottom * 2;
	
	const float padding = 0.15f;
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";
	spec.Width = horizontal - (2 * padding * horizontal);
	spec.Height = vertical - (2 * padding * vertical);

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}
