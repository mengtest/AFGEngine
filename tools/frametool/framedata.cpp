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
constexpr uint32_t currentVersion = 99'1;

struct BoxSizes
{
	int8_t greens;
	int8_t reds;
	int8_t collision;
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
	if(header.version != 99'0)
	{
		std::cerr << "Format version mismatch.\n";
		return false;
	}

	{ //To be removed
		int table_size;
		file.read(rv(table_size), sizeof(int));
		file.ignore( sizeof(int) * table_size);
		file.ignore( sizeof(int) * table_size);

		file.read(rv(motionLenG), sizeof(int));
		file.read(rv(motionLenA), sizeof(int));

		for (int i = 0; i < motionLenG; ++i)
		{
			file.read(rv(motionListDataG[i].bufLen), sizeof(int));
			file.read(rv(motionListDataG[i].seqRef), sizeof(int));

			int strSize;
			file.read(rv(strSize), sizeof(int));

			std::string fullMotionStr(strSize, '\0');
			file.read(rptr(fullMotionStr.data()), strSize);

			file.read(rv(motionListDataG[i].button), sizeof(char));

			motionListDataG[i].motionStr = fullMotionStr;
		}

		for (int i = 0; i < motionLenA; ++i)
		{
			file.read((char *)&motionListDataA[i].bufLen, sizeof(int));
			file.read((char *)&motionListDataA[i].seqRef, sizeof(int));

			int strSize;
			file.read((char *)&strSize, sizeof(int));

			std::string fullMotionStr(strSize, '\0');

			file.read((char *)fullMotionStr.data(), strSize);
			file.read(&motionListDataA[i].button, sizeof(char));

			motionListDataA[i].motionStr = fullMotionStr;
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

			file.read(rptr(currFrame.greenboxes.data()), sizeof(int) * bs.greens);
			file.read(rptr(currFrame.redboxes.data()), sizeof(int) * bs.reds);
			file.read(rptr(currFrame.colbox.data()), sizeof(int) * bs.collision);

			file.read(rv(currFrame.spriteIndex), sizeof(int));
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

	{ //To be removed
		file.read(rv(motionLenG), sizeof(int));
		file.read(rv(motionLenA), sizeof(int));

		for (int i = 0; i < motionLenG; ++i)
		{
			file.read(rv(motionListDataG[i].bufLen), sizeof(int));
			file.read(rv(motionListDataG[i].seqRef), sizeof(int));

			int strSize;
			file.read(rv(strSize), sizeof(int));

			std::string fullMotionStr(strSize, '\0');
			file.read(rptr(fullMotionStr.data()), strSize);

			file.read(rv(motionListDataG[i].button), sizeof(char));

			motionListDataG[i].motionStr = fullMotionStr;
		}

		for (int i = 0; i < motionLenA; ++i)
		{
			file.read((char *)&motionListDataA[i].bufLen, sizeof(int));
			file.read((char *)&motionListDataA[i].seqRef, sizeof(int));

			int strSize;
			file.read((char *)&strSize, sizeof(int));

			std::string fullMotionStr(strSize, '\0');

			file.read((char *)fullMotionStr.data(), strSize);
			file.read(&motionListDataA[i].button, sizeof(char));

			motionListDataA[i].motionStr = fullMotionStr;
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

			file.read(rptr(currFrame.greenboxes.data()), sizeof(int) * bs.greens);
			file.read(rptr(currFrame.redboxes.data()), sizeof(int) * bs.reds);
			file.read(rptr(currFrame.colbox.data()), sizeof(int) * bs.collision);

			file.read(rv(currFrame.spriteIndex), sizeof(int));
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

	file.write(rv(motionLenG), sizeof(int));
	file.write(rv(motionLenA), sizeof(int));

	for (int i = 0; i < motionLenG; ++i)
	{
		file.write(rv(motionListDataG[i].bufLen), sizeof(int));
		file.write(rv(motionListDataG[i].seqRef), sizeof(int));

		int strSize = motionListDataG[i].motionStr.size();
		file.write(rv(strSize), sizeof(int));
		file.write(rptr(motionListDataG[i].motionStr.data()), strSize);
		file.write(rv(motionListDataG[i].button), sizeof(char));
	}

	for (int i = 0; i < motionLenA; ++i)
	{
		file.write(rv(motionListDataA[i].bufLen), sizeof(int));
		file.write(rv(motionListDataA[i].seqRef), sizeof(int));

		int strSize = motionListDataA[i].motionStr.size();
		file.write(rv(strSize), sizeof(int));
		file.write(rptr(motionListDataA[i].motionStr.data()), strSize);
		file.write(rv(motionListDataA[i].button), sizeof(char));
	}

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

			file.write(rptr(currFrame.greenboxes.data()), sizeof(int) * bs.greens);
			file.write(rptr(currFrame.redboxes.data()), sizeof(int) * bs.reds);
			file.write(rptr(currFrame.colbox.data()), sizeof(int) * bs.collision);

			file.write(rv(currFrame.spriteIndex), sizeof(int));
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