#include "point.h"
#include <image.h>
#include <geometry.h>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string>

namespace fs = std::filesystem;
typedef Rect2d<int> Rect;

struct ImageMeta
{
	std::unique_ptr<ImageData> data;
	std::string name;
};

/*
struct Chunk
{
	Point2d<unsigned int> pos;
};

*/

struct Atlas
{
	ImageData image;
	int x, y;

	Atlas(uint32_t width, uint32_t height, uint32_t bytesPerPixel):
	image(width, height, bytesPerPixel), x(0), y(0)
	{}

	bool Advance(uint32_t chunkSize)
	{
		if(x + chunkSize >= image.width)
		{
			if(y + chunkSize >= image.height)
			{
				//The atlas is full.
				return false;
			}
			y += chunkSize;
			x =
		}
		else
			x += chunkSize;
		return true;
	}
};


bool IsPixelTrans(const ImageData &im, uint32_t x, uint32_t y)
{
	uint32_t mul = im.bytesPerPixel;
	if(mul != 1)
	{
		std::cerr << __func__ << ": Unhandled number of channels.\n";
		abort();
	}
	if(mul == 1)
	{
		return im.data[y*im.width*mul + x*mul] == 0;
	}
	return false;
}

bool IsChunkTrans(const ImageData &im, uint32_t xStart, uint32_t yStart, uint32_t chunkSize)
{
	for(int y = yStart; y < chunkSize; y++){
		for(int x = xStart; x < chunkSize; x++){
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

	std::cout << left <<" "<< bottom <<" "<< right <<" "<< top << "\n";
	return Rect(left, bottom, right, top);
}

Rect CalculateChunkRect(const ImageData &image, int chunkSize)
{
	Rect size = GetImageBoundaries(image);
	//Size of rect in chunks. At least 1.
	int wChunks = 1 + (size.topRight - size.bottomLeft).x/chunkSize;
	int hChunks = 1 + (size.topRight - size.bottomLeft).y/chunkSize;

	Rect requiredSize(
		size.bottomLeft.x, size.bottomLeft.y,
		size.bottomLeft.x + wChunks*chunkSize, size.bottomLeft.y + hChunks*chunkSize
	);
	//Boundary check and adjustment
	int xTranslate = image.width - requiredSize.topRight.x;
	int yTranslate = image.height - requiredSize.topRight.y;
	if(xTranslate < 0)
	{
		requiredSize = requiredSize.Translate(xTranslate, 0);
	}
	if(yTranslate < 0)
	{
		requiredSize = requiredSize.Translate(0, yTranslate);
	}
	//The image is too small for all the chunks to fit.
	//If it becomes an issue I could support for rectangular chunks.
	if(requiredSize.bottomLeft.x < 0 || requiredSize.bottomLeft.y < 0)
	{
		std::cerr << __func__ << ": Chunks do not fit in image.\n" <<
			"Image (w,h) " << image.width << " " << image.height <<
			". Chunk size " << chunkSize << " (w,h) " << wChunks << " " << hChunks << "\n";
		abort();
	}

	int numberChunks = 0;
	for(int y = requiredSize.bottomLeft.y; y < requiredSize.topRight.y; y += chunkSize)
	{
		for(int x = requiredSize.bottomLeft.x; x < requiredSize.topRight.x; x += chunkSize)
		{
			if(!IsChunkTrans(image, x, y, chunkSize))
			{
				numberChunks++;
			}
		}
	}
}

bool CopyChunk(ImageData &dst, const ImageData &src, 
uint32_t xDst, uint32_t yDst, uint32_t xSrc, uint32_t ySrc, uint32_t chunkSize)
{
	if(dst.bytesPerPixel != src.bytesPerPixel)
	{
		std::cerr << __func__ << ": Number of channels differ.";
		return false;
	}

	if(	xDst+chunkSize > dst.width || yDst+chunkSize > dst.height
	||	xSrc+chunkSize > src.width || ySrc+chunkSize > src.height
	||	xDst < 0 || yDst < 0 || xSrc < 0 || ySrc < 0 
	)
	{
		std::cerr << __func__ << ": Trying to access chunk data out of boundaries.";
		return false;
	}

	int mul = dst.bytesPerPixel;
	for(unsigned int row = 0; row < chunkSize; row++)
	{
		//Y + X + row, chunkSize
		memcpy(
			dst.data + (yDst+row)*dst.width*mul + xDst*mul,
			src.data + (ySrc+row)*src.width*mul + xSrc*mul,
			chunkSize
		);
	}

	return true; 
}

bool CopyToAtlas(Atlas &dst, const ImageMeta &src, uint32_t chunkSize)
{
	ImageData &srcIm = *src.data; 
	Rect chunkRect = CalculateChunkRect(srcIm, chunkSize);

	
}

int main()
{
	std::string folderIn = "images/char/";
	std::string folderOut = "images/test/";

	std::list<ImageMeta> images8bpp;
	int tempi = 1;
	for(const fs::directory_entry &file : fs::directory_iterator(folderIn))
	{
		if(tempi > 8)
			break;
		ImageMeta im {
			std::make_unique<ImageData>(file.path().generic_string().c_str()),
			file.path().filename().string()
		};
		if(im.data->data)
		{
			if(im.data->bytesPerPixel == 1)
			{
				
				//image->WriteAsPng((folderOut + filename).c_str());
				images8bpp.push_back(std::move(im));
				tempi++;
			}
			else
			{
				//std::cout << filename << " is not 8bpp. Ignoring...\n";
			}
		}
		
	}

	GetImageBoundaries(*images8bpp.front().data);

	Atlas atlas(2048, 2048, 1);
	memset(atlas.image.data, 0, atlas.image.GetMemSize());
	
	int i = 0, chunksize = 256;
	for(const auto &im: images8bpp)
	{
		if(!CopyToAtlas(atlas, im, chunksize))
			break;
	}
	
	std::cout << images8bpp.size() << " images.\n";
	atlas.image.WriteAsPng("test.png");
	return 0;
}
