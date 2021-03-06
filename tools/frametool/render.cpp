#include "render.h"
#include <iostream>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <glad/glad.h>
#include <imgui.h>

namespace fs = std::filesystem;

constexpr int maxBoxes = 33;

struct VertexData8
{
	unsigned short x,y,s,t;
	unsigned short atlasId;
};

Render::Render():
vGeometry(Vao::F3F3, GL_STREAM_DRAW, maxBoxes*5*sizeof(uint16_t)),
colorRgba{1,1,1,1},
spriteId(-1),
quadsToDraw(0),
x(0), offsetX(0),
y(0), offsetY(0),
rotX(0), rotY(0), rotZ(0),
uniforms("Common", 1)
{
	sSimple.LoadShader("data/simple.vert", "data/simple.frag");
	sSimple.Use();
	lAlphaS = sSimple.GetLoc("Alpha");

	//Bind transform matrix uniforms.
	uniforms.Init(sizeof(float)*16);
	uniforms.Bind(sSimple.program);
	uniforms.Bind(gfx.indexedS.program);
	uniforms.Bind(gfx.rectS.program);

	float lines[]
	{
		-10000, 0, -1,	1,1,1,
		10000, 0, -1,	1,1,1,
		0, 10000, -1,	1,1,1,
		0, -10000, -1,	1,1,1,
	};
	
	geoParts[LINES] = vGeometry.Prepare(sizeof(lines), lines);
	geoParts[BOXES] = vGeometry.Prepare(sizeof(float)*6*4*maxBoxes, nullptr);
	vGeometry.Load();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
}

void Render::LoadPalette(fs::path file)
{
	std::ifstream pltefile(file, std::ifstream::in | std::ifstream::binary);
	uint8_t palette[256*3];

	pltefile.read((char*)palette, 256*3);
	pltefile.close();

	GLuint paletteGlId;
	glGenTextures(1, &paletteGlId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, paletteGlId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glActiveTexture(GL_TEXTURE0);
}

void Render::LoadGraphics(fs::path path)
{
	gfx.LoadGfxFromDef(path);
	gfx.LoadingDone();
}

void Render::Draw()
{
	if(int err = glGetError())
	{
		std::cerr << "GL Error: 0x" << std::hex << err << "\n";
	}

	//Lines
	sSimple.Use();
	glm::mat4 view = glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, 1.f));
	view = glm::translate(view, glm::vec3(x,y,0.f));
	SetModelView(std::move(view));
	glUniform1f(lAlphaS, 0.25f);
	vGeometry.Bind();
	vGeometry.Draw(geoParts[LINES], GL_LINES);

	//Sprite
	constexpr float tau = glm::pi<float>()*2.f;
	view = glm::mat4(1.f);
	view = glm::scale(view, glm::vec3(scale, scale, 1.f));
	view = glm::translate(view, glm::vec3(x,y,0.f));
 	view = glm::scale(view, glm::vec3(scaleX,scaleY,0));
	view = glm::rotate(view, -rotZ*tau, glm::vec3(0.0, 0.f, 1.f));
	view = glm::rotate(view, rotY*tau, glm::vec3(0.0, 1.f, 0.f));
	view = glm::rotate(view, rotX*tau, glm::vec3(1.0, 0.f, 0.f));
	view = glm::translate(view, glm::vec3(offsetX, offsetY,0.f)); 
	glDisable(GL_PRIMITIVE_RESTART);
	SetModelView(std::move(view));
	gfx.Begin();
	gfx.Draw(spriteId);
	gfx.End();
	glEnable(GL_PRIMITIVE_RESTART);

	//Boxes
	sSimple.Use();
	view = glm::mat4(1.f);
	view = glm::scale(view, glm::vec3(scale, scale, 1.f));
	view = glm::translate(view, glm::vec3(x,y,0.f));
	SetModelView(std::move(view));

	vGeometry.Bind();
	glUniform1f(lAlphaS, 0.6f);
	glDrawElementsBaseVertex(GL_LINE_LOOP, quadsToDraw, GL_UNSIGNED_SHORT, nullptr, 4); //4 is the offset after lines. w/e
	glUniform1f(lAlphaS, 0.3f);
	glDrawElementsBaseVertex(GL_TRIANGLE_FAN, quadsToDraw, GL_UNSIGNED_SHORT, nullptr, 4);
}

void Render::SetModelView(glm::mat4&& view)
{
	uniforms.SetData(glm::value_ptr(projection*view));
}

void Render::UpdateProj(float w, float h)
{
	glViewport(0, 0, w, h);
	projection = glm::ortho<float>(0, w, 0, h, -1024.f, 1024.f);
}

void Render::GenerateHitboxVertices(const std::vector<int> &hitboxes, color_t pickedColor)
{
	int size = hitboxes.size();
	if(size <= 0)
	{
		quadsToDraw = 0;
		return;
	}
	
	//r, g, b, z order
	constexpr float colors[][3]={
		{1, 1, 1},		//gray
		{0.2, 1, 0.2},	//green
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
			clientQuads[dataI+j+2] = zOrder+1000.f;
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

void Render::LoadHitboxVertices()
{
	vGeometry.UpdateBuffer(geoParts[BOXES], clientQuads.data(), acumQuads*sizeof(float));
	vGeometry.UpdateElementBuffer(clientElements.data(), (acumElements)*sizeof(uint16_t));
	quadsToDraw = acumElements;
	
	acumQuads = 0;
	acumElements = 0;
	acumSize = 0;
	zOrder = 0;
}

void Render::DontDraw()
{
	quadsToDraw = 0;
}

void Render::SetImageColor(float *rgba)
{
	if(rgba)
	{
		for(int i = 0; i < 4; i++)
			colorRgba[i] = rgba[i];
	}
	else
	{
		for(int i = 0; i < 4; i++)
			colorRgba[i] = 1.f;
	}
}
