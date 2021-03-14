#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <string>
#include <vector>

/*
Pretty much a bunch of miscellaneous functions
that will be removed or refactored when the time comes
*/

void PushQuad(float x, float y, float w, float h, std::vector<float> *vertArray); //Same as CoordinateHelper's, except this one pushes quad vertexes.
void ModifyQuad(float x, float y, float w, float h, std::vector<float> *vertArray, int index, bool relativeMovement = 0);

void DrawTextA(std::string text, unsigned int texId, float x, float y, float z); //Draws text on the screen for debugging purposes.

std::string ReadFile(const char *filePath); //Reads a plain text file and turns it into a single std::string.

enum e_coordtype
{
    NONE,
    CHUNK_SIZE,
    ROWCOL
};

class CoordinateHelper //Helps to access/modify texture coordinates through chunk sizes in texels.
{
private:
    float width;
    float height;
	float chunkWidth;
    float chunkHeight;
    std::vector<float> *coordPusher;

public:
	bool shrinkBorders; //Shrinks the UV box borders by half texel so bilinear artifacts don't occur.

public:
    CoordinateHelper(float w, float h, float a, float b, std::vector<float> *coordArray, e_coordtype choice);
    void GetUVCoords(float &u, float &v); //Maps texel coordinates to UV's [0,1] domain.
    void GetChunkCoords(float x, float y, float &u, float &v);
    void PushQuad(float x, float y, bool hFlip = false); //Pushes quad texture coords of a xy chunk to a texture coords array in CCW order starting from bottom left.
    void ChangeDimensions(float w, float h, float a, float b, e_coordtype choice);
};

#endif // UTIL_H_INCLUDED
