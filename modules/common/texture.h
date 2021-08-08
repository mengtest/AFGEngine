#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <filesystem>
#include <string>
#include <image.h>

extern int pal;

struct texture_options{
	bool repeat = false;
	bool linearFilter = false;
	bool rectangle = false; //Rectangle textures only.
	bool linearAlpha = true; //Set to false to fix the image's alpha when loading it.
};

class Texture
{
public: //public only for ease of reading access, do not write anything.
	unsigned int id; //OpenGL texture id.
	bool isApplied;
	std::string filename;
	bool rectangle = false;
	bool is8bpp = false;

public:
	Texture();
	Texture(Texture&& texture);
	~Texture();
	Texture( const Texture& ) = delete; // non construction-copyable
	Texture& operator=( const Texture& ) = delete; // non copyable
	
	void LoadPng(std::filesystem::path imageFile, const texture_options options={}, std::filesystem::path paletteFile = {});
	void LoadLzs3(std::filesystem::path imageFile, const texture_options options={});
	void Unapply();

private:
	void Apply(const texture_options &options, int dataType, int w, int h, void* data, int compressedSize = 0);
};

#endif // IMAGE_H_INCLUDED
