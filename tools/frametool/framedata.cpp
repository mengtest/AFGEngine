/* #include <cmath>
#include <deque>
*/

#include "framedata.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#define rv(X) ((char*)&X)
#define rptr(X) ((char*)X)

bool Framedata::Load(std::string charFile, NameMap &nameMap)
{
	//loads character from a file and fills sequences/frames and all that yadda.
	std::ifstream file(charFile, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file.\n";
		return false;
	}

	uint64_t sanity_check;
	uint16_t header_bytes;
	CharFileHeader h;

	file.read(rv(sanity_check), sizeof(uint64_t));
	if (sanity_check != SANITY_CHECK)
	{
		std::cerr << "This is not a proper character file.\n";
		return false;
	}

	file.read(rv(header_bytes), sizeof(uint16_t));
	if(header_bytes != sizeof(CharFileHeader))
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

	int motionLenG;
	int motionLenA;

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

		file.read((char *)&sequences[i].level, sizeof(int));
		file.read((char *)&sequences[i].metercost, sizeof(int));
		file.read((char *)&sequences[i].loops, sizeof(bool));
		file.read((char *)&sequences[i].beginLoop, sizeof(int));
		file.read((char *)&sequences[i].gotoSeq, sizeof(int));
		sequences[i].gotoSeq -= 1;

		file.read((char *)&sequences[i].machineState, sizeof(int));

		uint8_t seqlength;
		file.read((char *)&seqlength, sizeof(uint8_t));

		sequences[i].frames.resize(seqlength);
		sequences[i].frameNumber = seqlength; //TODO
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
	return true;

errorlog:
	std::cerr << charFile << "\n";
	file.close();
	return false;
}

std::string Framedata::GetDecoratedName(int n)
{
	std::stringstream ss;
	ss.flags(std::ios_base::right);
	
	ss << std::setfill('0') << std::setw(3) << n << " " << sequences[n].name;
	return ss.str();
}

/* 
glm::mat4 Framedata::GetSpriteTransform()
{
	glm::mat4 tranform(1);
	
	tranform = glm::scale(tranform, glm::vec3(spriteSide, 1.f, 1.f));
	tranform = glm::translate(tranform, glm::vec3((root.x)*spriteSide-128.f, root.y-40.f, 0));
	
	
	return tranform;
} */