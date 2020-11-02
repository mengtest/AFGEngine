# Fighting Game Engine #

A fighting game engine that aims to reduce hardcoding as much as possible. It is currently in very early stages and it'll go through many changes. Don't expect to be usable for anything yet.

![screenshot](https://user-images.githubusercontent.com/39018575/97825338-20c29b80-1c9d-11eb-9d25-5287eea75cba.png)

-----------------------
## How to play ##

Assets for the .char files aren't provided, so you'll have to get your own.

To configure your keys/buttons in-game do the following:

For keyboard press F1 and input the keys you that want to be set in this order: 
UP DOWN LEFT RIGHT A B C D.

For joy input you'll have to press F2 and input A B C D in order.
The movement is hardcoded for joy and it hasn't been tested well so there might be issues with it.

-----------------------
## Compiling ##
Whether you're on windows (MSYS2) or linux, install the packages for:

* GLEW
* GLFW3
* pnglib + zlib

The CMake file included should cover everything else for now.
Audio and networking capabilities are disabled, so there are no dependencies for them.

The character editor is not provided in this repository.
