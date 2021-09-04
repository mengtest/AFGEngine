#include "pack_image.h"
#include <image.h>
#include <geometry.h>

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <iomanip>
#include <set>

#include <args.hxx>

//Pairs filename with the desired ID.
int xOffset, yOffset;
bool centerH = false;
bool centerV = false;
std::unordered_map<std::string, int> nameMap;

bool ignoreCaseCompare(std::string &&str1, std::string &&str2)
{
	return (
		(str1.size() == str2.size()) &&
		std::equal(
			str1.begin(), str1.end(), str2.begin(), [](char &c1, char &c2) {
				return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
			}
		)
	);
}

std::istream& operator>>(std::istream& is, std::tuple<int, int>& ints)
{
    is >> std::get<0>(ints);
    is.get();
    is >> std::get<1>(ints);
    return is;
}

int main(int argc, char **argv)
{
	namespace fs = std::filesystem;

	args::ArgumentParser parser("Cuts and packs multiple images into a single PNG image.",
		"The images must be PNG. If 8bpp and 32bpp images are in the same folder, they "
		"will be packed separately into two files respectively.");
	args::Positional<std::string> srcFolder(parser, "FOLDER", "Folder containing the images.");
	args::Positional<std::tuple<int,int>> pivot(parser, "X,Y", "Pivot: X and Y amount to offset the ouput vertex coordinates");
	args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});
	args::Flag namesOnly(parser, "namesOnly", "Only output names.txt", {'n',"names"});
	args::ValueFlag<int> startingNameIndex(parser, "index", "Starting index in names.txt.", {'i', "index"}, 16);
	args::ValueFlag<std::string> outName(parser, "name", "Output filename. Defaults to [FOLDER].", {'o'});
	args::Flag pow2flag(parser, "pow2", "Make output dimensions a power of two.", {'p',"pow2"});
	args::Flag borderFlag(parser, "border", "Add a 1px border around each tile to avoid texture bleeding.", {'b',"border"});
	args::ValueFlag<int> tileSize(parser, "size",
		"The size in pixel of a square tile. Defaults to 16.", {'s', "size"}, 16);
	args::ValueFlag<std::string> center(parser, "'h' or 'v'", "Center the pivot horizontally and/or vertically. Overrides pivot offset.", {'c'});
	args::Group pickGroup(parser, "Optionally pick only one type:", args::Group::Validators::AtMostOne);
	args::Flag only8(pickGroup, "only8", "Only process 8bpp images", {"8bpp"});
	args::Flag only32(pickGroup, "only32", "Only process 32bpp images", {"32bpp"});

	try
	{
		parser.ParseCLI(argc, argv);
		if(!srcFolder)
		{
			std::cout << "No folder chosen. Use -h for help.";
			return 0;
		}
	}
	catch (const args::Help&)
	{
		std::cout << parser;
		return 0;
	}
	catch (const args::ParseError& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	catch (args::ValidationError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	bool border = borderFlag;
	xOffset = std::get<0>(args::get(pivot));
	yOffset = std::get<1>(args::get(pivot));

	if(center.Get().find('h') != std::string::npos)
		centerH = true;
	if(center.Get().find('v') != std::string::npos)
		centerV = true;

	fs::path folderIn = srcFolder.Get();
	std::string filenameOut = (folderIn.parent_path()/folderIn.stem()).string();
	if(outName)
		filenameOut = outName;
	
	int chunkSize = tileSize.Get();
	if(chunkSize < 4 || chunkSize > 256)
	{
		std::cout << "Invalid tile size. The size of a tile must be between 4 and 256.";
		return 0;
	}
	if(border)
		chunkSize -= 2;
	
	std::set<int> usedIds;
	std::ifstream nameFile(folderIn.string() + "/names.txt");
	if(nameFile.good())
	{
		while(!nameFile.eof())
		{
			std::string out;
			int id;
			nameFile >> quoted(out) >> id;
			if(out.empty())
				continue;
			if(nameFile.fail() || nameFile.bad())
			{
				std::cerr << "Names.txt has wrong format. Stopping...\n";
				break;
			}
			if(usedIds.count(id) > 0)
			{
				std::cerr << "Names.txt has dupe IDs. Stopping...\n";
				break;
			}
			usedIds.insert(id);
			nameMap.insert({out, id});
		}
	}
	nameFile.close();

	//Iterate through all images in the folder and calculate how to chop them up.
	int nChunks8 = 0; int nChunks32 = 0;
	std::list<ImageMeta> images8bpp, images32bpp;
	try {
		int counter = startingNameIndex;
		for(const fs::directory_entry &file : fs::directory_iterator(folderIn))
		{
			if(ignoreCaseCompare(file.path().extension().string(), ".png"))
			{
				if(namesOnly)
				{
					std::string&& fn = file.path().filename().string();
					if(nameMap.count(fn) == 0)
					{
						while(usedIds.count(counter) > 0)
							counter++;
						nameMap.insert({fn, counter});
						counter++;
					}
					else
						counter = nameMap[fn]+1;
				}
				else
				{
					ImageMeta im {
						std::make_unique<ImageData>(file.path().generic_string().c_str(), nullptr, true),
						file.path().filename().string()
					};
					if(im.data->data)
					{
						if(im.data->bytesPerPixel == 1)
						{
							if(!only32)
							{
								if(nameMap.count(im.name) == 0)
								{
									while(usedIds.count(counter) > 0)
										counter++;
									nameMap.insert({im.name, counter});
									counter++;
								}
								else
									counter = nameMap[im.name]+1;

								im.CalculateChunks(chunkSize);
								nChunks8 += im.chunks.size();
								images8bpp.push_back(std::move(im));
								
							}
						}
						else if(!only8)
						{
							if(nameMap.count(im.name) == 0)
							{
								while(usedIds.count(counter) > 0)
									counter++;
								nameMap.insert({im.name, counter});
								counter++;
							}
							else
								counter = nameMap[im.name]+1;

							im.CalculateChunks(chunkSize);
							nChunks32 += im.chunks.size();
							images32bpp.push_back(std::move(im));
						}
					}
				}
			}
		}
	}
	catch(fs::filesystem_error const& ex)
	{
		std::cout << "Filesystem error: "<< ex.what() << '\n'<<"Make sure the input folder actually exists and is accessible.\n";
		return 1;
	}

	std::ofstream outNames(folderIn.string() + "/names.txt");
	for(const auto &name : nameMap)
	{
		outNames << quoted(name.first) << " "<<name.second<<"\n";
	}

	if(namesOnly)
		return 0;

	//Calculate size of atlas and write image data.
	if(nChunks8)
	{
		int width8, height8;
		CalcSizeInChunks(nChunks8, chunkSize, width8, height8, pow2flag, border);

		std::cout << "Read " << images8bpp.size() << " 8bpp images.\n" << nChunks8 << " chunks required.\n";
		std::cout << "Output image: "<<width8<<"x"<<height8<<"\n\n";

		Atlas atlas(width8, height8, 1, chunkSize, border);
		memset(atlas.image.data, 0, atlas.image.GetMemSize());

		//Fill each channel of the atlas with image data.
		for(auto &im: images8bpp)
		{
			atlas.CopyToAtlas(im);
		}
		
		std::cout << "Writing file... ";
		std::string pngO = filenameOut+"8.png";
		atlas.image.WriteAsPng(pngO.c_str());
		WriteVertexData(filenameOut+".vt1", nChunks8, images8bpp, chunkSize, width8, height8);
		std::cout << "Done\n\n";
	}
	if(nChunks32)
	{
		int width32, height32;
		CalcSizeInChunks(nChunks32, chunkSize, width32, height32, pow2flag, border);

		std::cout << "Read " << images32bpp.size() << " 32bpp images.\n" << nChunks32 << " chunks required.\n";
		std::cout << "Output image: "<<width32<<"x"<<height32<<"\n\n";

		Atlas atlas(width32, height32, 4, chunkSize, border);
		memset(atlas.image.data, 0, atlas.image.GetMemSize());

		//Fill the atlas with image data.
		for(auto &im: images32bpp)
			atlas.CopyToAtlas(im);

		std::cout << "Writing file... ";
		std::string pngO = filenameOut+"32.png";
		atlas.image.WriteAsPng(pngO.c_str());
		WriteVertexData(filenameOut+".vt4", nChunks32, images32bpp, chunkSize, width32, height32);

		std::cout << "Done\n\n";
	}
	return 0;
}

bool IsPixelTrans(const ImageData &im, uint32_t x, uint32_t y)
{
	uint32_t mul = im.bytesPerPixel;
	if(mul == 4)
	{
		return im.data[y*im.width*mul + x*mul + 3] == 0;
	}
	if(mul == 1)
	{
		return im.data[y*im.width*mul + x*mul] == 0;
	}
	return false;
}

bool IsChunkTrans(const ImageData &im, uint32_t xStart, uint32_t yStart, uint32_t chunkSize)
{
	uint32_t xEnd = chunkSize+xStart;
	uint32_t yEnd = chunkSize+yStart;
	if(yEnd > im.height)
		yEnd = im.height;
	if(xEnd > im.width)
		xEnd = im.width;
	for(unsigned int y = yStart; y < yEnd; y++){
		for(unsigned int x = xStart; x < xEnd; x++){
			if(!IsPixelTrans(im,x,y)){
				return false;
			}
		}
	}
	return true;
}

Rect GetImageBoundaries(const ImageData &im)
{
	int bottom = -1, top = 0, left = 0, right = 0;

	//Find bottom.
	for(int y = 0; y < (int)im.height; y++){
		for(int x = 0; x < (int)im.width; x++){
			if(!IsPixelTrans(im,x,y)){
				bottom = y;
				goto bottom_end;
			}
		}
	}
	if(bottom == -1)
	{
		std::cout << __func__ << ": The image is empty.";
		return Rect();
	}
	bottom_end:

	//Find top.
	for(int y = im.height-1; y >= bottom; y--){
		for(int x = 0; x < (int)im.width; x++){
			if(!IsPixelTrans(im,x,y)){
				top = y+1;
				goto top_end;
			}
		}
	}
	top_end:

	//Find left.
	for(int x = 0; x < (int)im.width; x++){
		for(int y = bottom; y < top; y++){
			if(!IsPixelTrans(im,x,y)){
				left = x;
				goto left_end;
			}
		}
	}
	left_end:

	//Find right;
	for(int x = im.width-1; x >= left; x--){
		for(int y = bottom; y < top; y++){
			if(!IsPixelTrans(im,x,y)){
				right = x+1;
				goto right_end;
			}
		}
	}
	right_end:

	return Rect(left, bottom, right, top);
}

bool CopyChunk(ImageData &dst, const ImageData &src, 
int32_t xDst, int32_t yDst, int32_t xSrc, int32_t ySrc, int32_t chunkSize)
{
	if(dst.bytesPerPixel != src.bytesPerPixel)
	{
		std::cerr << __func__ << ": Number of channels differ.";
		return false;
	}

	if(	xDst+chunkSize > dst.width || yDst+chunkSize > dst.height
	||	xDst < 0 || yDst < 0
	)
	{
		std::cerr << __func__ << ": Trying to access chunk data out of boundaries.";
		abort();
	}

	int rightFill = 0;
	int chunkXSize = src.width - xSrc;
	if(chunkXSize > chunkSize)
	{
		chunkXSize = chunkSize;
	}
	else
		rightFill = 1;
	
	int chunkYSize = src.height - ySrc;
	if(chunkYSize > chunkSize)
		chunkYSize = chunkSize;

	if(ySrc < 0)
	{
		chunkYSize += ySrc;
		yDst -= ySrc;
		ySrc = 0;
		
	}
	int leftFill = 0;
	if(xSrc < 0)
	{
		leftFill = xSrc;
		chunkXSize += xSrc;
		xDst -= xSrc;
		xSrc = 0;
	}

	int mul = dst.bytesPerPixel;
	for(unsigned int row = 0; row < chunkYSize; row++)
	{
		//Y + X + row, chunkSize
		memcpy(
			dst.data + (yDst+row)*dst.width*mul + xDst*mul,
			src.data + (ySrc+row)*src.width*mul + xSrc*mul,
			chunkXSize*mul
		);
		if(leftFill)
			memcpy(
			dst.data + (yDst+row)*dst.width*mul + (xDst+leftFill)*mul,
			src.data + (ySrc+row)*src.width*mul + xSrc*mul,
			mul);
		if(rightFill)
			memcpy(
			dst.data + (yDst+row)*dst.width*mul + (xDst+chunkXSize)*mul, 
			src.data + (ySrc+row)*src.width*mul + (xSrc+chunkXSize-1)*mul,
			mul);
	}

	return true; 
}

void WriteVertexData(std::string filename, int nChunks, std::list<ImageMeta> &metas, int chunkSize, float width, float height)
{
	constexpr int tX[] = {0,1,1, 1,0,0};
	constexpr int tY[] = {0,0,1, 1,1,0};
	int nSprites = metas.size();

	auto chunksPerSprite = new uint16_t[nSprites];
	VertexData4* data = new VertexData4[nChunks*6];
	int chunkCheck = 0;
	
	int dataI = 0;
	int spriteI = 0;
	for(auto &meta: metas)
	{
/* 		if(nameMap.at(meta.name) == 500)
		{
			std::cout<<"bug";
		} */

		int x = xOffset;
		int y = yOffset;
		if(centerH)
			x += meta.data->width>>1;
		if(centerV)
			y += meta.data->height>>1;
		//nameMap.insert({meta.name, spriteI});
		chunksPerSprite[spriteI] = meta.chunks.size();
		//memset(data+dataI, 0, sizeof(VertexData4)*6*chunksPerSprite[spriteI]);
		for(auto &chunk: meta.chunks)
		{
			for(int i = 0; i < 6; i++)
			{
				data[dataI+i].x = -x + chunk.pos.x + chunkSize*tX[i];
				data[dataI+i].y = -y + chunk.pos.y + chunkSize*tY[i];
				data[dataI+i].s = chunk.tex.x + chunkSize*tX[i];
				data[dataI+i].t = chunk.tex.y + chunkSize*tY[i];

				/* data[dataI+i].s = (float)(chunk.tex.x + chunkSize*tX[i])*UINT16_MAX/width;
				data[dataI+i].t = (float)(chunk.tex.y + chunkSize*tY[i])*UINT16_MAX/height; */
			}
			dataI += 6;
			++chunkCheck;
		}
		spriteI++;
	}

	assert(chunkCheck==nChunks);
	assert(spriteI==nSprites);

	std::ofstream vertexFile(filename, std::ios_base::binary);
	vertexFile.write((char*)&nSprites, sizeof(int));
	vertexFile.write((char*)chunksPerSprite, sizeof(uint16_t)*nSprites);
	vertexFile.write((char*)&nChunks, sizeof(int));
	vertexFile.write((char*)data, nChunks*6*sizeof(VertexData4));

	//Write (true index) -> id in name map.
	for(auto &meta: metas)
	{
		uint16_t virtualId = nameMap.at(meta.name);
		vertexFile.write((char*)&virtualId, sizeof(virtualId));
	}

	vertexFile.close();
	delete[] data;
	delete[] chunksPerSprite;
}

unsigned LogMsb(unsigned int number)
{
	unsigned r = 0;
	while (number >>= 1) {
		r++;
	}
	return r;
}

void CalcSizeInChunks(int chunks, int chunkSize, int &width, int &height, bool pow2, bool border)
{
	if(border)
		chunkSize += 2;

	if(pow2)
	{
		height = width = 1;
		while(width*height < chunks)
		{
			width <<= 1;
			if(width*height < chunks)
				height = width;
		}
	}
	else
	{
		width = sqrt(chunks);
		while(width*width < chunks)
			width++;

		height = chunks/width + !!(chunks%width);
	}

	width*=chunkSize;
	height*=chunkSize;

	if(pow2)
	{
		width = 1 << (LogMsb(width-1)+1);
		height = 1 << (LogMsb(height-1)+1);
	}
}
