#include <fstream>

#include "raw_input.h"
#include "window.h"

//As usual, all of this should be wrapped in a class?
//And as always, there are temporary(?) globals for convenience.

unsigned int keySend[2] = {key::buf::TRUE_NEUTRAL, key::buf::TRUE_NEUTRAL};
int modifiableSCKeys[buttonsN*2] = {0};
int modifiableJoyKeys[4] = {0};
int keyIterator = 0;

void CbSetupKeys(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action != GLFW_PRESS)
        return;
    modifiableSCKeys[keyIterator]=scancode;
    ++keyIterator;
}

void SetupKeys(GLFWwindow* window, int offset)
{
	keyIterator = offset;
	glfwSetKeyCallback(window, CbSetupKeys);
	while(keyIterator<buttonsN+offset)
	{
		glfwWaitEvents();
	}
	std::ofstream keyfile("keyconf.bin", std::ofstream::out | std::ofstream::binary);
	keyfile.write((const char*)modifiableSCKeys, sizeof(int)*buttonsN*2);
	keyfile.close();
	glfwSetKeyCallback(window, CbGameLoopKeys);
}

void CbGameLoopKeys(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_REPEAT)
		return;

	for(int i = 0; i < buttonsN; ++i)
	{
		if(scancode == modifiableSCKeys[i])
		{
			if(action == GLFW_PRESS)
				keySend[0] |= 1 << i;
			else if(action == GLFW_RELEASE)
				keySend[0] &= ~(1 << i);
		}
	}
	for(int i = 0; i < buttonsN; ++i)
	{
		if(scancode == modifiableSCKeys[i+buttonsN])
		{
			if(action == GLFW_PRESS)
				keySend[1] |= 1 << i;
			else if(action == GLFW_RELEASE)
				keySend[1] &= ~(1 << i);
		}
	}


    //unmodifiable keys.
    if(action != GLFW_PRESS)
        return;
    switch (key){
    case GLFW_KEY_F1: //Set custom keys for player one
		SetupKeys(window, 0);
        break;
	case GLFW_KEY_F2: //and player two too
		SetupKeys(window, buttonsN);
        break;
	case GLFW_KEY_F3: //Set joy buttons
        {
        	int count;
			glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
			int i = 0;
			int used[count] = {0};
            while(i < 4)
            {
                const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
                for(int buttonItr = 0; buttonItr < count; ++buttonItr)
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
            std::ofstream keyfile("joyconf.bin", std::ofstream::out | std::ofstream::binary);
			keyfile.write((const char*)modifiableJoyKeys, sizeof(int)*4);
            keyfile.close();
        }
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(mainWindow, GL_TRUE);
        break;
    }
}

void GameLoopJoy()
{
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
}
