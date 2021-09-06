# Fighting Game Engine #

A fighting game engine in C++
It is currently in very early stages and it's going through many changes.
It has character framedata editor and scripting support.
Rollback netplay code is planned.

![screenshot](https://user-images.githubusercontent.com/39018575/129495632-f9d50690-9469-4033-b8fe-1e218d82892c.png)


-----------------------
## How to play ##

You can download the released binaries if you don't want to compile the game.

To configure your keys/buttons do the following while in the game:

For keyboard, press F9 to configure player 1 and F10 for player 2.
For controller, press F7 to configure player 1 and F8 for player 2.
Then input any key, axis value, button, etc. you want in the follower order: 
Up Down Left Right A B C D. This is a four button fighter.

You can swap the controllers by pressing F6.

-----------------------
## Compiling ##

Clone the repo, init the submodules, run cmake with your preferred settings and build.
Set AFGE_BUILD_TOOLS to true if you want to build the developer tools.
