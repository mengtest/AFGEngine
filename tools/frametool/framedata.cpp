#include "framedata.h"
#include "render.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#define rv(X) ((char*)&X)
#define rptr(X) ((char*)X)

constexpr const char *charSignature = "AFGECharacterFile";
constexpr uint32_t currentVersion = 99'2;

struct BoxSizes
{
	int8_t greens;
	int8_t reds;
	int8_t collision;
};

struct Frame_property_old
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

struct seqProp_old
{
	int level = 0;
	int metercost = 0;
	bool loops = false;
	int beginLoop = 0;
	int gotoSeq = 0;
	int machineState = 0;
};

bool Framedata::LoadOld(std::string charFile)
{
	//loads character from a file and fills sequences/frames and all that yadda.
	std::ifstream file(charFile, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file.\n";
		return false;
	}

	CharFileHeader header;
	file.read(rv(header), sizeof(CharFileHeader));
	if(strcmp(charSignature, header.signature))
	{
		std::cerr << "Signature mismatch.\n";
		return false;
	}
	if(header.version != 99'1)
	{
		std::cerr << "Format version mismatch.\n";
		return false;
	}

	{ //Ignore old table data
		int motionLenG, motionLenA;
		file.read(rv(motionLenG), sizeof(int));
		file.read(rv(motionLenA), sizeof(int));

		for (int i = 0; i < motionLenG+motionLenA; ++i)
		{
			file.ignore(sizeof(int));
			file.ignore(sizeof(int));

			int strSize;
			file.read(rv(strSize), sizeof(int));
			file.ignore(strSize);

			file.ignore(sizeof(char));
		}
	}

	sequences.resize(header.sequences_n);
	for (uint16_t i = 0; i < header.sequences_n; ++i)
	{
		auto &currSeq = sequences[i];
		uint8_t namelength;
		file.read(rv(namelength), sizeof(namelength));
		currSeq.name.resize(namelength);
		file.read(rptr(currSeq.name.data()), namelength);

		seqProp_old oldSeq;
		file.read(rv(oldSeq), sizeof(seqProp_old));
		currSeq.props.level = oldSeq.level;

		uint8_t seqlength;
		file.read(rv(seqlength), sizeof(seqlength));
		currSeq.frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			//How many boxes are used per frame
			BoxSizes bs;
			file.read(rv(bs), sizeof(BoxSizes));
			currFrame.greenboxes.resize(bs.greens);
			currFrame.redboxes.resize(bs.reds);
			currFrame.colbox.resize(bs.collision);

			Frame_property_old of;
			auto &nf = currFrame.frameProp;
			auto &nfa = currFrame.attackProp;
			file.read(rv(of), sizeof(Frame_property_old));
			nf.duration = of.duration;
			nf.state = of.state;
			nf.vel[0] = of.vel[0]*1000;
			nf.vel[1] = of.vel[1]*1000;
			nf.accel[0] = of.accel[0]*1000;
			nf.accel[1] = of.accel[1]*1000;
			nfa.damage[0] = of.damage[2];
			nfa.correction = of.proration*1000;
			nfa.meterGain = of.mgain[1];
			nf.flags = of.flags;

			file.read(rptr(currFrame.greenboxes.data()), sizeof(int) * bs.greens);
			file.read(rptr(currFrame.redboxes.data()), sizeof(int) * bs.reds);
			file.read(rptr(currFrame.colbox.data()), sizeof(int) * bs.collision);

			int spriteIndex;
			file.read(rv(spriteIndex), sizeof(int));
			currFrame.frameProp.spriteIndex = spriteIndex;
		}
	}

	file.close();
	loaded = true;
	return true;
}


