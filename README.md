# PNM #
This is the official repo for *Plight:*. 

-----------------------
## How to playtest ##

Download every .7z file at "downloads" and decompress in the same directory.
Run PNM.exe to play the game or frametool.exe if you want to test the character editor.

To configure your keys/buttons in-game do the following:

For keyboard press F1 and input the keys you that want to be set in this order: 
UP DOWN LEFT RIGHT A B C D.

For joy input you'll have to press F2 and input only A B C D, in that order.
The movement is hardcoded for joy so there might be issues with it.

Compressed binaries won't be updated regularly so request them if you want the lastest ones.

-----------------------
## Compiling ##
The CMake file included should cover everything for now.

As of now the dependencies are:

* GLEW
* GLFW
* pnglib + zlib

Audio and networking capabilities were cut down for your convenience in the meantime.

Don't ask about the source of the editor.
