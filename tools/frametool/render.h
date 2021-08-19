#ifndef RENDER_H_GUARD
#define RENDER_H_GUARD

#include <texture.h>
#include <shader.h>
#include <vao.h>
#include <ubo.h>
#include "types.h"

#include <vector>
#include <filesystem>
#include <unordered_map>
#include <glm/mat4x4.hpp>
#include <gfx_handler.h>

class Render
{
private:
	glm::mat4 projection;

	GfxHandler gfx;
	Ubo uniforms;
	Vao vGeometry;
	enum{
		LINES = 0,
		BOXES,
		GEO_SIZE
	};
	int geoParts[GEO_SIZE];
	std::vector<float> clientQuads;
	std::vector<uint16_t> clientElements;
	int quadsToDraw;
	size_t acumSize = 0;
	size_t acumQuads = 0;
	size_t acumElements = 0;
	float zOrder = 0;
	
	Shader sSimple;

	int lAlphaS;
	float colorRgba[4]; //Shader parameters?
	
	void SetModelView(glm::mat4&& view);

public:
	enum color_t 
	{
		gray = 0,
		green,
		red,
	};

	int spriteId;
	int x, offsetX;
	int y, offsetY;
	float scale;
	float scaleX, scaleY;
	float rotX, rotY, rotZ;
	int highLightN = -1;
	
	
	Render();

	void Draw();
	void UpdateProj(float w, float h);

	void GenerateHitboxVertices(const std::vector<int> &boxes, color_t pickedColor);
	void LoadHitboxVertices();

	void DontDraw();
	void SetImageColor(float *rgbaArr);

	void LoadGraphics(std::filesystem::path path);
	void LoadPalette(std::filesystem::path path);
};

#endif /* RENDER_H_GUARD */
