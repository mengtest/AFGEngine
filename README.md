# Fighting Game Engine #

A fighting game engine in C++ that will be used in a game I'm planning.
It is currently in very early stages and it's going through many changes.
It's relatively small in scope and it won't be too robust, but adding new functionality shouldn't be too much of an issue.
Don't expect to be usable for anything yet.

![screenshot](https://user-images.githubusercontent.com/39018575/97825338-20c29b80-1c9d-11eb-9d25-5287eea75cba.png)

-----------------------
## How to play ##

It's not playable since the assets files aren't provided YET, but they will soon be.

To configure your keys/buttons in-game do the following:

For keyboard press F1 and input the keys you that want to be set in this order: 
UP DOWN LEFT RIGHT A B C D.

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

The character editor is being reworked at this moment. It won't be provided in this repository yet.
