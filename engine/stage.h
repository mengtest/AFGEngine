#ifndef STAGE_H_GUARD
#define STAGE_H_GUARD

#include "camera.h"
#include <gfx_handler.h>
#include <vector>
#include <filesystem>
#include <functional>
#include <glm/mat4x4.hpp>

class Stage
{	
	struct element
	{
		float x,y;
		int drawId;
		int movementType;
		float speedX, speedY;
		float accelX, accelY;
		float centerX, centerY;
	};

	struct layer
	{
		int mode;
		float x,y;
		float scale;
		float xParallax;
		float yParallax;
		float xScroll;
		std::vector <element> elements;
	};

	std::vector<layer> layers;
	int height, width;
	float globalScale;

	GfxHandler *gfx;
	std::function <void(glm::mat4&)> setView;

	enum //lua constants
	{
		normal = 0,
		additive = 1,
		horizontal = 0x1,
		vertical = 0x2,
	};

public:
	int defId;

	Stage(GfxHandler &gfx, std::filesystem::path file, std::function <void(glm::mat4&)> setViewFun);
	std::pair<int,int> GetDimensions();

	//Must be used after GfxHandler's Begin()
	void Draw(glm::mat4 &view, centerScale camera);	
	
};

#endif /* STAGE_H_GUARD */
