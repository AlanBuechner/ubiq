# UBIQ [![License](https://img.shields.io/github/license/AlanBuechner/ubiq)](https://github.com/AlanBuechner/Romulus/blob/master/LICENSE)
The Ubiq Game Engine

## Platforms
![Platform](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

![Architecture](https://img.shields.io/badge/Architecture-x64-blue.svg)


## Getting started
Visual Studio 2022 is recomended. Ubiq is untested in other environments .

<ins>**1. Downloading the repository:**</ins>

clone the repository with:

```
git clone --recursive https://github.com/AlanBuechner/ubiq
```

If the repository was cloned non-recursively previously you will need to clone the submoduals using:

```
git submodule update --init
```
<ins>**2. Setup:**</ins>

Run `Setup.bat` to download all dependances.

<ins>**3. Generate Project:**</ins>

Goto `projects/NewProject` and run `GenerateProjects.bat` to generate visual studio project files.


<ins>**4. Doing what premake can not:**</ins>

Unfortunently premake does not generate NMake project files for visual studio properly. As a result you will need to manualy finish what premake started.

1. For each project copy the contants of the `includedirs` array in the projects `premake5.lua` into the `Include Search Path` field inside the projects properties under the `NMake` tab. Each entry should be seperated by a `;`

2. For each project copy the defines from the projects `Build.py` file and add the `STRIP_REFLECTION`define to stop visual studio form complaning about the reflection system.

3. Add `/std:c++17` into the `Aditional Options` field under the `NMake` tab otherwise visual studio will think you are using c++ 14 and will give lots of errors.

4. In the properties for `NewProject` in the `Debugging` tab paset `{your project location}\bin\Release-windows-x86_64\USG-Editor\USG-Editor.exe` for the release configuration and do the same for debug and dist but replace Release with Debug and Dist in the file path

Any time you run `GenerateProjects.bat` for you will need to redo this step. 

Engine project files are shared between game projects so running `GenerateProjects.bat` or making any manual edits the the vistual studio project files will be reflected across all game projects

## Aditional Notes

When you go to build the game do not press `Ctl+B` the custom build script will automaticly handle building all the dependances so this will just cause each project to be built multiple times. Insted right click on the project and click build.

## Creating a new project

If you want to create a new project just copy the `NewProject` folder and rename it than run `GenerateProjects.bat`. This will require you to redo step 4 of the setup process.

If you want the game project to exist in a different folder you will need to update the reference to the engine folder in the `GenerateProjects.bat`.

## Developers

Alan Buechner

[![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/alan-buechner-223aa5206/)
[![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/AlanBuechner)