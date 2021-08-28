#include "raw_input.h"
#include "window.h"
#include <SDL.h>
#include <fstream>

unsigned int keySend[2] {};
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

void EventLoop(std::function<void(SDL_KeyboardEvent &e)> f)
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				mainWindow->wantsToClose = true;
				return;
			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						mainWindow->UpdateViewport(event.window.data1, event.window.data2);
						break;
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				f(event.key);
				break;
		}
	}
}
