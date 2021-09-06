#ifndef RAW_INPUT_H_INCLUDED
#define RAW_INPUT_H_INCLUDED

#include <functional>
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

struct JoyInputInfo
{
	SDL_JoystickID id; //Idk
	int type; //0 Button //1 Axis
	uint8_t axis;
	uint8_t button;
	enum{
		BUTTON,
		AXIS
	};
};

constexpr int buttonsN = key::END;

extern short deadZone;
extern unsigned int keySend[2];
extern SDL_Scancode modifiableSCKeys[buttonsN*2];
extern JoyInputInfo modifiableJoyKeys[buttonsN*2];
extern std::unordered_map<SDL_JoystickID, int> JoyInstanceIds;

void InitControllers(std::vector<SDL_GameController*> &controllers);
void SetupKeys(int offset); //Sets up and uses the callback to configure keys.
void SetupJoy(int offset); //Sets up and uses the callback to configure keys.
void EventLoop(std::function<bool(const SDL_KeyboardEvent&)> keyHandler);

#endif // RAW_INPUT_H_INCLUDED
