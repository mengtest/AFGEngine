#ifndef SPRITE_H_GUARD
#define SPRITE_H_GUARD

#include "vao.h"
#include "texture.h"
#include <vector>


class Sprite{
private:
	Vao indexed, truecolor;
	std::vector<Texture> textures;

public:
	Sprite(const char* defFile);
	
};

#endif /* SPRITE_H_GUARD */
