#include "raw_input.h"
#include "window.h"
#include <SDL.h>
#include <fstream>
#include <iostream>

unsigned int keySend[2] {};
SDL_Scancode modifiableSCKeys[buttonsN*2];
JoyInputInfo modifiableJoyKeys[buttonsN*2] = {0};
std::unordered_map<SDL_JoystickID, int> JoyInstanceIds;

short deadZone = 12540; //Sin of (90/4) * 2^15

void InitControllers(std::vector<SDL_GameController*> &controllers)
{
	auto nControllers = SDL_NumJoysticks();
	controllers.resize(nControllers);
	for (int i = 0; i < nControllers; ++i) {
		if (SDL_IsGameController(i)) {
			controllers[i] = SDL_GameControllerOpen(i);
			if (controllers[i]) {
				JoyInstanceIds.insert({SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[i])), i});
				std::cout<< "Game controller "<<i<<" "<<SDL_GameControllerName(controllers[i])<<" initialized\n";
			} else {
				std::cerr<< "Could not open gamecontroller "<<i<<" "<<SDL_GetError()<<"\n";
			}
		}
	}
}

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

void SetupJoy(int offset)
{
	int keyIterator = 0;
	keyIterator = offset;
	Uint8 lastAxis = -1;
	while(keyIterator<buttonsN+offset)
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		if(event.type == SDL_CONTROLLERBUTTONDOWN)
		{
			modifiableJoyKeys[keyIterator].id = JoyInstanceIds[event.cbutton.which];
			modifiableJoyKeys[keyIterator].button = event.cbutton.button;
			modifiableJoyKeys[keyIterator].type = JoyInputInfo::BUTTON;
			++keyIterator;
		}
		else if(event.type == SDL_CONTROLLERAXISMOTION)
		{
			auto caxis = event.caxis;
			auto packdAxis = ((caxis.value > 0)<<7)+caxis.axis;
			if(abs(caxis.value) < deadZone)
				continue;
			if(lastAxis != packdAxis)
			{
				lastAxis = packdAxis;
			}
			else
				continue;
			modifiableJoyKeys[keyIterator].id = JoyInstanceIds[caxis.which];
			modifiableJoyKeys[keyIterator].axis = packdAxis;
			modifiableJoyKeys[keyIterator].type = JoyInputInfo::AXIS;
			++keyIterator;
		}
		else
			continue;
	}
	std::ofstream keyfile("joyconf.bin", std::ofstream::out | std::ofstream::binary);
	keyfile.write((const char*)modifiableJoyKeys, sizeof(modifiableJoyKeys));
	keyfile.close();
}

void KeyHandle(SDL_KeyboardEvent &e)
{
	if(e.repeat)
		return;
	
	SDL_Scancode scancode = e.keysym.scancode; 

	bool action = e.type == SDL_KEYDOWN;
	for(int i = 0; i < buttonsN*2; ++i)
	{
		if(scancode == modifiableSCKeys[i])
		{
			if(action)
				keySend[i/buttonsN] |= 1 << (i%buttonsN);
			else
				keySend[i/buttonsN] &= ~(1 << (i%buttonsN));
		}
	}
	//unmodifiable keys.
	if(!action)
		return;

	switch (scancode){
	case SDL_SCANCODE_F5: //Switches between different framerates
		mainWindow->ChangeFramerate();
		break;
	case SDL_SCANCODE_F6: //Swap controller
		for(auto& id : JoyInstanceIds)
			id.second = (id.second+1)%2;
		break;
	case SDL_SCANCODE_F7: //Set joy buttons for p1
		SetupJoy(0);
		break;
	case SDL_SCANCODE_F8: //for p2
		SetupJoy(buttonsN);
		break;
	case SDL_SCANCODE_F9: //Set custom keys for player one
		SetupKeys(0);
		break;
	case SDL_SCANCODE_F10: //and player two too
		SetupKeys(buttonsN);
		break;
	case SDL_SCANCODE_ESCAPE:
			mainWindow->wantsToClose = true;
		break;
	default:
		return;
	}
}

void AxisHandle(const SDL_ControllerAxisEvent &caxis)
{
	bool isDown = abs(caxis.value) > deadZone;
	for(int i = 0; i < buttonsN*2; ++i)
	{
		int controller = i/buttonsN;
		if( modifiableJoyKeys[i].id == JoyInstanceIds[caxis.which] &&
			modifiableJoyKeys[i].type == JoyInputInfo::AXIS &&
			(modifiableJoyKeys[i].axis & 0x7F) == caxis.axis)
		{
			if(isDown && !!(modifiableJoyKeys[i].axis & 0x80) == caxis.value > 0)
				keySend[controller] |= 1 << (i%buttonsN);
			else
				keySend[controller] &= ~(1 << (i%buttonsN));
		}
	}
}

void ButtonHandle(const SDL_ControllerButtonEvent &cbutton)
{

	bool isDown = cbutton.state > 0; //Pressed down
	for(int i = 0; i < buttonsN*2; ++i)
	{
		int controller = i/buttonsN;
		if( modifiableJoyKeys[i].id == JoyInstanceIds[cbutton.which] &&
			modifiableJoyKeys[i].type == JoyInputInfo::BUTTON &&
			modifiableJoyKeys[i].button == cbutton.button)
		{
			if(isDown)
				keySend[controller] |= 1 << (i%buttonsN);
			else
				keySend[controller] &= ~(1 << (i%buttonsN));
		}
	}
}



void EventLoop(std::function<bool(const SDL_KeyboardEvent&)> keyHandler)
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
				if(event.key.repeat)
					break;
				if(!keyHandler(event.key)) //Didn't handle the key
					KeyHandle(event.key);
				break;
			case SDL_CONTROLLERAXISMOTION:
				AxisHandle(event.caxis);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				ButtonHandle(event.cbutton);
				break;
		}
	}
}
