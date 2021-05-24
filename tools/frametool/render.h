#ifndef RENDER_H_GUARD
#define RENDER_H_GUARD

#include <texture.h>
#include <shader.h>
#include <vao.h>
#include <ubo.h>
#include "hitbox.h"

#include <vector>
#include <unordered_map>
#include <glm/mat4x4.hpp>

class Render
{
private:
	glm::mat4 projection;

	Ubo uniforms;
	Vao vSprite;
	Vao vGeometry;
	enum{
		LINES = 0,
		BOXES,
		GEO_SIZE
	};
	int geoParts[GEO_SIZE];
	std::vector<float> clientQuads;
	int quadsToDraw;

	int lAlphaS;
	Shader sSimple;
	Shader sTextured;
	Texture texture;
	float colorRgba[4];

	int curImageId;
	
	void SetModelView(glm::mat4&& view);

public:
	int x, offsetX;
	int y, offsetY;
	float scale;
	float scaleX, scaleY;
	float rotX, rotY, rotZ;
	int highLightN = -1;
	
	Render();
	std::unordered_map<std::string, uint16_t> LoadGraphics(const char* pngFile, const char* vtxFile);
	void LoadPalette(const char* file);

	void Draw();
	void UpdateProj(float w, float h);

	void GenerateHitboxVertices(const BoxList &hitboxes);

	void DontDraw();
	void SetImageColor(float *rgbaArr);
};

#endif /* RENDER_H_GUARD */
