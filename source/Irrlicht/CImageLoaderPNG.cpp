// Copyright (C) 2002-2006 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_LIBPNG_
	#ifndef _IRR_USE_NON_SYSTEM_LIB_PNG_
	#include <png.h> // use system lib png
	#else // _IRR_USE_NON_SYSTEM_LIB_PNG_
	#include "libpng/png.h" // use irrlicht included lib png
	#endif // _IRR_USE_NON_SYSTEM_LIB_PNG_
#endif // _IRR_COMPILE_WITH_LIBPNG_

#include "CImageLoaderPNG.h"
#include <string.h>
#include "CImage.h"
#include "CReadFile.h"
#include "os.h"

namespace irr
{
namespace video
{

#ifdef _IRR_COMPILE_WITH_LIBPNG_
// PNG function for error handling
static void png_cpexcept_error(png_structp png_ptr, png_const_charp msg)
{
	os::Printer::log("PNG FATAL ERROR", msg, ELL_ERROR);
	longjmp(png_ptr->jmpbuf, 1);
}

// PNG function for file reading
void user_read_data_fcn(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	// changed by zola {
	irr::io::IReadFile* file=(irr::io::IReadFile*)png_ptr->io_ptr;
	check=(png_size_t) file->read((void*)data,length);
	// }

	if (check != length)
		png_error(png_ptr, "Read Error");
}
#endif // _IRR_COMPILE_WITH_LIBPNG_

CImageLoaderPng::CImageLoaderPng()
{
	// do something?
}

CImageLoaderPng::~CImageLoaderPng()
{
	// do something?
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageLoaderPng::isALoadableFileExtension(const c8* fileName)
{
#ifdef _IRR_COMPILE_WITH_LIBPNG_
	// added fix for file extension check by jox
	const c8* ext = strrchr(fileName, '.');
	if (ext == 0);
		return false;
	return (strcmp(ext, ".PNG") == 0) || (strcmp(ext, ".png") == 0);
#else
	return false;
#endif // _IRR_COMPILE_WITH_LIBPNG_
}


//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderPng::isALoadableFileFormat(irr::io::IReadFile* file)
{
#ifdef _IRR_COMPILE_WITH_LIBPNG_
	if (!file)
		return false;

	png_byte buffer[8];
	// Read the first few bytes of the PNG file
	if (file->read(buffer, 8) != 8)
		return false;

	// Check if it really is a PNG file
	return !png_sig_cmp(buffer, 0, 8);
#else
	return false;
#endif // _IRR_COMPILE_WITH_LIBPNG_
}


// load in the image data
IImage* CImageLoaderPng::loadImage(irr::io::IReadFile* file)
{
#ifdef _IRR_COMPILE_WITH_LIBPNG_
	if (!file)
		return 0;

	Image = 0;
	RowPointers = 0;

	png_byte buffer[8];
	// Read the first few bytes of the PNG file
	if( file->read(buffer, 8) != 8 )
	{
		os::Printer::log("LOAD PNG: can't read file\n", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Check if it really is a PNG file
	if( png_sig_cmp(buffer, 0, 8) )
	{
		os::Printer::log("LOAD PNG: not really a png\n", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Allocate the png read struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, (png_error_ptr)png_cpexcept_error, NULL);
	if (!png_ptr)
	{
		os::Printer::log("LOAD PNG: Internal PNG create read struct failure\n", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Allocate the png info struct
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		os::Printer::log("LOAD PNG: Internal PNG create info struct failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}

	// for proper error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		if (RowPointers)
			delete [] RowPointers;
		return 0;
	}

	// changed by zola so we don't need to have public FILE pointers
	png_set_read_fn(png_ptr, file, user_read_data_fcn);

	png_set_sig_bytes(png_ptr, 8); // Tell png that we read the signature

	png_read_info(png_ptr, info_ptr); // Read the info section of the png file

	// Extract info
	png_get_IHDR(png_ptr, info_ptr,
		(png_uint_32*)&Width, (png_uint_32*)&Height,
		&BitDepth, &ColorType, NULL, NULL, NULL);

	// Convert palette color to true color
	if (ColorType==PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	// Convert low bit colors to 8 bit colors
	if (BitDepth < 8)
		if (ColorType==PNG_COLOR_TYPE_GRAY)
			png_set_gray_1_2_4_to_8(png_ptr);
		else
			png_set_packing(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	// Convert high bit colors to 8 bit colors
	if (BitDepth == 16)
		png_set_strip_16(png_ptr);

	// Convert RGBA to BGRA
	if (ColorType==PNG_COLOR_TYPE_PALETTE || ColorType==PNG_COLOR_TYPE_RGB || ColorType==PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_bgr(png_ptr);

	// Convert gray color to true color
	if (ColorType==PNG_COLOR_TYPE_GRAY || ColorType==PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	// Update the changes
	png_read_update_info(png_ptr, info_ptr);

	// Extract info
	png_get_IHDR(png_ptr, info_ptr,
		(png_uint_32*)&Width, (png_uint_32*)&Height,
		&BitDepth, &ColorType, NULL, NULL, NULL);

	// Create the image structure to be filled by png data
	if (ColorType==PNG_COLOR_TYPE_RGB_ALPHA || ColorType==PNG_COLOR_TYPE_GRAY_ALPHA)
		Image = new CImage(ECF_A8R8G8B8, core::dimension2d<s32>(Width, Height));
	else
		Image = new CImage(ECF_R8G8B8, core::dimension2d<s32>(Width, Height));
	if (!Image)
	{
		os::Printer::log("LOAD PNG: Internal PNG create info struct failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}

	// Create array of pointers to rows in image data
	RowPointers = new png_bytep[Height];
	if (!RowPointers)
	{
		os::Printer::log("LOAD PNG: Internal PNG create info struct failure\n", file->getFileName(), ELL_ERROR);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		delete Image;
		return 0;
	}

	// Fill array of pointers to rows in image data
	unsigned char* data = (unsigned char*)Image->lock();
	for (u32 i=0; i<Height; ++i)
	{
		RowPointers[i]=data;
		data += Image->getPitch();
	}

	// Read data using the library function that handles all transformations including interlacing
	png_read_image(png_ptr, RowPointers);

	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr,&info_ptr, 0); // Clean up memory

	return Image;
#else
	return 0;
#endif // _IRR_COMPILE_WITH_LIBPNG_
}



IImageLoader* createImageLoaderPNG()
{
	return new CImageLoaderPng();
}


}// end namespace irr
}//end namespace video

