#include "main.h"
#include "battle_scene.h"
#include "raw_input.h"
#include "window.h"
#include "game_state.h"
#include <fstream>

int gameState = GS_MENU;

int main(int argc, char** argv)
{
	mainWindow = new Window();

	//This should be in raw input
	std::ifstream keyfile("keyconf.bin", std::ifstream::in | std::ifstream::binary);
	if(keyfile.is_open())
	{
		keyfile.read((char*)modifiableSCKeys, sizeof(int)*buttonsN*2);
		keyfile.close();
	}
	keyfile.open("joyconf.bin", std::ifstream::in | std::ifstream::binary);
	if(keyfile.is_open())
	{
		keyfile.read((char*)modifiableJoyKeys, sizeof(int)*4);
		keyfile.close();
	}

	while(!mainWindow->wantsToClose)
	{
		try{
			switch (gameState)
			{
				case GS_MENU:
				//break; Not implemented so have a fallthru.
				case GS_CHARSELECT:
				//break;
				case GS_WIN:
				//break;
				case GS_PLAY: {
					BattleScene bs;
					gameState = bs.PlayLoop();
					break;
				}
			}
		}
		catch(std::exception e)
		{
			std::cerr << e.what() << " The program can't continue. Press enter to quit.";
			std::cin.get();
			return 0;
		}
	}

	return 0;
}

