#include <fstream>
#include <iostream>
#include <string>

#include <glad/glad.h>

#include <png.h>

#include "texture.h"

int pal = 0; //temporary

//Auxiliary function for printing warnings. Images saved with photoshop generate iCCP warnings, so it's annoying.
static void png_user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	#ifdef LIB_PNG_WARN
	char* error_ptr = (char*)png_get_error_ptr(png_ptr);
	std::cerr << error_ptr << " - " << warning_msg << "\n";
	#endif
}

TextureData Texture::LoadTexture(const char* file_name)
{
	TextureData image;
	png_byte header[8];

	FILE *fp = fopen(file_name, "rb");
	if (fp == 0)
	{
		std::cerr << file_name << ": Couldn't open\n";
		return image;
	}

	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8))
	{
		std::cerr << file_name << ": Not a PNG\n";
		fclose(fp);
		return image;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp*)file_name, NULL, png_user_warning_fn);
	if (!png_ptr)
	{
		std::cerr << file_name << " error: png_create_read_struct returned 0.\n";
		fclose(fp);
		return image;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		std::cerr << file_name << " error: png_create_info_struct returned 0.\n";
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return image;
	}

	png_infop end_info = nullptr;
	// create png end info struct. Maybe not needed.
	/*
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		std::cerr << file_name << " error: png_create_info_struct returned 0.\n";
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		fclose(fp);
		return image;
	}
	*/

	if (setjmp(png_jmpbuf(png_ptr))) {
		std::cerr << "\t" << file_name << "\n";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return image;
	}

	// init png reading
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	png_set_alpha_mode(png_ptr, PNG_ALPHA_BROKEN, PNG_DEFAULT_sRGB);

	int bit_depth, color_type, interlace_method;
	png_uint_32 temp_width, temp_height;

	png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
		NULL, NULL, &interlace_method);
	interlace_method = png_set_interlace_handling(png_ptr);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{

		png_color* palette = (png_color*)png_malloc(png_ptr, 256*sizeof(png_color));

		std::ifstream pltefile;
		if(pal == 1)//Temporary hardcoded palettes into the sprites.
			pltefile.open("palettes/play2.act", std::ifstream::in | std::ifstream::binary);
		else
			pltefile.open("palettes/akicolor.act", std::ifstream::in | std::ifstream::binary);

		if(!pltefile.is_open())
		{
			std::cout << "Couldn't open grayscale palette file.\n";
			return image;
		}

		for (unsigned p = 0; p < 256; p++)
		{
		  png_color* col = &palette[p];
		  pltefile.read((char*)&col->red, sizeof(png_byte));
		  pltefile.read((char*)&col->green, sizeof(png_byte));
		  pltefile.read((char*)&col->blue, sizeof(png_byte));
		}
		pltefile.close();


			png_set_PLTE(png_ptr, info_ptr, palette, 256);

		png_set_palette_to_rgb(png_ptr);

	}


	//png_set_invert_alpha(png_ptr);



	png_read_update_info(png_ptr, info_ptr);
	

	// lenght in bytes of a row.
	unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	// glTexImage2d requires rows to be 4-byte aligned
	rowbytes += 3 - ((rowbytes-1) % 4);


	png_byte * image_data;
	image_data = png_bytep(malloc(rowbytes * temp_height * sizeof(png_byte)+15));
	if (image_data == NULL)
	{
		std::cerr << file_name << " error: could not allocate memory for PNG image data\n";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return image;
	}


	png_bytep * row_pointers;
	row_pointers = png_bytepp(malloc (sizeof(png_bytep) * temp_height) );
	if (row_pointers == NULL)
	{
		std::cerr << file_name << " error: could not allocate memory for PNG row pointers\n";
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free(image_data);
		fclose(fp);
		return image;
	}

	// set the individual row_pointers to point at the correct offsets of image_data
	// And that is backwards because of opengl.
	for (unsigned int i = 0; i < temp_height; i++)
	{
		row_pointers[temp_height - 1 - i] = image_data + i*rowbytes;
	}
	
	// Let libpng do the rest.
	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, end_info);


	image.data = image_data;
	image.width = temp_width;
	image.height = temp_height;
	image.format = GL_RGBA;
	if (color_type == PNG_COLOR_TYPE_RGB)
		image.format = GL_RGB;
	//std::cout << "Loaded " << file_name << "\n";

	// clean up
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	free(row_pointers);
	fclose(fp);

	return image;
}

Texture::Texture() : data(), id(0), isLoaded(false), isApplied(false){}; //initializes as 0
Texture::~Texture()
{
	if(isApplied) //This gets triggered by vector.resize() when the class instance is NOT a pointer.
		Unapply();
	if(isLoaded)
		Unload();
}

void Texture::Load(std::string _filename)
{
	filename = _filename;
	data = LoadTexture(filename.c_str());
}

void Texture::Apply(bool repeat, bool linearFilter)
{
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

	glTexImage2D(GL_TEXTURE_2D, 0, data.format, data.width, data.height, 0, data.format, GL_UNSIGNED_BYTE, data.data);

}
void Texture::Unapply()
{
	isApplied = false;
	glDeleteTextures(1, &id);
}
void Texture::Unload()
{
	isLoaded = false;
	free(data.data);
}
