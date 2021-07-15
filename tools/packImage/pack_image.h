#ifndef PACK_IMAGE_HPP_GUARD
#define PACK_IMAGE_HPP_GUARD

#include <image.h>
#include <geometry.h>
#include <list>
#include <cstdint>
#include <iostream>

typedef Rect2d<int> Rect;
typedef Point2d<int> Point;

Rect GetImageBoundaries(const ImageData &im);
bool IsChunkTrans(const ImageData &im, uint32_t xStart, uint32_t yStart, uint32_t chunkSize);
bool IsPixelTrans(const ImageData &im, uint32_t x, uint32_t y);
bool CopyChunk(ImageData &dst, const ImageData &src, 
	int32_t xDst, int32_t yDst, int32_t xSrc, int32_t ySrc, int32_t chunkSize);
void CalcSizeInChunks(int chunks, int chunkSize, int &width, int &height, bool pow2, bool border);

struct VertexData4
{
	unsigned short x,y,s,t;
};

struct ChunkMeta
{
	Point pos;
	Point tex;
};

struct ImageMeta
{
	std::unique_ptr<ImageData> data;
	std::string name;
	std::list<ChunkMeta> chunks;

	void CalculateChunks(int chunkSize)
	{
		ImageData &image = *data; 
		Rect size = GetImageBoundaries(image);
		//Size of rect in chunks. At least 1.
		auto dif = (size.topRight - size.bottomLeft);
		int wChunks = dif.x/chunkSize + !!(dif.x%chunkSize);
		int hChunks = dif.y/chunkSize + !!(dif.y%chunkSize);

		Rect requiredSize(
			size.bottomLeft.x, size.bottomLeft.y,
			size.bottomLeft.x + wChunks*chunkSize, size.bottomLeft.y + hChunks*chunkSize
		);

		for(int y = requiredSize.bottomLeft.y; y < requiredSize.topRight.y; y += chunkSize)
		{
			for(int x = requiredSize.bottomLeft.x; x < requiredSize.topRight.x; x += chunkSize)
			{
				//TODO: Calculate hash instead.
				if(!IsChunkTrans(image, x, y, chunkSize))
				{
					ChunkMeta chunk{Point(x,y)};
					chunks.push_back(chunk);
				}
			}
		}
	}
};

void WriteVertexData(std::string filename, int nChunks, std::list<ImageMeta> &metas, int chunkSize, float width, float height);


struct Atlas
{
	ImageData image;
	int x, y;
	uint32_t chunkSize;
	int border;

	Atlas(uint32_t width, uint32_t height, uint32_t bytesPerPixel, uint32_t _chunkSize, bool _border = false):
	image(width, height, bytesPerPixel), x(0), y(0), border(_border), chunkSize(_chunkSize)
	{
		chunkSize+=border*2;
	}

	bool Advance()
	{
		if(x + chunkSize > image.width-chunkSize)
		{
			if(y + chunkSize > image.height-chunkSize)
			{
				//The atlas is full.
				return false;
			}
			y += chunkSize;
			x = 0;
		}
		else
			x += chunkSize;
		return true;
	}

	bool CopyToAtlas(ImageMeta &src)
	{
		std::list<ChunkMeta> &chunks = src.chunks;
		for(auto i = chunks.begin(); i!=chunks.end(); i++)
		{
			ChunkMeta &chunk = *i;
			if(!CopyChunk(image, *src.data, x, y, chunk.pos.x-border, chunk.pos.y-border, chunkSize))
			{
				std::cerr << __func__ << " failed.\tyn";
				return false;
			}
			chunk.tex.x = x+border;
			chunk.tex.y = y+border;
			if(!Advance())
			{
				std::cerr << "Atlas filled. Last image: "<<src.name<<".\n";
				return false;
			}
		}
		return true;
	}
};

#endif /* PACK_IMAGE_HPP_GUARD */
