#include <iostream>
#include <bitset> //input display for testing
#include "chara_input.h"
#include "raw_input.h"
#include "chara.h"
#include <deque>

//This is going to get removed
#define BTN_CHECK(key) {				\
	if(key & key::buf::A) 				\
		return act::A5; 				\
	else if(key & key::buf::B) 			\
		return act::B5; 				\
	else if(key & key::buf::C) 			\
		return act::C5; 				\
	else if(key & key::buf::D) 			\
		return act::D5;					\
}

int InstantInput(input_deque *keyPresses, float side, Motion_Data* motions)
{
	int key = (*keyPresses)[0];

	int left;
	int right;
	if(side == -1) //Inverts absolute input depending on side. Apparent input is corrected.
	{
		left = key::buf::RIGHT;
		right = key::buf::LEFT;
	}
	else
	{
		left = key::buf::LEFT;
		right = key::buf::RIGHT;
	}

	if(key & right && key & left)//Avoids a nasty bug involving an instant apparent right to left input which messed up mirroring.
	{
		key &= ~key::buf::RIGHT;

		if(key & key::buf::DOWN)
			key &= ~key::buf::LEFT; //makes half circles easier by transforming a simultaneous 123 into a 2.
	}
	if(key & key::buf::DOWN && key & key::buf::UP)//jumps if pressing both.
	{
		key &= ~key::buf::DOWN;
	}

	for(int i = key::A; i < key::D+1; ++i) //Checks if a button (A to D) is is being held
	{
		int atkBtnCheck = 1 << i;
		if( key & atkBtnCheck && //is previous button still being held?
			(*keyPresses)[1] & atkBtnCheck)
		{
            key &= ~atkBtnCheck; //If so ignore it.
		}
	}

/*
	std::bitset<32> x(key);
	std::cout << x << "\n";
*/

	int i = 0;
	while(motions[i].bufLen > 0) //It's impossible for the buffer length to be 0 unless it was left uninitialized.
	{
		if(motions[i].button == '0' && MotionInput(motions[i].motionStr, keyPresses, motions[i].bufLen, side))
		{
			return 0x1000 + motions[i].seqRef; //0x1000 is added for me to know if it returned a motion action
		}
		else
		{
			int buttonChk = 1 << (motions[i].button - 'A' + key::A); //Sets the bitflag corresponding to the assigned button of the motion.
			if((key & buttonChk) && MotionInput(motions[i].motionStr, keyPresses, motions[i].bufLen, side))
			{
				return 0x1000 + motions[i].seqRef;
			}
		}
		++i;
	}

	if(key & key::buf::UP && key & right)
	{
		BTN_CHECK(key)
		return act::UP_RIGHT;
	}

	else if(key & key::buf::UP && key & left)
	{
		BTN_CHECK(key)
		return act::UP_LEFT;
	}

	else if(key & key::buf::UP)
	{
		BTN_CHECK(key)
		return act::UP;
	}

    else if(key & key::buf::DOWN && key & right)
	{
		if(key & key::buf::A)
			return act::A2;
		else if(key & key::buf::B)
			return act::B3;
		else if(key & key::buf::C)
			return act::C3;
		else if(key & key::buf::D)
			return act::D2;
		return act::DOWN_RIGHT;
	}


	else if(key & key::buf::DOWN && key & left)
	{
		if(key & key::buf::A)
			return act::A2;
		else if(key & key::buf::B)
			return act::B2;
		else if(key & key::buf::C)
			return act::C2;
		else if(key & key::buf::D)
			return act::D2;
		return act::DOWN_LEFT;
	}

	else if(key & key::buf::DOWN)
	{
		if(key & key::buf::A)
			return act::A2;
		else if(key & key::buf::B)
			return act::B2;
		else if(key & key::buf::C)
			return act::C2;
		else if(key & key::buf::D)
			return act::D2;
		return act::DOWN;
	}

    else if(key & right)
	{
		if(key & key::buf::A)
			return act::A6;
		else if(key & key::buf::B)
			return act::B6;
		else if(key & key::buf::C)
			return act::C6;
		else if(key & key::buf::D)
			return act::D5;
		return act::RIGHT;
	}


	else if(key & left)
	{
		if(key & key::buf::A)
			return act::A5;
		else if(key & key::buf::B)
			return act::B4;
		else if(key & key::buf::C)
			return act::C4;
		else if(key & key::buf::D)
			return act::D5;
		return act::LEFT;
	}

	else if(key & key::buf::NEUTRAL) //must be always last
	{
		BTN_CHECK(key)
		return act::NEUTRAL;
	}

	return act::NOTHING;
}

bool MotionInput(std::string motion, input_deque *keyPresses, int window, float side)
{
	int left;
	int right;

	int strLen = motion.size();
	if(side == -1) //Inverts absolute input depending on side. Apparent input is corrected.
	{
		left = key::buf::RIGHT;
		right = key::buf::LEFT;
	}
	else
	{
		left = key::buf::LEFT;
		right = key::buf::RIGHT;
	}

	int motionIndex = 0;
	for(int i = window-1; i >= 0; --i)
	{
		//std::cout << i << " window\n";
		int key = (*keyPresses)[i];

		key &= ~(key::buf::NEUTRAL); //ignores all keypresses but movement; sets them to 0 for correct comparison.

		if(key & key::buf::DOWN && key & key::buf::UP)//jumps if pressing both.
		{
			key &= ~key::buf::DOWN;
		}



		switch (motion[motionIndex]) //numpad notation
		{
		case '1':
			if(key == (key::buf::DOWN | left))
				++motionIndex;
			break;

		case '2':
			if(key == (key::buf::DOWN))
				++motionIndex;
			break;

		case '3':
			if(key == (key::buf::DOWN | right)) //
				++motionIndex;
			break;

		case '4':
			if(key == (left)) //
				++motionIndex;
			break;

		case '5': //5 means any motion but anything containing the next one.
			switch(motion[motionIndex+1]) //checks
			{
			case '2':
			case 'D':
				if(!(key & key::buf::DOWN))
					++motionIndex;
				break;

			case '4':
			case 'L':
				if(!(key & left)) //
					++motionIndex;
				break;

			case '6':
			case 'R':
				if(!(key & right)) //
					++motionIndex;
				break;

			default:
				++motionIndex;
				break;
			}
			break;

		case '6':
			if(key == (right))
				++motionIndex;
			break;

		case '7':
			if(key == (key::buf::UP | left))
				++motionIndex;
			break;


		case '8':
			if(key == (key::buf::UP))
				++motionIndex;
			break;

		case '9':
			if(key == (key::buf::UP | right))
				++motionIndex;
			break;

		case 'D': //Any down motion (i.e 1, 2 and 3)
			if(key & key::buf::DOWN)
				++motionIndex;
			break;

		case 'L': //Any left
			if(key & left)
				++motionIndex;
			break;

		case 'R': //Any right
			if(key & right)
				++motionIndex;
			break;

		default:
			std::bitset<32> x (key);
			std::cout << "\tinvalid input(!):\n motion index: " << motionIndex << " case: __" << motion[motionIndex] << "__ input: " << x << "\n";
			return false;
		}

		if(motionIndex >= strLen)
			return true;
	}

	return false;
}
