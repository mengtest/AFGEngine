#include "gfx_handler.h"
#include <iostream>
#include <sol/sol.hpp>
#include <fstream>

GfxHandler::GfxHandler():
vertices(Vao::F2F2_short, GL_STATIC_DRAW)
{
	rectS.LoadShader("data/def.vert", "data/defRect.frag");
	indexedS.LoadShader("data/def.vert", "data/palette.frag");
	indexedS.Use();
	//Set texture unit indexes
	glUniform1i(indexedS.GetLoc("tex0"), 0 ); 
	glUniform1i(indexedS.GetLoc("palette"), 1 );
	paletteSlotL = indexedS.GetLoc("paletteSlot");
}

int GfxHandler::LoadGfxFromDef(std::filesystem::path defFile)
{
	auto folder = defFile.parent_path();
	sol::state lua;
	auto result = lua.script_file(defFile.string());
	if(!result.valid()){
		sol::error err = result;
		std::cerr << err.what() << std::endl;
	}
	sol::table graphics = lua["graphics"];

	int mapId = idMapList.size();
	idMapList.push_back({});
	for(const auto &entry : graphics)
	{
		sol::table arr = entry.second;
		std::string imageFile = arr["image"];
		std::string vertexFile = arr["vertex"];
		bool indexed = arr["indexed"];

		Texture texture;
		if(indexed)
		{
			texture_options opt;
			opt.rectangle = true;
			texture.LoadPng(folder/imageFile, opt);
		}
		else
		{
			texture_options opt;
			opt.linearFilter = true;
			opt.rectangle = true;
			texture.LoadPng(folder/imageFile, opt);
		}
		textures.push_back(std::move(texture));
		LoadToVao(folder/vertexFile, mapId, textures.size()-1);
	}
	return mapId;
}

void GfxHandler::LoadToVao(std::filesystem::path file, int mapId, int textureIndex)
{
	int nSprites, nChunks, indexed;
	std::ifstream vertexFile(file, std::ios_base::binary);

	vertexFile.read((char *)&nSprites, sizeof(int));
	auto chunksPerSprite = new uint16_t[nSprites];
	vertexFile.read((char *)chunksPerSprite, sizeof(uint16_t)*nSprites);

	vertexFile.read((char *)&nChunks, sizeof(int));

	int vdStart = tempVDContainer.size();
	//tempVDContainer.push_back(std::make_unique<VertexData4[]>(nChunks*6));
	tempVDContainer.emplace_back(new VertexData4[nChunks*6]);
	int size = sizeof(VertexData4);

	vertexFile.read((char *)(tempVDContainer[vdStart].get()), nChunks*6*size);
	unsigned int chunkCount = 0;
	for(int i = 0; i < nSprites; i++)
	{
		uint16_t virtualId;
		vertexFile.read((char*)&virtualId, sizeof(virtualId));

		int trueId = vertices.Prepare(size*6*chunksPerSprite[i], &(tempVDContainer[vdStart].get()[6*chunkCount]));
		chunkCount += chunksPerSprite[i];

		idMapList[mapId].insert({virtualId, {trueId, textureIndex}});
	}

	delete[] chunksPerSprite;
}

void GfxHandler::LoadingDone()
{
	vertices.Load();
	tempVDContainer.clear();
	loaded = true;
}

void GfxHandler::Begin()
{
	vertices.Bind();
}

void GfxHandler::End()
{
	boundTexture = -1;
	boundProgram = -1;
}

void GfxHandler::Draw(int id, int defId)
{
	auto search = idMapList[defId].find(id);
    if (search != idMapList[defId].end())
	{
        auto meta = search->second;
		if(boundTexture != meta.textureIndex)
		{
			boundTexture = meta.textureIndex;
			glBindTexture(GL_TEXTURE_RECTANGLE, textures[boundTexture].id);
		}
		int nextProgram = textures[boundTexture].is8bpp;
		if(boundProgram != nextProgram)
		{
			boundProgram = nextProgram;
			if(nextProgram == 1)
			{
				indexedS.Use();
			}
			else
				rectS.Use();
		}
		
		if(boundProgram == 1)
			glUniform1i(paletteSlotL, paletteSlot);
		vertices.Draw(meta.trueId);
    }
}

void GfxHandler::SetPaletteSlot(int palette)
{
	paletteSlot = palette;
}

int GfxHandler::GetVirtualId(int id, int defId)
{
	for(auto search: idMapList[defId])
	{
		if(search.second.trueId == id)
			return search.first;
	}
	return -1;
}