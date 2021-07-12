#include "sprite.h"
#include <iostream>
#include <sol/sol.hpp>
#include <fstream>

struct VertexData1
{
	unsigned short x,y,s,t;
	unsigned short atlasId;
};

struct VertexData4
{
	unsigned short x,y,s,t;
};

void LoadToVao(Vao &VaoChar, const char* file)
{
	int nSprites, nChunks, indexed;
	std::ifstream vertexFile(file, std::ios_base::binary);

	vertexFile.read((char *)&indexed, sizeof(int));
	vertexFile.read((char *)&nSprites, sizeof(int));
	auto chunksPerSprite = new uint16_t[nSprites];
	vertexFile.read((char *)chunksPerSprite, sizeof(uint16_t)*nSprites);

	vertexFile.read((char *)&nChunks, sizeof(int));
	void* vertexData;
	int size;
	if(indexed == 0)
	{
		vertexData = malloc(sizeof(VertexData4)*nChunks*6);
		size = sizeof(VertexData4);
	}
	else
	{
		vertexData = malloc(sizeof(VertexData1)*nChunks*6);
		size = sizeof(VertexData1);
	}

	vertexFile.read((char *)vertexData, nChunks*6*size);
	unsigned int chunkCount = 0;
	for(int i = 0; i < nSprites; i++)
	{
		uint8_t strLen;
		vertexFile.read((char*)&strLen, 1);

		//Ignore namemap's name and index.
		vertexFile.ignore(strLen);
		vertexFile.ignore(sizeof(uint16_t));

		if(indexed == 0)
			VaoChar.Prepare(size*6*chunksPerSprite[i], &((VertexData4*)vertexData)[6*chunkCount]);
		else
			VaoChar.Prepare(size*6*chunksPerSprite[i], &((VertexData1*)vertexData)[6*chunkCount]);
		chunkCount += chunksPerSprite[i];
	}
	std::cout << "Chunks read: "<<chunkCount<<"/"<<nChunks<<"\n";
	vertexFile.close();

	VaoChar.Load();
	free(vertexData);
	delete[] chunksPerSprite;
}

Sprite::Sprite(const char* defFile):
indexed(Vao::F2F2I1, GL_STATIC_DRAW),
truecolor(Vao::F2F2, GL_STATIC_DRAW)
{
	sol::state lua;
	auto result = lua.script_file(defFile);
	if(!result.valid()){
		sol::error err = result;
		std::cerr << err.what() << std::endl;
	}
	sol::table graphics = lua["graphics"];
	for(const auto &entry : graphics)
	{
		sol::table arr = entry.second;
		std::string imageFile = arr["image"];
		std::string vertexFile = arr["vertex"];
		bool indexed = arr["indexed"];

		Texture texture;
		if(indexed)
			texture.Load(imageFile, true);
		else
			texture.Load(imageFile);
		
		texture.Apply();
		texture.Unload();
		textures.push_back(std::move(texture));
		
	}
}