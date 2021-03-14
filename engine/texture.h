#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED
#include <string>

#include <png.h>

extern int pal;

struct TextureData{ //I probably should integrate this directly into the Texture class.
  png_byte *data = 0;
  int width = 0;
  int height = 0;
  int flag = 0;
  int format = 0; //OpenGL texel format.
};

class Texture //Point to it, do not use directly.
{
public: //public only for ease of reading access, do not write anything.
    TextureData data;
    unsigned int id; //OpenGL texture id.
    bool isLoaded;
    bool isApplied;
    std::string filename;

public:
	static TextureData LoadTexture(const char* filename);

    Texture();
    ~Texture();
    void Load(std::string name);
    void Apply(bool repeat = false, bool linearFilter = true);
    void Unapply();
    void Unload();
};

#endif // IMAGE_H_INCLUDED
