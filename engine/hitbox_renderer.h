#ifndef HITBOX_RENDER_H_GUARD
#define HITBOX_RENDER_H_GUARD
#include <vector>
#include <vao.h>
#include <shader.h>

//Holds the state needed to render hitboxes for debug purposes.
class HitboxRenderer
{
public:
	HitboxRenderer();
	Shader sSimple;

	enum 
	{
		gray = 0,
		green,
		red,
	};

	void GenerateHitboxVertices(const std::vector<float> &boxes, int pickedColor);
	void LoadHitboxVertices();
	void DontDraw();
	void Draw();

private:
	Vao vGeometry;
	int geoVaoId;

	std::vector<uint16_t> clientElements;
	std::vector<float> clientQuads;
	int quadsToDraw;
	size_t acumSize = 0;
	size_t acumQuads = 0;
	size_t acumElements = 0;
	float zOrder = 0; 

	int lAlphaS;
};

#endif /* HITBOX_RENDER_H_GUARD */
