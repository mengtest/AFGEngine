#include "image.h"

#include <memory>
#include <png.h>
#include <cstdio>
#include <csetjmp>
#include <fstream>
#include <cassert>

#ifndef LIB_PNG_WARN
	#define LIB_PNG_WARN 1
#endif

ImageData::ImageData(): data(nullptr), width(0), height(0), flag(0), bytesPerPixel(0)
{}

//Auxiliary function for printing warnings. Images saved with photoshop generate iCCP warnings. It's annoying.
void png_user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	if(LIB_PNG_WARN)
	{
		char* error_ptr = (char*)png_get_error_ptr(png_ptr);
		fprintf(stderr, "%s - %s\n", error_ptr, warning_msg);
	}
}

std::unique_ptr<ImageData> LoadImageFromPng(const char* file_name, const char* palette_file)
{
	std::unique_ptr<ImageData> image = std::make_unique<ImageData>();
	image->data = nullptr;
	png_byte header[8];

	FILE *fp = fopen(file_name, "rb");
	if (fp == 0)
	{
		fprintf(stderr, "ImageLoader: %s couldn't be opened\n", file_name);
		return image;
	}

	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8))
	{
		fprintf(stderr, "ImageLoader: %s is not a PNG file\n", file_name);
		fclose(fp);
		return image;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp*)file_name, nullptr, png_user_warning_fn);
	if (!png_ptr)
	{
		fprintf(stderr, "ImageLoader: %s error: png_create_read_struct returned 0\n", file_name);
		fclose(fp);
		return image;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "ImageLoader: %s error: png_create_info_struct returned 0\n", file_name);
		png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
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
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);
		fclose(fp);
		return image;
	}
	*/

	//error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "ImageLoader: %s error: Calling setjmp to pass png_jmpbuf failed\n", file_name);
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
		nullptr, nullptr, &interlace_method);
	assert(bit_depth == 8);
	interlace_method = png_set_interlace_handling(png_ptr);

	png_color* palette = nullptr;
	if (color_type == PNG_COLOR_TYPE_PALETTE && palette_file)
	{
		palette = (png_color*)png_malloc(png_ptr, 256*sizeof(png_color));

		std::ifstream pltefile;

		pltefile.open(palette_file, std::ifstream::in | std::ifstream::binary);
		if(!pltefile.is_open())
		{
			fprintf(stderr, "Couldn't open palette file.\n");
			return image;
		}

		for (unsigned int p = 0; p < 256; p++)
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
	if (image_data == nullptr)
	{
		fprintf(stderr, "ImageLoader: %s error: could not allocate memory for PNG image data\n", file_name);
		png_free(png_ptr, palette);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return image;
	}


	png_bytep * row_pointers;
	row_pointers = png_bytepp(malloc (sizeof(png_bytep) * temp_height) );
	if (row_pointers == nullptr)
	{
		fprintf(stderr, "ImageLoader: %s error: could not allocate memory for PNG row pointers\n", file_name);
		png_free(png_ptr, palette);
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
	png_free(png_ptr, palette);

	image->data = image_data;
	image->width = temp_width;
	image->height = temp_height;
	image->bytesPerPixel = rowbytes/temp_width;

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	free(row_pointers);
	fclose(fp);

	return image;
}

ImageData::~ImageData()
{
	free(data);
}