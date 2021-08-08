#include "image.h"
#include <memory>
#include <png.h>
#include <cstdio>
#include <csetjmp>
#include <fstream>
#include <cassert>
#include <stdint.h>

#ifndef LIB_PNG_WARN
	#define LIB_PNG_WARN 1
#endif

//Auxiliary function for printing warnings. Images saved with photoshop generate iCCP warnings. It's annoying.
void png_user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	if(LIB_PNG_WARN)
	{
		char* error_ptr = (char*)png_get_error_ptr(png_ptr);
		fprintf(stderr, "%s - %s\n", error_ptr, warning_msg);
	}
}

ImageData::ImageData():
data(nullptr), width(0), height(0), flag(0), bytesPerPixel(0)
{}

ImageData::ImageData(uint32_t _width, uint32_t _height, uint32_t _bytesPerPixel):
data(nullptr), width(_width), height(_height), flag(0), bytesPerPixel(_bytesPerPixel)
{
	data = (uint8_t*) malloc(width * height * bytesPerPixel * sizeof(uint8_t));
}

ImageData::ImageData(const char* image, const char* palette, bool linearAlpha): ImageData()
{
	LoadFromPng(image, palette, linearAlpha);
}

ImageData::~ImageData()
{
	free(data);
}

std::size_t ImageData::GetMemSize() const
{
	return width*height*bytesPerPixel;
}

void ImageData::FreeData()
{
	free(data);
	data = nullptr;
}

