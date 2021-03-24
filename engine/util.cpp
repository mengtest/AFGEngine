#include <glad/glad.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

#include "util.h"

int DrawText(std::string text, std::vector<float> &arr, float x, float y)
{
	int i = 0;
	while (i < text.size() && i < arr.size()/24){
			int character = (text[i]);
			constexpr int rows = 16, cols = 16;
			const float magicxoffset = (1.f/rows);
			const float magicyoffset = (1.f/cols);
			constexpr float width = 4, height = 8; 

			//BL
			arr[i*24 + 0] = 0+x+(i*width);
			arr[i*24 + 1] = y;
			arr[i*24 + 2] = magicxoffset*((character%rows));
			arr[i*24 + 3] = 1-(magicyoffset*(character/rows));

			//BR
			arr[i*24 + 4] = width+x+(i*width);
			arr[i*24 + 5] = y;
			arr[i*24 + 6] = magicxoffset*((character%rows)+1);
			arr[i*24 + 7] = 1-(magicyoffset*(character/rows));

			//TR
			arr[i*24 + 8] = width+x+(i*width);
			arr[i*24 + 9] = y-height;
			arr[i*24 + 10] = magicxoffset*((character%rows)+1);
			arr[i*24 + 11] = 1-(magicyoffset*((character/rows)+1));

			//SPLIT
			//TR
			arr[i*24 + 12] = width+x+(i*width);
			arr[i*24 + 13] = y-height;
			arr[i*24 + 14] = magicxoffset*((character%rows)+1);
			arr[i*24 + 15] = 1-(magicyoffset*((character/rows)+1));

			//TL
			arr[i*24 + 16] = 0+x+(i*width);
			arr[i*24 + 17] = y-height;
			arr[i*24 + 18] = magicxoffset*((character%rows));
			arr[i*24 + 19] = 1-(magicyoffset*((character/rows)+1));

			//TL
			arr[i*24 + 20] = 0+x+(i*width);
			arr[i*24 + 21] = y;
			arr[i*24 + 22] = magicxoffset*((character%rows));
			arr[i*24 + 23] = 1-(magicyoffset*(character/rows));

			++i;
	}

	return i*6;
}

CoordinateHelper::CoordinateHelper(GLfloat w, GLfloat h, GLfloat a, GLfloat b, std::vector<float> &coordArray, e_coordtype choice) :
shrinkBorders(false),
coordPusher(coordArray)
{
	ChangeDimensions(w, h, a, b, choice);
}

void CoordinateHelper::ChangeDimensions(GLfloat w, GLfloat h, GLfloat a, GLfloat b, e_coordtype choice)
{
	width = w;
	height = h;

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


//TODO
//God fucking damnit, quads. Get rid of this garbage asap
void CoordinateHelper::PushQuad(GLfloat x, GLfloat y, int start, bool hFlip)
{
	if(!hFlip)
	{
		coordPusher[start+0]=(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher[start+1]=(chunkHeight*y/height + shrinkBorders*0.5/height);   //BR
		coordPusher[start+4]=(chunkWidth*x/width + shrinkBorders*0.5/width);		coordPusher[start+5]=(chunkHeight*y/height + shrinkBorders*0.5/height);   //BL
		coordPusher[start+8]=(chunkWidth*x/width + shrinkBorders*0.5/width);		coordPusher[start+9]=(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TL

		coordPusher[start+12]=(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher[start+13]=(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TL
		coordPusher[start+16]=(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher[start+17]=(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TR
		coordPusher[start+20]=(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher[start+21]=(chunkHeight*y/height + shrinkBorders*0.5/height);   //BR
	}
	else //it's horizontally flipped
	{
		coordPusher[start+0]=(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher[start+1]=(chunkHeight*y/height + shrinkBorders*0.5/height);   //BL
		coordPusher[start+4]=(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher[start+5]=(chunkHeight*y/height + shrinkBorders*0.5/height);   //BR
		coordPusher[start+8]=(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher[start+9]=(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TR

		coordPusher[start+12]=(chunkWidth*(x+1)/width - shrinkBorders*0.5/width);		coordPusher[start+13]=(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TR
		coordPusher[start+16]=(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher[start+17]=(chunkHeight*(y+1)/height - shrinkBorders*0.5/height); //TL
		coordPusher[start+20]=(chunkWidth*x/width + shrinkBorders*0.5/width);			coordPusher[start+21]=(chunkHeight*y/height + shrinkBorders*0.5/height);   //BL
	}
}

//gross
void PushQuad(float x, float y, float w, float h, std::vector<float> *vertArray, CoordinateHelper &ch, GLfloat xt, GLfloat yt, bool flip)
{
	vertArray->push_back(x); vertArray->push_back(y);  //BL
	int tcoords = ch.coordPusher.size(); vertArray->push_back(0); vertArray->push_back(0);

	vertArray->push_back(x+w);  vertArray->push_back(y); vertArray->push_back(0); vertArray->push_back(0); //BR
	vertArray->push_back(x+w);  vertArray->push_back(y+h); vertArray->push_back(0); vertArray->push_back(0);  //TR

	vertArray->push_back(x+w);  vertArray->push_back(y+h); vertArray->push_back(0); vertArray->push_back(0);  //TR
	vertArray->push_back(x);    vertArray->push_back(y+h); vertArray->push_back(0); vertArray->push_back(0);  //TL
	vertArray->push_back(x);    vertArray->push_back(y); vertArray->push_back(0); vertArray->push_back(0);  //BL

	ch.PushQuad(xt, yt, tcoords, flip);
}

//Painful
void ModifyQuad(float x, float y, float w, float h, std::vector<float> *vertArray, int index, bool relativeMovement)
{
	if(vertArray == nullptr)
		return;
	(*vertArray)[index]=(x)+	(*vertArray)[index]*relativeMovement;		(*vertArray)[index+1]=(y)+	(*vertArray)[index+1]*relativeMovement;  //BL
	(*vertArray)[index+4]=(x+w)+(*vertArray)[index+2]*relativeMovement;		(*vertArray)[index+5]=(y)+	(*vertArray)[index+3]*relativeMovement;  //BR
	(*vertArray)[index+8]=(x+w)+(*vertArray)[index+4]*relativeMovement;		(*vertArray)[index+9]=(y+h)+(*vertArray)[index+5]*relativeMovement;  //TR

	(*vertArray)[index+12]=(x+w)+(*vertArray)[index+6]*relativeMovement;	(*vertArray)[index+13]=(y+h)+(*vertArray)[index+7]*relativeMovement;  //TR
	(*vertArray)[index+16]=(x)+	(*vertArray)[index+8]*relativeMovement;		(*vertArray)[index+17]=(y+h)+(*vertArray)[index+9]*relativeMovement;  //TL
	(*vertArray)[index+20]=(x)+	(*vertArray)[index+10]*relativeMovement;	(*vertArray)[index+21]=(y)+	(*vertArray)[index+11]*relativeMovement;  //BL
}
