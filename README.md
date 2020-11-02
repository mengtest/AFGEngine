# PNM #

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
Audio and networking capabilities are disabled, so there no dependencies for them.

The character editor is not provided in this repository.
