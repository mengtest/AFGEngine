#include <glad/glad.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

#include "util.h"

void DrawTextA(std::string text, GLuint textureid, float x, float y, float z)
{
    glBindTexture(GL_TEXTURE_2D, textureid);
    int i = 0;
    glBegin(GL_QUADS);
    while (text[i] != 0){
            int character = (text[i]);
            const float magicxoffset = (1.f/32.f);
            const float magicyoffset = (1.f/8.f);
            glColor3f(1, 1, 1);
            glTexCoord2f(magicxoffset*((character%32)), 1-(magicyoffset*(character/32)));
            glVertex3f(0+x+(i*8), y, z);
            glTexCoord2f(magicxoffset*((character%32)+1), 1-(magicyoffset*(character/32)));
            glVertex3f(8+x+(i*8), y, z);
            glTexCoord2f(magicxoffset*((character%32)+1), 1-(magicyoffset*((character/32)+1)));
            glVertex3f(8+x+(i*8), y-12, z);
            glTexCoord2f(magicxoffset*((character%32)), 1-(magicyoffset*((character/32)+1)));
            glVertex3f(0+x+(i*8), y-12, z);
            ++i;
    }
    glEnd();
}

CoordinateHelper::CoordinateHelper(GLfloat w, GLfloat h, GLfloat a, GLfloat b, std::vector<float> *coordArray, e_coordtype choice) : shrinkBorders(false)
{
	coordPusher = coordArray;
	ChangeDimensions(w, h, a, b, choice);
}

void CoordinateHelper::ChangeDimensions(GLfloat w, GLfloat h, GLfloat a, GLfloat b, e_coordtype choice)
{
	width = w;
	height = h;
	assert((choice != NONE && !coordPusher)==0);
	switch(choice)
    {
    case ROWCOL:
        chunkWidth = width/a;
        chunkHeight = height/b;
        break;
    case CHUNK_SIZE:
        chunkWidth = a;
        chunkHeight = b;
        break;
    case NONE:
    default:
        break;
    }
}

void CoordinateHelper::GetUVCoords(float &u, float &v)
{
    u = u/width;
    v = v/height;
}

void CoordinateHelper::GetChunkCoords(GLfloat x, GLfloat y, float &u, float &v)
{
    u = chunkWidth*x/width;
    v = chunkHeight*y/height;
}

void CoordinateHelper::PushQuad(GLfloat x, GLfloat y, bool hFlip)
{
	if(!hFlip)
	{
		coordPusher->push_back(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher->push_back(chunkHeight*y/height + shrinkBorders*0.5/height);   //BR
		coordPusher->push_back(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher->push_back(chunkHeight*y/height + shrinkBorders*0.5/height);   //BL
		coordPusher->push_back(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher->push_back(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TL
		coordPusher->push_back(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher->push_back(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TR
	}
	else //it's horizontally flipped
	{
		coordPusher->push_back(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher->push_back(chunkHeight*y/height + shrinkBorders*0.5/height);   //BL
		coordPusher->push_back(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher->push_back(chunkHeight*y/height + shrinkBorders*0.5/height);   //BR
		coordPusher->push_back(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher->push_back(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TR
		coordPusher->push_back(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher->push_back(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TL
	}
}

void PushQuad(float x, float y, float w, float h, std::vector<float> *vertArray)
{
    vertArray->push_back(x);    vertArray->push_back(y);    //BL
    vertArray->push_back(x+w);  vertArray->push_back(y);    //BR
    vertArray->push_back(x+w);  vertArray->push_back(y+h);  //TR
    vertArray->push_back(x);    vertArray->push_back(y+h);  //TL
}

void ModifyQuad(float x, float y, float w, float h, std::vector<float> *vertArray, int index, bool relativeMovement)
{
	if(vertArray == nullptr)
		return;
    (*vertArray)[index]=(x)+	(*vertArray)[index]*relativeMovement;		(*vertArray)[index+1]=(y)+	(*vertArray)[index+1]*relativeMovement;  //BL
    (*vertArray)[index+2]=(x+w)+(*vertArray)[index+2]*relativeMovement;		(*vertArray)[index+3]=(y)+	(*vertArray)[index+3]*relativeMovement;  //BR
    (*vertArray)[index+4]=(x+w)+(*vertArray)[index+4]*relativeMovement;		(*vertArray)[index+5]=(y+h)+(*vertArray)[index+5]*relativeMovement;  //TR
    (*vertArray)[index+6]=(x)+	(*vertArray)[index+6]*relativeMovement;		(*vertArray)[index+7]=(y+h)+(*vertArray)[index+7]*relativeMovement;  //TL
}
