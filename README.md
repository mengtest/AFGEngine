# Fighting Game Engine #

A fighting game engine in C++ that will be used as a base for my game.
It is currently in very early stages and it's going through many changes, mostly refactoring.
It's relatively small in scope and it won't be too robust. Although a character editor, scripting support and rollback netcode are planned. Adding new functionality shouldn't be too much of an issue.
Don't expect it to be usable for anything yet.

![screenshot](https://user-images.githubusercontent.com/39018575/97825338-20c29b80-1c9d-11eb-9d25-5287eea75cba.png)

-----------------------
## How to play ##

You can download the released binaries if you don't want to compile the game.

To configure your keys/buttons do the following in-game:

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

If you're on windows you can use vcpkg or MSYS2 to install the package for these. There are plans to add submodules.
The CMake file included should cover everything else for now.
Audio and networking capabilities are disabled. There are no dependencies for them.

I won't be updating this repository until I get around reworking the character editor.
The old editor is not and won't be provided in this repo.
