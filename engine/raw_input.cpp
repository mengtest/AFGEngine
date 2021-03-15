#include "raw_input.h"
#include <SDL.h>
#include <fstream>

#include "window.h"

//As usual, all of this should be wrapped in a class?
//And as always, there are temporary(?) globals for convenience.

unsigned int keySend[2] = {key::buf::TRUE_NEUTRAL, key::buf::TRUE_NEUTRAL};
SDL_Scancode modifiableSCKeys[buttonsN*2];
int modifiableJoyKeys[4] = {0};

void SetupKeys(int offset)
{
	int keyIterator = 0;
	keyIterator = offset;
	while(keyIterator<buttonsN+offset)
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		if(event.type != SDL_KEYDOWN)
			continue;
		
		modifiableSCKeys[keyIterator]=event.key.keysym.scancode;
		++keyIterator;
	}
	std::ofstream keyfile("keyconf.bin", std::ofstream::out | std::ofstream::binary);
	keyfile.write((const char*)modifiableSCKeys, sizeof(int)*buttonsN*2);
	keyfile.close();
}

void GameLoopKeyHandle(SDL_KeyboardEvent &e)
{
	if(e.repeat)
		return;
	
	SDL_Scancode scancode = e.keysym.scancode; 

	bool action = e.type == SDL_KEYDOWN;
	for(int i = 0; i < buttonsN; ++i)
	{
		if(scancode == modifiableSCKeys[i])
		{
			if(action)
				keySend[0] |= 1 << i;
			else
				keySend[0] &= ~(1 << i);
		}
	}
	for(int i = 0; i < buttonsN; ++i)
	{
		if(scancode == modifiableSCKeys[i+buttonsN])
		{
			if(action)
				keySend[1] |= 1 << i;
			else
				keySend[1] &= ~(1 << i);
		}
	}


	//unmodifiable keys.
	if(!action)
		return;

	switch (scancode){
	case SDL_SCANCODE_F1: //Set custom keys for player one
		SetupKeys(0);
		break;
	case SDL_SCANCODE_F2: //and player two too
		SetupKeys(buttonsN);
		break;
	case SDL_SCANCODE_F3: //Set joy buttons
		{
			/* TODO
			int buttonN = 0;
			glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonN);
			int i = 0;
			int *used = new int[buttonN];
			while(i < 4)
			{
				const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonN);
				for(int buttonItr = 0; buttonItr < buttonN; ++buttonItr)
				{
					if(buttons[buttonItr] == GLFW_PRESS)
					{
						if(used[buttonItr] == 1)
							continue;
						used[buttonItr] = 1;
						modifiableJoyKeys[i] = buttonItr;
						++i;
						break;
					}
				}
			}
			delete[] used;
			std::ofstream keyfile("joyconf.bin", std::ofstream::out | std::ofstream::binary);
			keyfile.write((const char*)modifiableJoyKeys, sizeof(int)*4);
			keyfile.close();
			*/
		}
		break;
	case SDL_SCANCODE_F5: //Switches between different framerates
			//ChangeFramerate();
		break;
	case SDL_SCANCODE_ESCAPE:
			mainWindow->wantsToClose = true;
		break;
	}
}

void GameLoopJoy()
{
	/* TODO
	int acount;
	const float *axesArray = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &acount);
	if(acount >= 2)
	{
		if(axesArray[0] == 1)
			keySend[0] |= key::buf::RIGHT;
		else if(axesArray[0] == -1)
			keySend[0] |= key::buf::LEFT;
		else
			keySend[0] &= ~(key::buf::RIGHT | key::buf::LEFT);

		if(axesArray[1] == 1)
			keySend[0] |= key::buf::DOWN;
		else if(axesArray[1] == -1)
			keySend[0] |= key::buf::UP;
		else
			keySend[0] &= ~(key::buf::DOWN | key::buf::UP);
	}

	int bcount;
	const unsigned char* buttonArray = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &bcount);
	for(int i = 0; i < bcount; ++i)
	{
		if(buttonArray[i] == GLFW_PRESS)
		{
			for(int i2 = 0; i2 < 4; ++i2)
			{
				if(i == modifiableJoyKeys[i2])
				{
					keySend[0] |= 1 << (i2+key::A);
				}
			}
		}
		else
		{
			for(int i2 = 0; i2 < 4; ++i2)
			{
				if(i == modifiableJoyKeys[i2])
					keySend[0] &= ~(1 << (i2+key::A));
			}
		}
	}
	*/
}
