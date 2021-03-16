#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED
#include <string>
#include <image.h>

extern int pal;

class Texture //Point to it, do not use directly.
{
public: //public only for ease of reading access, do not write anything.
	std::unique_ptr<ImageData> image;
	unsigned int id; //OpenGL texture id.
	bool isLoaded;
	bool isApplied;
	std::string filename;

public:
	Texture();
	Texture(Texture&& texture);
	~Texture();
	void Load(std::string imageFile, std::string paletteFile = std::string());
	void Apply(bool repeat = false, bool linearFilter = true);
	void Unapply();
	void Unload();

};

#endif // IMAGE_H_INCLUDED
