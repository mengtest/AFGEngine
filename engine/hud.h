#ifndef HUD_H_INCLUDED
#define HUD_H_INCLUDED
#include <glad/glad.h>
#include <vector>

enum
{
	B_P1LifeRed,
	B_P2LifeRed,
	B_P1Life,
	B_P2Life
};

enum e_side
{
	LEFT,
	RIGHT
};

class Bar //as in lifebar.
{
private:

	float width;
	float height;
	float offsetX; //from the bottom of the screen
	float offsetY; //from the middle of the screen. Convenient since bars are symmetrical.
	float texChunkX; //Indexes of XY chunks
	float texChunkY;
	e_side side;
	std::vector<float> *vertexArray;
	std::vector<float> *coordArray;
	int index; //Where is the bar in the vertex array.

public:
	Bar(float width, float height, float offsetX, float offsetY, std::vector<float> *vertexArray, std::vector<float> *coordArray, float texAtlasIndexX, float texAtlasIndexY, e_side side);
	void Resize(float amountX, float amountY); //Amounts are the ratio of the maximum size, not the actual size.

};

std::vector<Bar> InitBars();
void DrawHud(GLuint textureId);

#endif // HUD_H_INCLUDED