bool ImageData::WriteAsPng(const char* file_name, const char* palette_file) const
{
	FILE *fp = fopen(file_name, "wb");
	if (fp == 0)
	{
		fprintf(stderr, "Image write: %s couldn't be opened\n", file_name);
		return false;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp*)file_name, nullptr, png_user_warning_fn);
	if (!png_ptr)
	{
		fprintf(stderr, "Image write: %s error: png_create_write_struct returned 0\n", file_name);
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "Image write: %s error: png_create_info_struct returned 0\n", file_name);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Image write: %s fatal error.\n", file_name);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	int color_type = 0;
	switch(bytesPerPixel)
	{
		case 1:
			color_type = palette_file ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_GRAY;
			break;
		case 3:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case 4:
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
		default:
			fprintf(stderr, "Image write: %s error: unhandled number of channel: %i\n", file_name, bytesPerPixel);
			break;
	}
	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_color *palette = nullptr;
	if(palette_file)
	{
		palette = (png_color*)png_malloc(png_ptr, 256*sizeof(png_color));
		std::ifstream pltefile;

		pltefile.open(palette_file, std::ifstream::in | std::ifstream::binary);
		if(!pltefile.is_open())
		{
			fprintf(stderr, "Couldn't open palette file.\n");
			png_destroy_write_struct(&png_ptr, &info_ptr);
			png_free(png_ptr, palette);
			return false;
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
	}
	
	if(bytesPerPixel == 1)
	{
		png_byte trans_alpha = 0;
		png_set_tRNS(png_ptr, info_ptr, &trans_alpha, 1, nullptr);
	}

	png_bytep * row_pointers;
	row_pointers = png_bytepp(malloc (sizeof(png_bytep) * height) );
	if (row_pointers == nullptr)
	{
		fprintf(stderr, "Image write: %s error: could not allocate memory for PNG row pointers\n", file_name);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	// set the individual row_pointers to point at the correct offsets of image_data
	for (unsigned int i = 0; i < height; i++)
	{
		row_pointers[height - 1 - i] = data + i*(width*bytesPerPixel);
	}

	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	free(row_pointers);
	png_free(png_ptr, palette);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	return true;
}

bool ImageData::LoadFromPng(const char* file_name, const char* palette_file, bool linearAlpha)
{
	FreeData();
	png_byte header[8];
	FILE *fp = fopen(file_name, "rb");
	if (fp == 0)
	{
		fprintf(stderr, "Image load: %s couldn't be opened\n", file_name);
		return false;
	}

	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8))
	{
		fprintf(stderr, "Image load: %s is not a PNG file\n", file_name);
		fclose(fp);
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp*)file_name, nullptr, png_user_warning_fn);
	if (!png_ptr)
	{
		fprintf(stderr, "Image load: %s error: png_create_read_struct returned 0\n", file_name);
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "Image load: %s error: png_create_info_struct returned 0\n", file_name);
		png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
		fclose(fp);
		return false;
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
		fprintf(stderr, "Image load: %s error: Calling setjmp to pass png_jmpbuf failed\n", file_name);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return false;
	}

	// init png reading
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	if(!linearAlpha)
		png_set_alpha_mode(png_ptr, PNG_ALPHA_BROKEN, PNG_DEFAULT_sRGB);

	int bit_depth, color_type, interlace_method, interlace_type, compression_type, filter_method;
	png_uint_32 temp_width, temp_height;

	png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
		&interlace_type, &compression_type, &filter_method);
	assert(bit_depth == 8);
	interlace_method = png_set_interlace_handling(png_ptr);

	png_color* palette = nullptr;
	if ((color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_GRAY) && palette_file)
	{
		
		palette = (png_color*)png_malloc(png_ptr, 256*sizeof(png_color));

		std::ifstream pltefile;

		pltefile.open(palette_file, std::ifstream::in | std::ifstream::binary);
		if(!pltefile.is_open())
		{
			fprintf(stderr, "Couldn't open palette file.\n");
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			png_free(png_ptr, palette);
			return false;
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

		if(color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_expand(png_ptr);
		/*
		if(color_type == PNG_COLOR_TYPE_GRAY)
		{
			png_byte trans_alpha = 0;
			png_set_tRNS(png_ptr, info_ptr, &trans_alpha, 1, nullptr);
			png_set_gray_to_rgb(png_ptr);
		}*/
	}
	//png_set_invert_alpha(png_ptr);

	png_read_update_info(png_ptr, info_ptr);
	

	// lenght in bytes of a row.
	unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	// glTexImage2d requires rows to be 4-byte aligned
	rowbytes += 3 - ((rowbytes-1) % 4);


	png_byte * image_data;
	image_data = png_bytep(malloc(rowbytes * temp_height * sizeof(png_byte)));
	if (image_data == nullptr)
	{
		fprintf(stderr, "Image load: %s error: could not allocate memory for PNG image data\n", file_name);
		png_free(png_ptr, palette);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(fp);
		return false;
	}


	png_bytep * row_pointers;
	row_pointers = png_bytepp(malloc (sizeof(png_bytep) * temp_height) );
	if (row_pointers == nullptr)
	{
		fprintf(stderr, "Image load: %s error: could not allocate memory for PNG row pointers\n", file_name);
		png_free(png_ptr, palette);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free(image_data);
		fclose(fp);
		return false;
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

	bytesPerPixel = rowbytes/temp_width;

	if(color_type == PNG_COLOR_TYPE_GRAY && palette)
	{
		uint8_t *image_data32bit = (uint8_t*)malloc(temp_height*temp_width*4);
		for (unsigned int p = 0; p < temp_height * rowbytes * sizeof(png_byte); p+=bytesPerPixel)
		{
			png_byte ref = image_data[p];
			
			image_data32bit[p*4] = palette[ref].red;
			image_data32bit[p*4 + 1] = palette[ref].green;
			image_data32bit[p*4 + 2] = palette[ref].blue;
			image_data32bit[p*4 + 3] = ref == 0 ? 0 : 255;
		}
		free(image_data);
		image_data = image_data32bit;
		bytesPerPixel = 4;
	}

	png_free(png_ptr, palette);

	data = image_data;
	width = temp_width;
	height = temp_height;
	

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	free(row_pointers);
	fclose(fp);

	return true;
}

