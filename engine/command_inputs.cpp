#include <iostream>
#include <bitset> //input display for testing
#include "command_inputs.h"
#include "raw_input.h"
#include "chara.h"
#include <deque>

#include <sol/sol.hpp>

constexpr int chargeBufSize = 32;

CommandInputs::CommandInputs()
{
	for(int i = 0; i < 4; i++)
	{
		chargeBuffer[i].resize(chargeBufSize);
	}
}

void CommandInputs::LoadFromLua(std::filesystem::path defFile)
{
	sol::state lua;
	auto result = lua.script_file(defFile.string());
	if(!result.valid()){
		sol::error err = result;
		std::cerr << "The code has failed to run!\n"
		          << err.what() << "\nPanicking and exiting..."
		          << std::endl;
		return;
	}
	sol::table inputTbl = lua["inputs"]["ground"];
	for(const auto &val : inputTbl)
	{
		sol::table arr = val.second;
		MotionData md;
		md.motionStr = arr["input"];
		md.bufLen = arr["buf"];
		md.seqRef = arr["ref"];
		motions.push_back(std::move(md));
	}
}

void CommandInputs::Charge(const input_deque &keyPresses)
{
	auto key = keyPresses[0] & ~key::buf::TRUE_NEUTRAL;
	for(int i = key::UP, b = 0; i <= key::RIGHT; ++i, ++b)
	{
		int dirBit = 1 << i;
		if( key & dirBit)
            charges[b] += 1;
		else
			charges[b] = 0;
		
		chargeBuffer[b].pop_back();
		chargeBuffer[b].push_front(charges[b]);
	}
}

int CommandInputs::GetCharge(dir which, int frame, int side)
{
	if(frame < 0 || frame >= chargeBufSize)
		return 0;
	
	constexpr int invertIndex[] {up,down,right,left};
	if(side == -1)
		return chargeBuffer[invertIndex[which]][frame];
	else
		return chargeBuffer[which][frame];
}

int CommandInputs::GetSequenceFromInput(const input_deque &keyPresses, int side)
{
	for(const auto &md : motions)
	{
		if(MotionInput(md, keyPresses, side))
			return md.seqRef;
	}
	return -1; //No matches
}

bool CommandInputs::MotionInput(const MotionData& md, const input_deque &keyPresses, int side)
{
	const std::string &motion = md.motionStr;
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

	int c = motion.size() - 1;  //Reads last character and goes backwards until 0.
	int frameCounter = md.bufLen;
	int lastKey = 0;
	int heldButton = 0;
	bool correct = false;

	const int bufSize = keyPresses.size();
	for(int i = 0; i < bufSize; i++)
	{
		auto key = keyPresses[i];
		auto lever = key & ~(key::buf::NEUTRAL); 

		if(lever & right && lever & left)
		{
			lever &= ~key::buf::RIGHT; //Left beats right? TODO: Revise

			if(lever & key::buf::DOWN)
				lever &= ~key::buf::LEFT; //makes half circles easier by transforming a simultaneous 123 into a 2.
		}
		if(lever & key::buf::DOWN && lever & key::buf::UP)//jumps if pressing both.
		{
			lever &= ~key::buf::DOWN;
		}

motionBufferProcessing:
		int lastC = c;
		switch (motion[c])
		{
		case '1':
			if(lever == (key::buf::DOWN | left))
				--c;
			break;

		case '2':
			if(lever == (key::buf::DOWN))
				--c;
			break;

		case '3':
			if(lever == (key::buf::DOWN | right)) //
				--c;
			break;

		case '4':
			if(lever == (left)) //
				--c;
			break;

		case '5':
			if(lever == 0)
				--c;
			break;

		case '6':
			if(lever == (right))
				--c;
			break;

		case '7':
			if(lever == (key::buf::UP | left))
				--c;
			break;


		case '8':
			if(lever == (key::buf::UP))
				--c;
			break;

		case '9':
			if(lever == (key::buf::UP | right))
				--c;
			break;

		case 'D': //Any down motion (i.e 1, 2 and 3)
			if(lever & key::buf::DOWN)
				--c;
			break;

		case 'L': //Any left
			if(lever & left)
				--c;
			break;

		case 'R': //Any right
			if(lever & right)
				--c;
			break;
		
		case 'a': //Button A
			if(key & key::buf::A)
			{
				--c;
				heldButton |= key::buf::A;
			}
			break;
		
		case 'b': //Button B
			if(key & key::buf::B)
			{
				--c;
				heldButton |= key::buf::B;
			}
			break;
		
		case 'c': //Button C
			if(key & key::buf::C)
			{
				--c;
				heldButton |= key::buf::C;
			}
			break;
		
		case 'd': //Button D
			if(key & key::buf::D)
			{
				--c;
				heldButton |= key::buf::D;
			}
			break;
		
		case '~': //Makes sure the buttons aren't being held.
			if(!(key & heldButton))
			{
				heldButton = 0;
				--c;
			}
			break;
		
		case '/': //Charge input. Requires 2 extra characters before it.
			if(c < 2)
				return false;

			dir toCheck;
			switch(motion[c-1])
			{
			case 'U':
				toCheck = dir::up;
				break;
			case 'D':
				toCheck = dir::down;
				break;
			case 'L':
				toCheck = dir::left;
				break;
			case 'R':
				toCheck = dir::right;
				break;
			}
			
			if(GetCharge(toCheck, i, side) >= (unsigned char)motion[c-2])
			{
				c -= 3;
			}
			break;
		
		case '0': //5 means any motion but anything containing the next one.
/* 			switch(motion[c+1]) //checks
			{
			case '2':
			case 'D':
				if(!(lever & key::buf::DOWN))
					--c;
				break;

			case '4':
			case 'L':
				if(!(lever & left)) //
					--c;
				break;

			case '6':
			case 'R':
				if(!(lever & right)) //
					--c;
				break;

			default:
				--c;
				break;
			} */
			break;

		default:
			std::bitset<32> x (lever);
			std::cout << "\tinvalid input(!):\n motion index: " << c << " case: __" << motion[c] << "__ input: " << x << "\n";
			return false;
		}

		if(c < 0)
			return true;

		if(lastC != c)
		{
			lastKey = key;
			frameCounter = md.bufLen;
			correct = true;
			goto motionBufferProcessing;
		}

		if(correct && lastKey != key) //Give extra buffer frames if they stopped holding the input
		{
			correct = false;
			frameCounter = md.bufLen;
		}
		
		--frameCounter;
		if(frameCounter <= 0)
			return false;
	}

	return false;
}
