#include "framedata.h"
#include <iostream>
#include <fstream>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#define rv(X) ((char*)&X)
#define rptr(X) ((char*)X)

struct CharFileHeader //header for .char files.
{
	char signature[32];
	uint32_t version;
	uint16_t sequences_n;
};

struct BoxSizes
{
	int8_t greens;
	int8_t reds;
	int8_t collision;
};

glm::mat4 CalculateTransform(float offset[2], float rotation[3], float scale[2])
{
	float rotX = rotation[0];
	float rotY = rotation[1];
	float rotZ = rotation[2];
	float offsetX = offset[0];
	float offsetY = offset[1];
	float scaleX = scale[0];
	float scaleY = scale[1];
	
	constexpr float tau = glm::pi<float>()*2.f;
	glm::mat4 transform = glm::scale(glm::mat4(1.f), glm::vec3(scaleX, scaleY, 1.f));
	transform = glm::rotate(transform, -rotZ*tau, glm::vec3(0.0, 0.f, 1.f));
	transform = glm::rotate(transform, rotY*tau, glm::vec3(0.0, 1.f, 0.f));
	transform = glm::rotate(transform, rotX*tau, glm::vec3(1.0, 0.f, 0.f));
	return transform;
}

bool LoadSequences(std::vector<Sequence> &sequences, std::filesystem::path charFile, sol::state &lua)
{
	constexpr const char *charSignature = "AFGECharacterFile";
	constexpr uint32_t currentVersion = 99'6;

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
		uint8_t strSize;
		file.read(rv(strSize), sizeof(strSize));
		currSeq.name.resize(strSize);
		file.read(rptr(currSeq.name.data()), strSize);

		file.read(rv(strSize), sizeof(strSize));
		std::string funcName((int)strSize, '\0');
		file.read(rptr(funcName.data()), strSize);

		////Game only////
		if(!funcName.empty())
		{
			currSeq.function = lua[funcName];
			if(currSeq.function.get_type() == sol::type::function)
				currSeq.hasFunction = true;
			else
				std::cerr << "Unknown function "<<funcName<<" in sequence "<<i<<"\n";
		}
		////Game only////

		file.read(rv(currSeq.props), sizeof(seqProp));

		uint8_t seqlength;
		file.read(rv(seqlength), sizeof(seqlength));
		currSeq.frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			file.read(rv(strSize), sizeof(strSize));
			std::string frameScript((int)strSize, '\0');
			file.read(rptr(frameScript.data()), strSize);
			////Game only////
			if(!frameScript.empty())
			{
				currFrame.frameScript = lua.load(frameScript);
				if(currFrame.frameScript.get_type() == sol::type::function && currFrame.frameScript.valid())
					currFrame.hasFunction = true;
				else
					std::cerr << "Invalid script in sequence "<<i<<" : frame "<<i2<<"\n";
			}
			////Game only////

			BoxSizes bs;
			file.read(rv(bs), sizeof(BoxSizes));
			std::vector<int> greens(bs.greens);
			std::vector<int> reds(bs.reds);
			std::vector<int> collision(bs.collision);

			file.read(rv(currFrame.frameProp), sizeof(Frame_property));

			//This side only. Precalculate matrix transformation
			auto &fp = currFrame.frameProp;
			currFrame.transform = CalculateTransform(fp.spriteOffset, fp.rotation, fp.scale);

			file.read(rptr(greens.data()), sizeof(int) * bs.greens);
			file.read(rptr(reds.data()), sizeof(int) * bs.reds);
			file.read(rptr(collision.data()), sizeof(int) * bs.collision);

			for(int bi = 0; bi < bs.greens; bi+=4)
			{
				currFrame.greenboxes.push_back(
					Rect2d<FixedPoint>(
						Point2d<FixedPoint>(greens[bi+0],greens[bi+1]),
						Point2d<FixedPoint>(greens[bi+2],greens[bi+3]))
				);
			}
			for(int bi = 0; bi < bs.reds; bi+=4)
			{
				currFrame.redboxes.push_back(
					Rect2d<FixedPoint>(
						Point2d<FixedPoint>(reds[bi+0],reds[bi+1]),
						Point2d<FixedPoint>(reds[bi+2],reds[bi+3]))
				);
			}

			if(bs.collision>0)
			{
				sequences[i].frames[i2].colbox.bottomLeft.x = collision[0];
				sequences[i].frames[i2].colbox.bottomLeft.y = collision[1];
				sequences[i].frames[i2].colbox.topRight.x = collision[2];
				sequences[i].frames[i2].colbox.topRight.y = collision[3];
			}
		}
	}

	return true;
}