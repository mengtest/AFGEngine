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
constexpr uint32_t currentVersion = 99'6;

struct BoxSizes
{
	int8_t greens;
	int8_t reds;
	int8_t collision;
};

struct seqProp_OLD
{
	int level = 0;
	int landFrame = 0;
	int zOrder = 0;
};

bool Framedata::LoadOlder(std::string charFile)
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
	if(header.version != 99'3)
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

		file.read(rv(currSeq.props), sizeof(seqProp_OLD));

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
		}
	}

	file.close();
	loaded = true;
	return true;
}

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
	if(header.version != 99'5)
	{
		std::cerr << "Format version mismatch.\n";
		return false;
	}

	sequences.resize(header.sequences_n);
	for (uint16_t i = 0; i < header.sequences_n; ++i)
	{
		auto &currSeq = sequences[i];
		uint8_t size;
		file.read(rv(size), sizeof(size));
		currSeq.name.resize(size);
		file.read(rptr(currSeq.name.data()), size);

		file.read(rv(size), sizeof(size));
		currSeq.function.resize(size);
		file.read(rptr(currSeq.function.data()), size);

		file.read(rv(currSeq.props), sizeof(seqProp_OLD));

		uint8_t seqlength;
		file.read(rv(seqlength), sizeof(seqlength));
		currSeq.frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			file.read(rv(size), sizeof(size));
			currFrame.frameScript.resize(size);
			file.read(rptr(currFrame.frameScript.data()), size);

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
		uint8_t size;
		file.read(rv(size), sizeof(size));
		currSeq.name.resize(size);
		file.read(rptr(currSeq.name.data()), size);

		file.read(rv(size), sizeof(size));
		currSeq.function.resize(size);
		file.read(rptr(currSeq.function.data()), size);

		file.read(rv(currSeq.props), sizeof(seqProp));

		uint8_t seqlength;
		file.read(rv(seqlength), sizeof(seqlength));
		currSeq.frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			file.read(rv(size), sizeof(size));
			currFrame.frameScript.resize(size);
			file.read(rptr(currFrame.frameScript.data()), size);

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
		uint8_t strSize = currSeq.name.size();
		file.write(rv(strSize), sizeof(strSize));
		file.write(rptr(currSeq.name.data()), strSize);

		strSize = currSeq.function.size();
		file.write(rv(strSize), sizeof(strSize));
		file.write(rptr(currSeq.function.data()), strSize);

		file.write(rv(currSeq.props), sizeof(seqProp));

		uint8_t seqlength = currSeq.frames.size();
		file.write(rv(seqlength), sizeof(seqlength));
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			strSize = currFrame.frameScript.size();
			file.write(rv(strSize), sizeof(strSize));
			file.write(rptr(currFrame.frameScript.data()), strSize);

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