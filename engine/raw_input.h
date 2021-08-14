#ifndef RAW_INPUT_H_INCLUDED
#define RAW_INPUT_H_INCLUDED
#include <stdint.h>
#include <SDL.h>

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
		enum : uint32_t //Bit-mask for the keys pressed on each frame to be send to the key buffer
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
			CUT = 0x8000'0000 //Game uses it to stop processing inputs when it sees it.
		};
	}
}

const int buttonsN = key::END;

extern unsigned int keySend[2];
extern SDL_Scancode modifiableSCKeys[buttonsN*2];
extern int modifiableJoyKeys[4];

void GameLoopJoy(); //Called directly to poll Joy stick/pad status.
//Callbacks that are called when polling anyway.
void GameLoopKeyHandle(SDL_KeyboardEvent &e); //In-game input processing.
void SetupKeys(int offset); //Sets up and uses the callback to configure keys.
void EventLoop();


#endif // RAW_INPUT_H_INCLUDED
