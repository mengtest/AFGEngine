#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

#include "types.h"
#include <cstdint>
#include <string>
#include <vector>

struct CharFileHeader_old //old header
{
	uint16_t sequences_n;
	uint8_t version;
};

struct CharFileHeader //header for .char files.
{
	char signature[32];
	uint32_t version;
	uint16_t sequences_n;
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
	int32_t spriteIndex = 0;
	int32_t duration = 0;
	int32_t jumpTo = 0;
	int32_t jumpType = 0;
	bool relativeJump = false;

	uint32_t flags = 0;
	int32_t vel[2] = {0}; // x,y
	int32_t accel[2] = {0};
	int32_t movementType[2] = {0}; //Add or set X,Y

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
	//Attack_property attackProp;
	//Boxes are defined by BL, BR, TR, TL points, in that order.
	std::vector<int> greenboxes;
	std::vector<int> redboxes;
	std::vector<int> colbox;
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
	std::string function;
};

class Framedata
{
public:
	std::vector<Sequence> sequences;

public:
	bool Load(std::string charFile);
	bool LoadOld(std::string charFile);
	void Clear();
	void Close();
	void Save(std::string charFile);
	
	std::string GetDecoratedName(int n);
	bool loaded = false;
};


enum //impulses
{
		HITPUSHx,
		PUSHBACKx,
};


#endif // CHARACTER_H_INCLUDED
