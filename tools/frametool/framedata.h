#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED
/* 
#include <deque>


#include <utility>

 */

//#include <fixed_point.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::unordered_map<std::string, uint16_t> NameMap;

namespace flag
{
	enum //frame-dependent bit-mask flags
	{
		FRICTION = 0x1,
		GRAVITY = 0x2,
		KEEP_VEL = 0x4,
		KEEP_ACC = 0x8,
		CROUCH_BLOCK = 0x10,
		STAND_BLOCK = 0x20,
		AIR_BLOCK = 0x40,
		SINGLE_HIT = 0x80, //unused 

		CANCELLABLE = 0x100,
		CANCEL_WHIFF = 0x200,
		JUMP_C_HIT = 0x400, //u
		JUMP_C_BLOCK = 0x800, //u
		_UNUSED13 = 0x1000,
		_UNUSED14 = 0x2000,
		_UNUSED15 = 0x4000,
		IGNORE_INPUT = 0x8000,

		RESET_INFLICTED_VEL = 0x10000,
		_UNUSED18 = 0x20000, //No sprite mirroring (used only by frame 0)
	};
}

namespace pain
{
	enum
	{
		HIGH,
		LOW,
	};
}

namespace state
{
	enum //Seq-wise. Probably getting scrapped.
	{
		GROUNDED,
		AIRBORNE,
		BUSY_GRND,
		BUSY_AIR,
		PAIN_GRND,
		PAIN_AIR,
	};

	namespace fr //frame
	{
		enum
		{
			STANDING,
			CROUCHED,
			AIRBORNE,
			OTG,
		};
	}
}

struct CharFileHeader //header for .char files.
{
	uint16_t sequences_n;
	uint8_t version;
};

struct Motion_Data
{
	int bufLen = 0;
	int seqRef = 0;
	std::string motionStr; //Without the button press.
	char button = 0;
};

struct Frame_property
{
	int duration = 0;
	uint32_t flags = 0;
	float vel[2] = {0}; // x,y
	float accel[2] = {0};

	int damage[4] = {0}; // P-damage on hit and block plus R-damage on hit and block. 0 and 1 are unused
	float proration = 0;
	int mgain[2] = {0}; //Meter gain on hit and block respectively.
	int hitstun = 0;
	int blockstun = 0;
	int ch_stop = 0;
	int hitstop = 0;
	float push[2] = {0}; //(x,y) speed to be added to foe when he gets hit.
	float pushback[2] = {0}; //X pushback on hit and block respectively.

	int state = 0;

	int painType = 0;
	float spriteOffset[2]; //x,y
};

struct Frame
{
	Frame_property frameProp;
	//Boxes are defined by BL, BR, TR, TL points, in that order.
	std::vector<int> greenboxes;
	std::vector<int> redboxes;
	std::vector<int> colbox;

	int spriteIndex = 0;
};

struct Sequence
{
	int frameNumber = 0;
	int level = 0;
	int metercost = 0;
	bool loops = false;
	int beginLoop = 0;
	int gotoSeq = 0;
	int machineState = 0;
	std::vector<Frame> frames;
	std::string name;
};

class Framedata
{
public:
	std::vector<Sequence> sequences;

	int actTableG[64]; //Array to translate act:: constants to their assigned sequence.
	int actTableA[64]; //Aerial counterpart
	Motion_Data motionListDataG[32]; //List of motion inputs. Not only for special attack usage.
	Motion_Data motionListDataA[32];

	const uint64_t SANITY_CHECK = 0x1d150c10c001506f;

public:
	bool Load(std::string charFile, std::unordered_map<std::string, uint16_t> &nameMap);
	std::string GetDecoratedName(int n);
};


enum //impulses
{
		HITPUSHx,
		PUSHBACKx,
};


#endif // CHARACTER_H_INCLUDED
