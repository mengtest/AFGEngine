#ifndef RAW_INPUT_H_INCLUDED
#define RAW_INPUT_H_INCLUDED
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace key //Key press as an int after being processed.
{
	enum //plain enum used for the order of configurable keys
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,

		A,
		B,
		C,
		D,
		//E,
		END,
	};
	namespace buf
	{
		enum //Bit-mask for the keys pressed on each frame to be send to the key buffer
		{
			UP = 0x1,
			DOWN = 0x2,
			LEFT = 0x4,
			RIGHT = 0x8,
			A = 0x10,
			B = 0x20,
			C = 0x40,
			D = 0x80,
			//E = 0x100,
			NEUTRAL = ~( UP | DOWN | LEFT | RIGHT ), //this one is for checking neutral input
			TRUE_NEUTRAL = 0x80000000 //this one is for setting neutral input manually, without absolutely any keypresses.
			//most significant bit is always set to 1 because you can't compare against 'NEUTRAL' properly without it.
		};
	}
}

const int buttonsN = key::END;

extern unsigned int keySend[2];
extern int modifiableSCKeys[buttonsN*2];
extern int modifiableJoyKeys[4];

void GameLoopJoy(); //Called directly to poll Joy stick/pad status.
//Callbacks that are called when polling anyway.
void CbGameLoopKeys(GLFWwindow* window, int key, int scancode, int action, int mods); //In-game input processing.
void CbSetupKeys(GLFWwindow* window, int key, int scancode, int action, int mods);  //Auxiliary callback to setup keys. Temporary, as always.
void SetupKeys(GLFWwindow* window, int offset); //Sets up and uses the callback to configure keys.

#endif // RAW_INPUT_H_INCLUDED
