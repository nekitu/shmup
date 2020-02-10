/* Initial source code: psdslayer
 * Copyright 2009 Jetro Lauha
 *
 * This is simply the example code for using DrPetter's PSDImage class
 * combined with some options and stb_image + zlib + libpng for saving
 * each layer as a separate image file.
 *
 * My home page / blog URL:
 *   http://jet.ro
 *
 * Original URL for PSDImage release:
 *   http://www.ludumdare.com/compo/2008/04/12/psdimage-class/
 * URL for stb_image:
 *   http://nothings.org/stb_image.c
 *
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * $Id$
 * $Revision$
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "psdimage.h"
#include "png.h"
#include <stdint.h>
#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"
#include "cxxopts.hpp"
#include "vec2.h"
#include "rect.h"
#include "utils.h"

using namespace engine;

template <typename T>
T clamp(T value, T lower_bound, T upper_bound) {
	return std::min(std::max(value, lower_bound), upper_bound);
}
void png_error_fn(png_structp png_ptr, png_const_charp msg)
{
    fprintf(stderr, "libpng ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void png_warn_fn(png_structp png_ptr, png_const_charp msg)
{
    fprintf(stderr, "libpng warning: %s\n", msg);
}

bool write_png(const char *resultFile, int width, int height, unsigned int *imagedata)
{
    FILE *fp = fopen(resultFile, "wb");
    if (fp == 0)
        return false;

    png_structp png_ptr = 0;
    png_infop info_ptr = 0;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, png_error_fn, png_warn_fn);
    if (png_ptr == 0)
    {
        fclose(fp);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == 0)
    {
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    //png_set_bgr(png_ptr); // little-endian

    for (int y = 0; y < height; ++y)
    {
        png_write_row(png_ptr, (png_bytep)imagedata);
        imagedata += width;
    }

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);

    return true;
}

struct Args
{
	std::string filename = "";
	std::string outFilename = "";
	bool sheet = true;
	bool skipHiddenLayers = false;
	bool rotate = false;
	int startAngle = 0;
	int endAngle = 360;
	int stepAngle = 15;
	bool neareast = false;
} args;

cxxopts::ParseResult parseArgs(int argc, char* argv[])
{
	try
	{
		cxxopts::Options options(argv[0], " - sprite sheet generator");
		options
			.positional_help("[optional args]")
			.show_positional_help();

		bool apple = false;

		options
			.allow_unrecognised_options()
			.add_options()
			("s,sheet", "make a sheet", cxxopts::value<bool>(args.sheet))
			("h,nohidden", "skip hidden layers", cxxopts::value<bool>(args.skipHiddenLayers))
			("r,rotate", "rotate sprite and generate images", cxxopts::value<bool>(args.rotate))
			("n,nearest", "use nearest interpolation", cxxopts::value<bool>(args.neareast))
			("x,startangle", "start angle (deg)", cxxopts::value<i32>(args.startAngle))
			("y,endangle", "end angle (deg)", cxxopts::value<i32>(args.endAngle))
			("a,stepangle", "step angle (deg)", cxxopts::value<i32>(args.stepAngle))
			("f, file", "PSD filename", cxxopts::value<std::string>(), "Filename")
			("o, outfile", "Out filename for the PNG sheet", cxxopts::value<std::string>(), "Out filename")
			;

		options.parse_positional({ "file", "outfile", "positional" });

		auto result = options.parse(argc, argv);

		if (result.count("help"))
		{
			std::cout << options.help({ "", "Group" }) << std::endl;
			exit(0);
		}

		args.filename = result["file"].as<std::string>();
		args.outFilename = result["outfile"].as<std::string>();

		return result;

	}
	catch (const cxxopts::OptionException & e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		//exit(1);
	}
}

struct LayerImage
{
	u32* pixels = nullptr;
	u32* rotatedPixels = nullptr;
	Rect rect;
	Rect rotatedRect;
};

int main(int argc, char *argv[])
{
	auto argResult = parseArgs(argc, argv);

    PSDImage *img = new PSDImage();

	printf("Loading %s\n", args.filename.c_str());

    if (!img->Load(args.filename.c_str())) // load image file and store image data for all layers
    {
        printf("Error loading %s\n", args.filename.c_str());
        return EXIT_FAILURE;
    }

	std::vector<LayerImage*> layerImages;
	Rect maxRotateBounds = {0, 0, (f32)img->GetHeader().Columns, (f32)img->GetHeader().Rows};

	for (int i = 0; i < img->NumLayers(); ++i) // iterate over all layers (if desired)
	{
		// dimensions of this layer (outer boundaries of image content)
		int width;
		int height;
		// upper-left coordinate of bounding box, use this to place the layer accurately within the image
		int offset_x;
		int offset_y;
		uint32_t* imgdata = (uint32_t*)img->GetLayerData(i, width, height, offset_x, offset_y); // get pointer to layer image, and info
		
		if (!args.rotate)
		{
			LayerImage* limg = new LayerImage;
			limg->rect.set(offset_x, offset_y, width, height);
			limg->pixels = imgdata;
			limg->rotatedRect = limg->rect;
			limg->rotatedPixels = imgdata;
			layerImages.push_back(limg);
		}
		else
		{
			for (f32 angle = args.startAngle; angle <= args.endAngle; angle += args.stepAngle)
			{
				printf("Rotating %d %f\n", i, angle);
				LayerImage* limg = new LayerImage;

				limg->rect.set(0, 0, width, height);
				limg->pixels = imgdata;

				Vec2 v0(limg->rect.topLeft());
				Vec2 v1(limg->rect.topRight());
				Vec2 v2(limg->rect.bottomRight());
				Vec2 v3(limg->rect.bottomLeft());

				Vec2 center = limg->rect.center();
				auto angleRad = deg2rad(angle);

				v0.rotateAround(center, angleRad);
				v1.rotateAround(center, angleRad);
				v2.rotateAround(center, angleRad);
				v3.rotateAround(center, angleRad);

				limg->rotatedRect.set(center.x, center.y, 0, 0);
				limg->rotatedRect.add(v0);
				limg->rotatedRect.add(v1);
				limg->rotatedRect.add(v2);
				limg->rotatedRect.add(v3);
				limg->rotatedRect.width = ceilf(limg->rotatedRect.width);
				limg->rotatedRect.height = ceilf(limg->rotatedRect.height);

				printf("Rect %f %f %f %f\n", limg->rect.x, limg->rect.y, limg->rect.width, limg->rect.height);
				printf("RotatedRect %f %f %f %f\n", limg->rotatedRect.x, limg->rotatedRect.y, limg->rotatedRect.width, limg->rotatedRect.height);

				// rotate pixels into new buffer
				u32* rotatedImage = new u32[(u32)limg->rotatedRect.width * (u32)limg->rotatedRect.height];
				memset(rotatedImage, 0, (u32)limg->rotatedRect.width * (u32)limg->rotatedRect.height * 4);
				f32 sina = sinf(angleRad);
				f32 cosa = cosf(angleRad);

				limg->rotatedPixels = rotatedImage;

				for (int y = 0; y < limg->rotatedRect.height; y++)
				{
					for (int x = 0; x < limg->rotatedRect.width; x++)
					{
						bool forceNearest = false;
						f32 offsX = limg->rotatedRect.x - (u32)limg->rotatedRect.center().x;
						f32 offsY = limg->rotatedRect.y - (u32)limg->rotatedRect.center().y;
						f32 srcx0 = (x + offsX) * cosa + (y + offsY) * sina + limg->rotatedRect.center().x;
						f32 srcy0 = (y + offsY) * cosa - (x + offsX) * sina + limg->rotatedRect.center().y;

						if (!args.neareast)
						{
							f32 srcx1 = floorf(srcx0);
							f32 srcy1 = floorf(srcy0);
							f32 srcx2 = srcx1 + 1.0f;
							f32 srcy2 = srcy1 + 1.0f;

							if ((srcx1 >= 0) && (srcy1 >= 0) && (srcx2 < limg->rect.width) && (srcy2 < limg->rect.height))
							{
								u8* i00 = (u8*)&imgdata[(u32)srcy0 * (u32)limg->rect.width + (u32)srcx0];
								u8* i11 = (u8*)&imgdata[(u32)srcy1 * (u32)limg->rect.width + (u32)srcx1];
								u8* i21 = (u8*)&imgdata[(u32)srcy2 * (u32)limg->rect.width + (u32)srcx1];
								u8* i12 = (u8*)&imgdata[(u32)srcy1 * (u32)limg->rect.width + (u32)srcx2];
								u8* i22 = (u8*)&imgdata[(u32)srcy2 * (u32)limg->rect.width + (u32)srcx2];

								// we only interpolate when inside image, not on edge, due to alpha artifacts
								if (i11[3] == 255 && i12[3] == 255 && i22[3] == 255 && i21[3] == 255)
								{
									f32 dx = srcx2 - srcx0;
									f32 dy = srcy2 - srcy0;
									u8* dst = (u8*)&limg->rotatedPixels[y * (u32)limg->rotatedRect.width + x];
									dst[0] = i11[0] * dx * dy + i21[0] * dx * (1.0f - dy) + i12[0] * (1.0f - dx) * dy + i22[0] * (1.0f - dx) * (1.0f - dy);
									dst[1] = i11[1] * dx * dy + i21[1] * dx * (1.0f - dy) + i12[1] * (1.0f - dx) * dy + i22[1] * (1.0f - dx) * (1.0f - dy);
									dst[2] = i11[2] * dx * dy + i21[2] * dx * (1.0f - dy) + i12[2] * (1.0f - dx) * dy + i22[2] * (1.0f - dx) * (1.0f - dy);
									dst[3] = i00[3]; i11[3] * dx* dy + i21[3] * dx * (1.0f - dy) + i12[3] * (1.0f - dx) * dy + i22[3] * (1.0f - dx) * (1.0f - dy);
								}
								else
								{
									forceNearest = true;
								}
							}
						}
						else
						if ((args.neareast || forceNearest) && srcx0 >= 0 && srcx0 < limg->rect.width
							&& srcy0 >= 0 && srcy0 < limg->rect.height)
						{
							srcx0 = round(srcx0);
							srcy0 = round(srcy0);
							limg->rotatedPixels[y * (u32)limg->rotatedRect.width + x] = imgdata[(u32)srcy0 * (u32)limg->rect.width + (u32)srcx0];
						}
					}
				}

				if (limg->rotatedRect.width > maxRotateBounds.width) maxRotateBounds.width = limg->rotatedRect.width;
				if (limg->rotatedRect.height > maxRotateBounds.height) maxRotateBounds.height = limg->rotatedRect.height;

				layerImages.push_back(limg);
			}
		}
	}

	// compute sheet rows/cols
	u32 sheetGridSize = ceilf(sqrtf(layerImages.size()));
	printf("Grid sheet %d\n", sheetGridSize);
	u32 sheetWidth = (sheetGridSize * (u32)maxRotateBounds.width);
	u32 sheetHeight = (sheetGridSize * (u32)maxRotateBounds.height);
	u32 sheetImageSize = sheetWidth * sheetHeight;
	u32 sheetImageSizeBytes = sheetWidth * sheetHeight * 4;
	u32* imagedata = new u32[sheetImageSize];

	printf("Final image sheet grid:%d sprite: %dx%d sheet:%dx%d\n",
		sheetGridSize, (u32)maxRotateBounds.width, (u32)maxRotateBounds.height, sheetWidth, sheetHeight);

    for (auto i = 0; i < sheetImageSize; i++)
    {
        imagedata[i] = 0;
    }

    int sheetCol = 0;
    int sheetRow = 0;

    for (int i = 0; i < layerImages.size(); ++i) // iterate over all layers (if desired)
    {
		layerImages[i]->rotatedRect.x = maxRotateBounds.width / 2 - layerImages[i]->rotatedRect.width / 2;
		layerImages[i]->rotatedRect.y = maxRotateBounds.height / 2 - layerImages[i]->rotatedRect.height / 2;

        for (int y = 0; y < (u32)layerImages[i]->rotatedRect.height; ++y)
        {
            for (int x = 0; x < (u32)layerImages[i]->rotatedRect.width; ++x)
            {
                char* pixel = (char*)&(layerImages[i]->rotatedPixels[y* (u32)layerImages[i]->rotatedRect.width + x]);
                char* npixel= (char*)&(imagedata[(y + (u32)layerImages[i]->rotatedRect.y + sheetRow * (u32)maxRotateBounds.height) * sheetWidth
					+ x + (u32)layerImages[i]->rotatedRect.x + (u32)maxRotateBounds.width * sheetCol]);
                npixel[0] = pixel[2]; //R //B
                npixel[1] = pixel[1];// pixel[1];//G //G
                npixel[2] = pixel[0];// pixel[2];//B //R
                npixel[3] = pixel[3]; //A
            }
        }

        sheetCol++;

        if (sheetCol >= sheetGridSize)
        {
            sheetCol = 0;
            sheetRow++;
        }
    }
    
    write_png(args.outFilename.c_str(), sheetWidth, sheetHeight, (unsigned int *)imagedata);
    delete []imagedata;
    delete img; // note that this will free all the image data, so any pointers you might have to layers will be invalid

    return EXIT_SUCCESS;
}
