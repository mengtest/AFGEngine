#include "framedata.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#define rv(X) ((char*)&X)
#define rptr(X) ((char*)X)

constexpr const char *charSignature = "AFGECharacterFile";
constexpr uint32_t currentVersion = 99'0;

struct BoxSizes
{
	int8_t greens;
	int8_t reds;
	int8_t collision;
};

//Ver 5 old loading function.
bool Framedata::LoadV5(std::string charFile, NameMap &nameMap)
{
	const uint64_t SANITY_CHECK = 0x1d150c10c001506f;
	//loads character from a file and fills sequences/frames and all that yadda.
	std::ifstream file(charFile, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file.\n";
		return false;
	}

	uint64_t sanity_check;
	uint16_t header_bytes;
	CharFileHeader_old h;

	file.read(rv(sanity_check), sizeof(uint64_t));
	if (sanity_check != SANITY_CHECK)
	{
		std::cerr << "This is not a proper character file.\n";
		return false;
	}

	file.read(rv(header_bytes), sizeof(uint16_t));
	if(header_bytes != sizeof(CharFileHeader_old))
	{
		std::cerr << "File header size discrepancy.\n";
		return false;
	}

	file.read(rv(h), header_bytes);

	int table_size;
	file.read(rv(table_size), sizeof(int));
	file.read(rptr(actTableG), sizeof(int) * table_size);
	file.read(rptr(actTableA), sizeof(int) * table_size);

	for (int i = 0; i < 64; ++i)
	{
		actTableG[i] -= 1;
		actTableA[i] -= 1;
	}

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

	sequences.resize(h.sequences_n);
	for (uint16_t i = 0; i < h.sequences_n; ++i)
	{
		uint8_t namelength = 0;
		file.read((char *)&namelength, sizeof(uint8_t));
		sequences[i].name.resize(namelength);
		file.read(rptr(sequences[i].name.data()), namelength);

		file.read((char *)&sequences[i].props.level, sizeof(int));
		file.read((char *)&sequences[i].props.metercost, sizeof(int));
		file.read((char *)&sequences[i].props.loops, sizeof(bool));
		file.read((char *)&sequences[i].props.beginLoop, sizeof(int));
		file.read((char *)&sequences[i].props.gotoSeq, sizeof(int));
		file.read((char *)&sequences[i].props.machineState, sizeof(int));
		sequences[i].props.gotoSeq -= 1;

		uint8_t seqlength;
		file.read((char *)&seqlength, sizeof(uint8_t));

		sequences[i].frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			file.ignore(sizeof(float) * 8); //deprecated imagepos

			//How many boxes are used per frame
			int16_t activeGreens;
			int16_t activeReds;
			int8_t activeCols; //as in collision box, not column.
			file.read((char *)&activeGreens, sizeof(int16_t));
			file.read((char *)&activeReds, sizeof(int16_t));
			file.read((char *)&activeCols, sizeof(int8_t));

			sequences[i].frames[i2].greenboxes.resize(activeGreens/2);
			sequences[i].frames[i2].redboxes.resize(activeReds/2);
			sequences[i].frames[i2].colbox.resize(activeCols/2);

			uint16_t framePropSize = 0;
			file.read(rv(framePropSize), sizeof(uint16_t)); 
			file.read((char *)&sequences[i].frames[i2].frameProp, framePropSize);

			float greenboxes[32*4*2];
			float redboxes[32*4*2];
			float colbox[1*4*2];
			file.read(rptr(greenboxes), sizeof(float) * activeGreens);
			file.read(rptr(redboxes), sizeof(float) * activeReds);
			file.read(rptr(colbox), sizeof(float) * activeCols);
			for(int bi = 0; bi < activeGreens; bi+=8){
				sequences[i].frames[i2].greenboxes[bi/2+0] = greenboxes[bi];
				sequences[i].frames[i2].greenboxes[bi/2+1] = greenboxes[bi+1];
				sequences[i].frames[i2].greenboxes[bi/2+2] = greenboxes[bi+4];
				sequences[i].frames[i2].greenboxes[bi/2+3] = greenboxes[bi+5];
			}
			for(int bi = 0; bi < activeReds; bi+=8){
				sequences[i].frames[i2].redboxes[bi/2+0] = redboxes[bi];
				sequences[i].frames[i2].redboxes[bi/2+1] = redboxes[bi+1];
				sequences[i].frames[i2].redboxes[bi/2+2] = redboxes[bi+4];
				sequences[i].frames[i2].redboxes[bi/2+3] = redboxes[bi+5];
			}
			for(int bi = 0; bi < activeCols; bi+=8){
				sequences[i].frames[i2].colbox[bi/2+0] = colbox[bi];
				sequences[i].frames[i2].colbox[bi/2+1] = colbox[bi+1];
				sequences[i].frames[i2].colbox[bi/2+2] = colbox[bi+4];
				sequences[i].frames[i2].colbox[bi/2+3] = colbox[bi+5];
			}

			uint16_t filepathLenght;
			file.read((char *)&filepathLenght, sizeof(uint16_t));

			if (filepathLenght > 0) //If this string exist then extract the rest and save it.
			{
				//Why is this always the same as the path lenght???
				uint16_t filenameLenght;
				file.read((char *)&filenameLenght, sizeof(uint16_t));

				std::string filepath(filepathLenght, '\0');
				std::string filename(filenameLenght, '\0');

				file.read((char *)filepath.data(), sizeof(char) * filepathLenght);
				file.read((char *)filename.data(), sizeof(char) * filenameLenght);

				filename.erase(std::find(filename.begin(), filename.end(), '\0'), filename.end());

				sequences[i].frames[i2].spriteIndex = nameMap.at(filename);
			}
		}
	}

	file.close();
	loaded = true;
	return true;

errorlog:
	std::cerr << charFile << "\n";
	file.close();
	return false;
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
		int table_size;
		file.read(rv(table_size), sizeof(int));
		file.read(rptr(actTableG), sizeof(int) * table_size);
		file.read(rptr(actTableA), sizeof(int) * table_size);

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
	header.version = 99'0;
	strncpy_s(header.signature, charSignature, 32);
	file.write(rv(header), sizeof(CharFileHeader));

	int table_size = 64;
	file.write(rv(table_size), sizeof(int));
	file.write(rptr(actTableG), sizeof(int) * table_size);
	file.write(rptr(actTableA), sizeof(int) * table_size);

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