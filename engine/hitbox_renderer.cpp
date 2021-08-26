#include "hitbox_renderer.h"

constexpr int maxBoxes = 33;

HitboxRenderer::HitboxRenderer():
vGeometry(Vao::F3F3, GL_STREAM_DRAW, maxBoxes*5*sizeof(uint16_t)),
quadsToDraw(0)
{
	sSimple.LoadShader("data/simple.vert", "data/simple.frag");
	sSimple.Use();
	lAlphaS = sSimple.GetLoc("Alpha");

	geoVaoId = vGeometry.Prepare(sizeof(float)*6*4*maxBoxes, nullptr);
	vGeometry.Load();

	//Bit dangerous to have this here but I don't think it'll be used anywhere else.
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
	zOrder = 0;
}

void HitboxRenderer::Draw()
{
	if(quadsToDraw > 0)
	{
		glEnable(GL_DEPTH_TEST);
		sSimple.Use();
		vGeometry.Bind();
		glUniform1f(lAlphaS, 1.0f);
		glDrawElements(GL_LINE_LOOP, quadsToDraw, GL_UNSIGNED_SHORT, nullptr); //No offset this time
		glUniform1f(lAlphaS, 0.4f);
		glDrawElements(GL_TRIANGLE_FAN, quadsToDraw, GL_UNSIGNED_SHORT, nullptr);
		glDisable(GL_DEPTH_TEST);
	}
}

void HitboxRenderer::GenerateHitboxVertices(const std::vector<float> &hitboxes, int pickedColor)
{
	int size = hitboxes.size();
	if(size <= 0)
	{
		quadsToDraw = 0;
		return;
	}
	
	//r, g, b, z order
	constexpr float colors[][3]={
		{1.0, 1.0, 1.0},		//gray
		{0.0, 1, 0.0},	//green
		{1, 0.2, 0.2}	//red
	};

	const float *color = colors[pickedColor];

	constexpr int tX[] = {0,1,1,0};
	constexpr int tY[] = {0,0,1,1};


	int floats = size*6; //6 Vertices with 6 attributes.
	if(clientQuads.size() < floats + acumQuads)
		clientQuads.resize(floats + acumQuads);

	int elements = (size/4)*5;
	if(clientElements.size() < elements + acumElements)
		clientElements.resize(elements + acumElements);
	
	int dataI = acumQuads;
	for(int i = 0; i < size; i+=4)
	{
		for(int j = 0; j < 4*6; j+=6)
		{
			//X, Y, Z, R, G, B
			clientQuads[dataI+j+0] = hitboxes[i+0] + (hitboxes[i+2]-hitboxes[i+0])*tX[j/6];
			clientQuads[dataI+j+1] = hitboxes[i+1] + (hitboxes[i+3]-hitboxes[i+1])*tY[j/6];
			clientQuads[dataI+j+2] = 10000-zOrder;
			clientQuads[dataI+j+3] = color[0];
			clientQuads[dataI+j+4] = color[1];
			clientQuads[dataI+j+5] = color[2];
		}
		dataI += 4*6;

		int eI = (i/4)*5 + acumElements;
		for(int offset=0; offset < 4; offset++)
			clientElements[eI+offset] = offset+i+acumSize;
		clientElements[eI+4] = 0xFFFF;
	}

	acumQuads = dataI;
	acumElements += (elements);
	acumSize += size;
	zOrder++;
}

void HitboxRenderer::LoadHitboxVertices()
{
	vGeometry.UpdateBuffer(geoVaoId, clientQuads.data(), acumQuads*sizeof(float));
	vGeometry.UpdateElementBuffer(clientElements.data(), (acumElements)*sizeof(uint16_t));
	quadsToDraw = acumElements;
	
	acumQuads = 0;
	acumElements = 0;
	acumSize = 0;
	zOrder = 0;
}

void HitboxRenderer::DontDraw()
{
	quadsToDraw = 0;
}