#include "texture.h"
#include <image.h>
#include <glad/glad.h>
#include <lz4.h>

#include <fstream>
#include <iostream>
#include <string>
#include <cassert>

constexpr int s3tc = 0x1000;

int pal = 0; //temporary

Texture::Texture() : id(0), isApplied(false){}; //initializes as 0
Texture::Texture(Texture&& texture)
{
	id = texture.id;
	isApplied = texture.isApplied;
	filename = std::move(texture.filename);
	is8bpp = texture.is8bpp;

	texture.isApplied = false;
}

Texture::~Texture()
{
	if(isApplied)
		Unapply();
}

void Texture::LoadPng(std::filesystem::path imageFile, const texture_options options, std::filesystem::path paletteFile)
{
	filename = imageFile.filename().string();

	const char *palette = nullptr;
	if(!paletteFile.empty())
		palette = paletteFile.string().c_str();
	
	ImageData image(imageFile.string().c_str(), palette, options.linearAlpha);
	Apply(options, image.bytesPerPixel, image.width, image.height, image.data);
	if(image.bytesPerPixel == 1)
		is8bpp = true;
}

void Texture::LoadLzs3(std::filesystem::path imageFile, const texture_options options)
{
	filename = imageFile.filename().string();
	std::ifstream file(imageFile, std::ios::binary);

	struct{
		uint32_t type;
		uint32_t size;
		uint32_t cSize;
		uint16_t w,h;
	} meta;
	file.read((char*)&meta, sizeof(meta));
	auto data = std::make_unique<char[]>(meta.size);
	auto cData = std::make_unique<char[]>(meta.cSize);
	file.read(cData.get(), meta.cSize);

	int decompSize = LZ4_decompress_safe(cData.get(), data.get(), meta.cSize, meta.size);
	if(decompSize < 0 || decompSize != meta.size)
		throw std::runtime_error("Texture::LoadLzs3: Error while decompressing LZ4 - "+ filename + "\n");

	Apply(options, meta.type, meta.w, meta.h, data.get(), decompSize);
}

void Texture::Apply(const texture_options &options, int dataType, int w, int h, void* data, int compressedSize)
{
	this->rectangle = options.rectangle;

	isApplied = true;
	glGenTextures(1, &id);

	GLuint textype = GL_TEXTURE_2D;
	if(rectangle)
		textype = GL_TEXTURE_RECTANGLE;

	glBindTexture(textype, id);
	if(options.repeat && !rectangle)
	{
		glTexParameteri(textype, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(textype, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}
	else
	{
		glTexParameteri(textype, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri(textype, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}
	if(options.linearFilter)
		glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(textype, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if(compressedSize)
	{
		GLenum intType = 0;
		switch(dataType)
		{
			case 1:
				intType = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				break;
			case 2:
				intType = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				break;
			case 3:
				intType = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			default:
				std::cerr << filename.c_str() << " unhandled s3tc format "<< dataType << ".\n";
				break;
		}
		glCompressedTexImage2D(textype, 0, intType, w, h, 0, compressedSize, data);
	}
	else{
		GLenum extType = 0;
		GLenum intType = 0;
		switch(dataType)
		{
			case 1:
				extType = GL_RED;
				intType = GL_R8;
				break;
			case 3:
				extType = GL_RGB;
				intType = GL_RGB8;
				break;
			case 4:
				extType = GL_RGBA;
				intType = GL_RGBA8;
				//intType = GL_COMPRESSED_RGBA;
				break;
			default:
				std::cerr << filename.c_str() << " unhandled format "<< dataType << ".\n";
				break;
		}

		glTexImage2D(textype, 0, intType, w, h, 0, extType, GL_UNSIGNED_BYTE, data);
	}

}
void Texture::Unapply()
{
	isApplied = false;
	glDeleteTextures(1, &id);
}
