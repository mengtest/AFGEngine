#include "texture.h"
#include <image.h>
#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <string>

int pal = 0; //temporary

Texture::Texture() : id(0), isLoaded(false), isApplied(false){}; //initializes as 0
Texture::Texture(Texture&& texture)
{
	image = std::move(texture.image);
}

Texture::~Texture()
{
	if(isApplied) //This gets triggered by vector.resize() when the class instance is NOT a pointer.
		Unapply();
	if(isLoaded)
		Unload();
}

void Texture::Load(std::string imageFile, std::string paletteFile)
{
	filename = imageFile;

	const char *palette = nullptr;
	if(!paletteFile.empty())
		palette = paletteFile.c_str();
	image = LoadImageFromPng(imageFile.c_str(), palette);
}

void Texture::Apply(bool repeat, bool linearFilter)
{
	/*
	for(int i = 0; i < image->height; i++)
	{
		for(int j = 0; j < image->width; j++)
		{
			std::cout << (int)(image->data[i*image->width + j]);
		}
		std::cout << "\n";
	}
	*/

	isApplied = true;
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);
	if(repeat)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}
	if(linearFilter)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	GLenum extType = 0;
	GLenum intType = 0;
	switch(image->bytesPerPixel)
	{
		case 1:
			extType = GL_RED;
			intType = GL_RGB;
			std::cout << filename << " uses 8bpp!\n";
			break;
		case 3:
			extType = GL_RGB;
			intType = GL_RGBA;
			break;
		case 4:
			extType = GL_RGBA;
			intType = GL_RGBA;
			break;
		default:
			std::cout << filename << " unhandled format.\n";
			break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, intType, image->width, image->height, 0, extType, GL_UNSIGNED_BYTE, image->data);

}
void Texture::Unapply()
{
	isApplied = false;
	glDeleteTextures(1, &id);
}
void Texture::Unload()
{
	isLoaded = false;
}
