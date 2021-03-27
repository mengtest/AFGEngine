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
	uint32_t xDst, uint32_t yDst, uint32_t xSrc, uint32_t ySrc, uint32_t chunkSize);

struct VertexData8
{
	unsigned short x,y,s,t;
	unsigned short atlasId;
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
	int atlasId;

	void CalculateChunks(int chunkSize)
	{
		ImageData &image = *data; 
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


struct Atlas
{
	int id;
	ImageData image;
	int x, y;
	uint32_t chunkSize;

	Atlas(uint32_t width, uint32_t height, uint32_t bytesPerPixel, uint32_t _chunkSize, int _id = 0):
	id(_id), image(width, height, bytesPerPixel), x(0), y(0), chunkSize(_chunkSize)
	{}

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

	bool Fits(uint32_t chunkN) const
	{
		uint32_t available = (((image.height-y)/chunkSize)*image.width - x) / (chunkSize);
		return (available >= chunkN);
	}

	bool CopyToAtlas(ImageMeta &src)
	{
		std::list<ChunkMeta> &chunks = src.chunks;
		if(!Fits(chunks.size()))
		{
			std::cout << "Switching atlas channel...\n";
			return false;
		}

		src.atlasId = id;
		for(ChunkMeta &chunk : chunks)
		{
			if(!CopyChunk(image, *src.data, x, y, chunk.pos.x, chunk.pos.y, chunkSize))
			{
				std::cerr << __func__ << " failed.\n";
				return false;
			}
			chunk.tex.x = x;
			chunk.tex.y = y;
			if(!Advance())
			{
				std::cerr << "The atlas is full. "<<src.name<<" couldn't be copied properly.\n";
				return false;
			}
		}
		return true;
	}
};

#endif /* PACK_IMAGE_HPP_GUARD */
