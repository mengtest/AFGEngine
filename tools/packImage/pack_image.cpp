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
#include <args.hxx>

int main(int argc, char **argv)
{
	namespace fs = std::filesystem;

	args::ArgumentParser parser("Cuts and packs multiple images into a single PNG image.",
		"The images must be PNG. If 8bpp and 32bpp images are in the same folder, they "
		"will be packed separately into two files respectively.");
	args::Positional<std::string> srcFolder(parser, "FOLDER", "Folder containing the images.");
	args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});
    args::ValueFlag<std::string> outName(parser, "name", "Output filename. Defaults to [FOLDER].", {'o'});
	args::Flag pow2flag(parser, "pow2", "Make output dimensions a power of two.", {'p',"pow2"});
	args::Flag borderFlag(parser, "border", "Add a 1px border around each tile to avoid texture bleeding.", {'b',"border"});
	args::ValueFlag<int> tileSize(parser, "size",
		"The size in pixel of a square tile. Defaults to 16.", {'s', "size"}, 16);
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
	bool border = borderFlag;

	fs::path folderIn = srcFolder.Get();
	std::string filenameOut = folderIn.filename().string();
	if(outName)
		filenameOut = outName.Get();

	int chunkSize = tileSize.Get();
	if(chunkSize < 4 || chunkSize > 256)
	{
		std::cout << "Invalid tile size. The size of a tile must be between 4 and 256.";
		return 0;
	}
	if(border)
		chunkSize -= 2;

	//Iterate through all images in the folder and calculate how to chop them up.
	int nChunks8 = 0; int nChunks32 = 0;
	std::list<ImageMeta> images8bpp, images32bpp;
	try {
		for(const fs::directory_entry &file : fs::directory_iterator(folderIn))
		{
			ImageMeta im {
				std::make_unique<ImageData>(file.path().generic_string().c_str()),
				file.path().filename().string()
			};
			if(im.data->data)
			{
				
				if(im.data->bytesPerPixel == 1)
				{
					im.CalculateChunks(chunkSize);
					nChunks8 += im.chunks.size();
					images8bpp.push_back(std::move(im));
				}
				else 
				{
					im.CalculateChunks(chunkSize);
					nChunks32 += im.chunks.size();
					images32bpp.push_back(std::move(im));
				}
			}
			
		}
	}
	catch(fs::filesystem_error const& ex)
	{
		std::cout << "Filesystem error: "<< ex.what() << '\n'<<"Make sure the input folder actually exists and is accessible.";
		return 1;
	}

	//Calculate size of atlas.
	int channelChunks8 = nChunks8/4 + 1;
	int width8, height8, width32, height32;
	CalcSizeInChunks(channelChunks8, width8, height8);
	CalcSizeInChunks(nChunks32, width32, height32);

	int chunkSizeWBorder = chunkSize;
	if(border)
		chunkSizeWBorder += 2;

	width8*=chunkSizeWBorder;
	height8*=chunkSizeWBorder;
	width32*=chunkSizeWBorder;
	height32*=chunkSizeWBorder;

	//Write image data.
	if(nChunks8)
	{
		std::cout << "Read " << images8bpp.size() << " 8bpp images.\n" << nChunks8 << " chunks required ("<<channelChunks8<<" per channel)\n";
		std::cout << "Output image: "<<width8<<"x"<<height8<<"\n\n";

		std::vector<std::unique_ptr<Atlas>> atlases;
		atlases.reserve(4);
		for(int i = 0; i < 4; i++)
		{
			atlases.push_back(std::make_unique<Atlas>(width8, height8, 1, chunkSize, border, i));
			memset(atlases[i]->image.data, 0, atlases[i]->image.GetMemSize());
		}

		//Fill each channel of the atlas with image data.
		auto atlasI = atlases.begin();
		for(auto &im: images8bpp)
		{
			auto result = (*atlasI)->CopyToAtlas(im, im.chunks.begin());
			if(!result.first)
			{
				atlasI++;
				if(atlasI == atlases.end())
				{
					std::cout << "Not enough atlases.\n";
					break;
				}
				//Copy the image that couldn't be copied.
				(*atlasI)->CopyToAtlas(im, result.second);
			}
		}
		
		//Group the atlases into a single image.
		ImageData composite(width8, height8, 4);
		for(int y = 0; y < height8; y++)
		{
			for(int x = 0; x < width8; x++)
			{
				for(int ch = 0; ch < 4; ch++)
				{
					ImageData &srcIm = atlases[ch]->image;
					composite.data[y*composite.width*composite.bytesPerPixel + x*composite.bytesPerPixel + ch] =
						srcIm.data[y*srcIm.width*srcIm.bytesPerPixel + x*srcIm.bytesPerPixel];
				}
			}
		}

		std::cout << "Writing file... ";
		std::string pngO = filenameOut+"8.png";
		composite.WriteAsPng(pngO.c_str());
		WriteVertexData(filenameOut+".vt8", nChunks8, images8bpp, chunkSize, width8, height8);
		std::cout << "Done\n\n";
	}
	if(nChunks32)
	{
		std::cout << "Read " << images32bpp.size() << " 32bpp images.\n" << nChunks32 << " chunks required\n";
		std::cout << "Output image: "<<width32<<"x"<<height32<<"\n\n";

		Atlas atlas(width32, height32, 4, chunkSize, border, 0);
		memset(atlas.image.data, 0, atlas.image.GetMemSize());

		//Fill the atlas with image data.
		for(auto &im: images32bpp)
		{
			atlas.CopyToAtlas(im, im.chunks.begin());
		}

		std::cout << "Writing file... ";
		std::string pngO = filenameOut+"32.png";
		atlas.image.WriteAsPng(pngO.c_str());
		WriteVertexData(filenameOut+".vt9", nChunks32, images32bpp, chunkSize, width32, height32);

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
uint32_t xDst, uint32_t yDst, uint32_t xSrc, uint32_t ySrc, uint32_t chunkSize)
{
	if(dst.bytesPerPixel != src.bytesPerPixel)
	{
		std::cerr << __func__ << ": Number of channels differ.";
		return false;
	}
//	xSrc+chunkSize > src.width
	if(	xDst+chunkSize > dst.width || yDst+chunkSize > dst.height
	||	xDst < 0 || yDst < 0 || xSrc < 0 || ySrc < 0 
	)
	{
		std::cerr << __func__ << ": Trying to access chunk data out of boundaries.";
		return false;
	}

	int chunkXSize = xSrc+chunkSize - src.width;
	if(chunkXSize < 0)
		chunkXSize = chunkSize;
	
	int chunkYSize = ySrc+chunkSize - src.height;
	if(chunkYSize < 0)
		chunkYSize = chunkSize;

	int mul = dst.bytesPerPixel;
	for(unsigned int row = 0; row < chunkYSize; row++)
	{
		//Y + X + row, chunkSize
		memcpy(
			dst.data + (yDst+row)*dst.width*mul + xDst*mul,
			src.data + (ySrc+row)*src.width*mul + xSrc*mul,
			chunkXSize*mul
		);
	}

	return true; 
}

