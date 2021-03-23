#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <string>
#include <vector>

/*
Pretty much a bunch of miscellaneous functions
that will be removed or refactored when the time comes - Yeah, just about now.
*/

int DrawText(std::string text, std::vector<float> &arr, float x, float y); 

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
	

public:
	bool shrinkBorders; //Shrinks the UV box borders by half texel so bilinear artifacts don't occur.

public:
	std::vector<float> &coordPusher;
	CoordinateHelper(float w, float h, float a, float b, std::vector<float> &coordArray, e_coordtype choice);
	void GetUVCoords(float &u, float &v); //Maps texel coordinates to UV's [0,1] domain.
	void GetChunkCoords(float x, float y, float &u, float &v);
	void PushQuad(float x, float y, int start, bool hFlip = false); //Pushes quad texture coords of a xy chunk to a texture coords array in CCW order starting from bottom left.
	void ChangeDimensions(float w, float h, float a, float b, e_coordtype choice);
};

//Same as CoordinateHelper's, except this one pushes quad vertexes.
void PushQuad(float x, float y, float w, float h, std::vector<float> *vertArray, CoordinateHelper &ch, float xt, float yt, bool flip = false);
void ModifyQuad(float x, float y, float w, float h, std::vector<float> *vertArray, int index, bool relativeMovement = 0);

#endif // UTIL_H_INCLUDED
