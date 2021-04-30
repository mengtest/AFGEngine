# Fighting Game Engine #

A fighting game engine in C++ that will be used as a base for my game.
It is currently in very early stages and it's going through many changes, mostly refactoring.
It's relatively small in scope and it won't be too robust. Although a character editor, scripting support and rollback netcode are planned. Adding new functionality shouldn't be too much of an issue.
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

* glad
* GLM
* SDL 2.1
* libpng

Additionally, if you're building the editor by setting AFGE_BUILD_TOOLS to true, you'll also need 
dear ImGui (with opengl3, glad and sdl2 bindings)

If you're on windows you can use vcpkg or MSYS2 to install the packages for most or all of these.
There are plans to add git submodules so you don't have to get the dependencies yourself.
When that's done you'll have to run cmake. The CMake file included should cover everything else.
Audio and networking capabilities are currently on hold. There are no dependencies for them.
