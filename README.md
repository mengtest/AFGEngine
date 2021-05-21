# Fighting Game Engine #

A fighting game engine in C++
It is currently in very early stages and it's going through many changes.
A character data editor, scripting support and rollback netcode are planned.
Don't expect it to be usable for anything serious yet.

![screenshot](https://user-images.githubusercontent.com/39018575/97825338-20c29b80-1c9d-11eb-9d25-5287eea75cba.png "Rather outdated screenshot")


-----------------------
## How to play ##

You can download the released binaries if you don't want to compile the game.

To configure your keys/buttons do the following while in the game:

For keyboard press F1 and input the keys you that want to be set in this order: 
UP DOWN LEFT RIGHT A B C D.
Similarly, you can press F2 to configure the other player's controls.

Joy input is not supported currently.

-----------------------
## Compiling ##
This project depends on:

* GLM
* SDL 2.1
* libpng
* imgui (with opengl3, glad and sdl2 bindings)

You'll only need imgui if you're building the editor by setting AFGE_BUILD_TOOLS to true.

These come as git submodules, and it'll work out of the box if you init them.
You also have the option to use a package manager by setting AFGE_USE_SUBMODULES to off.
If you're on windows you can use vcpkg or MSYS2 to install the packages for most or all of these.

You'll have to run cmake and build. The CMake file included should cover everything else.
