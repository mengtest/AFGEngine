#ifndef HUD_H_INCLUDED
#define HUD_H_INCLUDED

//#include <vector>

#include "vao.h"
#include "texture.h"
#include <filesystem>

class Hud
{
	struct Coord{
		float x,y,s,t;
	};

	struct Bar
	{
		int pos;
		float w,h; //original size;
		float x,y;
		float tx,ty;
		bool flipped;
		int id;
	};
	
	float gScale;
	float width;
	float height;
	int startId;
	std::vector<Bar> barData;
	std::vector<Coord> coords;
	int staticCount = 0;
	Vao* vao;

public:
	Texture texture;
	Hud();
	Hud(std::filesystem::path file, Vao &vao);
	void Load(std::filesystem::path file, Vao &vao);
	void Draw();
	void ResizeBarId(int id, float horizPercentage);
};

#endif // HUD_H_INCLUDED
