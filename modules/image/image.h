#ifndef PNG_WRAPPER_H_GUARD
#define PNG_WRAPPER_H_GUARD
#include <cstdint>
#include <cstdio>
#include <cstddef>

struct ImageData{
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	uint32_t flag;
	uint32_t bytesPerPixel;

	ImageData();
	ImageData(uint32_t width, uint32_t height, uint32_t bytesPerPixel);
	ImageData(const char* image, const char* palette = nullptr, bool linearAlpha = true);
	~ImageData();

	/*
	Loads image data from a PNG file using a palette if applicable.
	Receives the path of the image and the palette file.
	The palette path can be null.
	*/
	bool LoadFromPng(const char* image, const char* palette = nullptr, bool linearAlpha = true);
	bool WriteAsPng(const char* path, const char* palette = nullptr) const;
	std::size_t GetMemSize() const;
	void FreeData();

	enum
	{
		RGBA = 1,
		RGB,
	};
};

#endif /* PNG_WRAPPER_H_GUARD */
