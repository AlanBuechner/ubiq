#include "Validate.h"
#include "ReflectionData.h"
#include "CodeGen.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  // validate all reflection files are present
  Validate();
  
  // get arguments
  std::string projectDir = argv[1];

  // remove trailing "\" from path
  if (projectDir[projectDir.size() - 1] == '\\')
    projectDir = projectDir.substr(0, projectDir.size() - 1);

  // get the name of the project from the folder name
  std::string projectName = std::filesystem::path(projectDir).filename().string();
  std::replace(projectName.begin(), projectName.end(), '-', '_');
  std::replace(projectName.begin(), projectName.end(), ' ', '_');

  // get reflection data from .reflected files
  std::cout << "building reflection data" << std::endl;
  ReflectionData data = GetReflectionDataFromFolder("generated");

  // write reflection code
  fs::path gen = "generated/generated.cpp";
  std::cout << "generating source" << std::endl;
  WriteCode(gen, projectName, data);

  return 0;
}