void WriteVertexData(std::string filename, int nChunks, std::list<ImageMeta> &metas, int chunkSize, float width, float height)
{
	constexpr int tX[] = {0,1,1, 1,0,0};
	constexpr int tY[] = {0,0,1, 1,1,0};
	int nSprites = metas.size();

	std::unordered_map<std::string, uint16_t> nameMap;
	nameMap.reserve(nSprites);

	auto chunksPerSprite = new uint16_t[nSprites];
	auto data = new VertexData8[nChunks*6];
	
	int dataI = 0;
	int spriteI = 0;
	for(auto &meta: metas)
	{
		nameMap.insert({meta.name, spriteI});
		chunksPerSprite[spriteI] = meta.chunks.size();
		for(auto &chunk: meta.chunks)
		{
			for(int i = 0; i < 6; i++)
			{
				data[dataI+i].x = chunk.pos.x + chunkSize*tX[i];
				data[dataI+i].y = chunk.pos.y + chunkSize*tY[i];

				data[dataI+i].s = (float)(chunk.tex.x + chunkSize*tX[i])*UINT16_MAX/width;
				data[dataI+i].t = (float)(chunk.tex.y + chunkSize*tY[i])*UINT16_MAX/height;

				data[dataI+i].atlasId = chunk.atlasId;
			}
			dataI += 6;
		}
		spriteI++;
	}

	std::ofstream vertexFile(filename, std::ios_base::binary);
	vertexFile.write((char*)&nSprites, sizeof(int));
	vertexFile.write((char*)chunksPerSprite, sizeof(uint16_t)*nSprites);
	vertexFile.write((char*)&nChunks, sizeof(int));
	vertexFile.write((char*)data, nChunks*6*sizeof(VertexData8));

	for(auto& p: nameMap)
	{
		uint8_t size = p.first.size();
		vertexFile.write((char*)&size, 1);
		vertexFile.write(p.first.c_str(), size);
		vertexFile.write((char*)&p.second, sizeof(uint16_t));
	}

	vertexFile.close();
	delete[] data;
	delete[] chunksPerSprite;
}

void CalcSizeInChunks(int chunks, int &width, int &height, bool pow2)
{
	width = sqrt(chunks);
	while(width*width < chunks)
		width++;
	width--;
	height = chunks/width + !!(chunks%width);
}