bool Framedata::Load(std::string charFile)
{
	//loads character from a file and fills sequences/frames and all that yadda.
	std::ifstream file(charFile, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file.\n";
		return false;
	}

	CharFileHeader header;
	file.read(rv(header), sizeof(CharFileHeader));
	if(strcmp(charSignature, header.signature))
	{
		std::cerr << "Signature mismatch.\n";
		return false;
	}
	if(header.version != currentVersion)
	{
		std::cerr << "Format version mismatch.\n";
		return false;
	}

	sequences.resize(header.sequences_n);
	for (uint16_t i = 0; i < header.sequences_n; ++i)
	{
		auto &currSeq = sequences[i];
		uint8_t namelength;
		file.read(rv(namelength), sizeof(namelength));
		currSeq.name.resize(namelength);
		file.read(rptr(currSeq.name.data()), namelength);

		file.read(rv(currSeq.props), sizeof(seqProp));

		uint8_t seqlength;
		file.read(rv(seqlength), sizeof(seqlength));
		currSeq.frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			//How many boxes are used per frame
			BoxSizes bs;
			file.read(rv(bs), sizeof(BoxSizes));
			currFrame.greenboxes.resize(bs.greens);
			currFrame.redboxes.resize(bs.reds);
			currFrame.colbox.resize(bs.collision);

			file.read(rv(currFrame.frameProp), sizeof(Frame_property));
			file.read(rv(currFrame.attackProp), sizeof(Attack_property));

			file.read(rptr(currFrame.greenboxes.data()), sizeof(int) * bs.greens);
			file.read(rptr(currFrame.redboxes.data()), sizeof(int) * bs.reds);
			file.read(rptr(currFrame.colbox.data()), sizeof(int) * bs.collision);
		}
	}

	file.close();
	loaded = true;
	return true;
}

void Framedata::Save(std::string charFile)
{
	//loads character from a file and fills sequences/frames and all that yadda.
	std::ofstream file(charFile, std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file.\n";
		return;
	}

	CharFileHeader header;
	header.sequences_n = sequences.size();
	header.version = currentVersion;
	strncpy_s(header.signature, charSignature, 32);
	file.write(rv(header), sizeof(CharFileHeader));

	for (uint16_t i = 0; i < header.sequences_n; ++i)
	{
		auto &currSeq = sequences[i];
		uint8_t namelength = currSeq.name.size();
		file.write(rv(namelength), sizeof(namelength));
		file.write(rptr(currSeq.name.data()), namelength);

		file.write(rv(currSeq.props), sizeof(seqProp));

		uint8_t seqlength = currSeq.frames.size();
		file.write(rv(seqlength), sizeof(seqlength));
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			//How many boxes are used per frame
			BoxSizes bs;
			bs.greens = currFrame.greenboxes.size();
			bs.reds = currFrame.redboxes.size();
			bs.collision = currFrame.colbox.size();
			file.write(rv(bs), sizeof(BoxSizes));

			file.write(rv(currFrame.frameProp), sizeof(Frame_property));
			file.write(rv(currFrame.attackProp), sizeof(Attack_property));

			file.write(rptr(currFrame.greenboxes.data()), sizeof(int) * bs.greens);
			file.write(rptr(currFrame.redboxes.data()), sizeof(int) * bs.reds);
			file.write(rptr(currFrame.colbox.data()), sizeof(int) * bs.collision);
		}
	}

	file.close();
	return;
}

std::string Framedata::GetDecoratedName(int n)
{
	std::stringstream ss;
	ss.flags(std::ios_base::right);
	
	ss << std::setfill('0') << std::setw(3) << n << " " << sequences[n].name;
	return ss.str();
}

void Framedata::Clear()
{
	sequences.clear();
	sequences.resize(1000);
	loaded = true;
}

void Framedata::Close()
{
	sequences.clear();
	loaded = false;
}

/* 
glm::mat4 Framedata::GetSpriteTransform()
{
	glm::mat4 tranform(1);
	
	tranform = glm::scale(tranform, glm::vec3(spriteSide, 1.f, 1.f));
	tranform = glm::translate(tranform, glm::vec3((root.x)*spriteSide-128.f, root.y-40.f, 0));
	
	
	return tranform;
} */