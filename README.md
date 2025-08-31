# UBIQ [![License](https://img.shields.io/github/license/AlanBuechner/ubiq)](https://github.com/AlanBuechner/Romulus/blob/master/LICENSE)
The Ubiq Game Engine is a game egnine that was created to allow for more low level control over the games and engines systems such as input, rendering, and physics making it easyer to implement varius system to work however is best for your project with as little friction from the engine its self.

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

Go to `projects/NewProject` and run `GenerateProjects.bat` to generate visual studio project files.

<ins>**4. Debugging:**</ins>

If you want the visual studio debugger to work you will need to install the [Microsoft Child Process Debugging Power Tool 2022](https://marketplace.visualstudio.com/items?itemName=vsdbgplat.MicrosoftChildProcessDebuggingPowerTool2022) and enable it in `Debug > Other Debug Targets > Child Process Debugging Settings` and checking `Enable child process debugging` than clicking `save`.

## Creating a new project

If you want to create a new project just copy the `NewProject` folder located in the `projects` folder and rename the folder than run `GenerateProjects.bat`.

If you want the game project to exist in a different folder than the engine's `project` folder you will need to update the reference to the engine folder in the `local.ini` file located in the game's project folder.

## Aditional Notes

When you go to just build the game do not press `Ctl+B` the custom build script will automaticly handle building all the dependance tree so this will just cause each project to be built multiple times. Insted right click on the game project and click build.

## Developers

Alan Buechner

[![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/alan-buechner-223aa5206/)
[![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/AlanBuechner)
