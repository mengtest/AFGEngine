# Fighting Game Engine #

A fighting game engine in C++
It is currently in very early stages and it's going through many changes.
A character data editor and scripting support are in progress.
Rollback netplay code is planned.
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

Clone the repo, init the submodules, run cmake with your preferred settings and build.
Set AFGE_BUILD_TOOLS to true if you want to build the developer tools.
There's also an option to use a package manager instead of using the submodules by
setting AFGE_USE_SUBMODULES to off, but it's not well maintained.
