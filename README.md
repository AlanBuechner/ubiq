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

## Aditional Notes

When you go to build the game do not press `Ctl+B` the custom build script will automaticly handle building all the dependances so this will just cause each project to be built multiple times. Insted right click on the project and click build.

## Creating a new project

If you want to create a new project just copy the `NewProject` folder and rename it than run `GenerateProjects.bat`. This will require you to redo step 4 of the setup process.

If you want the game project to exist in a different folder you will need to update the reference to the engine folder in the `local.ini` file.

## Developers

Alan Buechner

[![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/alan-buechner-223aa5206/)
[![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/AlanBuechner)
