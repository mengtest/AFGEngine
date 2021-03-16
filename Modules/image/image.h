#ifndef PNG_WRAPPER_H_GUARD
#define PNG_WRAPPER_H_GUARD
#include <cstdint>
#include <memory>

struct ImageData{
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	uint32_t flag;
	uint32_t bytesPerPixel; //Texel format for OpenGL usage.

	ImageData();
	~ImageData();

	enum
	{
		RGBA = 1,
		RGB
	};
};

/*
Loads image data from a PNG file using a palette if applicable.
Receives the path of the image and the palette file.
The palette path can be null.
*/
std::unique_ptr<ImageData> LoadImageFromPng(const char* image, const char* palette);

#endif /* PNG_WRAPPER_H_GUARD */
