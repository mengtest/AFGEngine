#ifndef FRAMEDATA_H_GUARD
#define FRAMEDATA_H_GUARD

#include <fixed_point.h>
#include <geometry.h>
#include <string>
#include <vector>
#include <filesystem>
#include <sol/sol.hpp>

constexpr int speedMultiplier = 240;

struct Frame_property
{
	int32_t spriteIndex = 0;
	int32_t duration = 0;
	int32_t jumpTo = 0;
	int32_t jumpType = 0;
	int32_t relativeJump = false;

	uint32_t flags = 0;
	int32_t vel[2] = {0}; // x,y
	int32_t accel[2] = {0};
	int32_t movementType[2] = {0}; //0 None. 1 Set-set. 2 Add-set. 3 Add-add

	int16_t cancelType[2] = {};
	int32_t state = 0;
	
	float spriteOffset[2]; //x,y
	int16_t loopN;
	int16_t chType;
	float scale[2];
	float color[4];
	int32_t blendType = 0;
	float rotation[3]; //XYZ
};

struct Frame
{
	
	Frame_property frameProp;
	sol::protected_function frameScript;
	bool hasFunction = false;
	//Attack_property attackProp;
	//Boxes are defined by BL, BR, TR, TL points, in that order.
	typedef std::vector<Rect2d<FixedPoint>> boxes_t;
	boxes_t greenboxes;
	boxes_t redboxes;
	Rect2d<FixedPoint> colbox;
};

struct seqProp
{
	int level = 0;
	int landFrame = 0;
	int zOrder = 0;
};

struct Sequence
{
	seqProp props;
	std::vector<Frame> frames;
	std::string name;
	sol::protected_function function;
	bool hasFunction = false;
};

bool LoadSequences(std::vector<Sequence> &sequences, std::filesystem::path charfile, sol::state &lua);

namespace flag
{
	enum //frame-dependent bit-mask flags
	{
		canMove = 0x1,
		dontWalk = 0x2,
		
		startHit = 0x8000'0000
	};
}

namespace jump
{
	enum
	{
		none = 0,
		frame,
		loop,
		seq
	};
}

enum state
{
	stand,
	crouch,
	air,
	otg,
};

#endif /* FRAMEDATA_H_GUARD */

