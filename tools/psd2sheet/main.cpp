#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "png.h"
#include <stdint.h>
#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"
#include "cxxopts.hpp"
#include "vec2.h"
#include "rect.h"
#include "utils.h"
#include "libpsd.h"

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
		cxxopts::Options options(argv[0], "Sprite sheet generator");
		options
			.positional_help("[optional args]")
			.show_positional_help();

		bool apple = false;

		options
			.allow_unrecognised_options()
			.add_options()
			("s,sheet", "Make a sheet, otherwise output separate PNG files for each layer", cxxopts::value<bool>(args.sheet))
			("h,nohidden", "Skip hidden layers", cxxopts::value<bool>(args.skipHiddenLayers))
			("r,rotate", "Rotate sprite and generate images", cxxopts::value<bool>(args.rotate))
			("n,nearest", "Use nearest interpolation when generating rotated images", cxxopts::value<bool>(args.neareast))
			("x,startangle", "Start angle (deg)", cxxopts::value<i32>(args.startAngle))
			("y,endangle", "End angle (deg)", cxxopts::value<i32>(args.endAngle))
			("a,stepangle", "Step angle (deg)", cxxopts::value<i32>(args.stepAngle))
			("f, file", "PSD filename", cxxopts::value<std::string>(), "<filename>")
			("o, outfile", "Out filename for the PNG sheet", cxxopts::value<std::string>(), "<outfilename>")
			;

		options.parse_positional({ "file", "outfile", "positional" });

		auto result = options.parse(argc, argv);

		if (result.count("help"))
		{
			std::cout << options.help({ "" }) << std::endl;
			exit(0);
		}

		args.filename = result["file"].as<std::string>();
		args.outFilename = result["outfile"].as<std::string>();

		return result;

	}
	catch (const cxxopts::OptionException & e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}
}

struct LayerImage
{
	u32* pixels = nullptr;
	u32* rotatedPixels = nullptr;
	Rect rect;
	Rect rotatedRect;
	Vec2 pivot;
};

int main(int argc, char *argv[])
{
	auto argResult = parseArgs(argc, argv);
	printf("Loading %s\n", args.filename.c_str());
	psd_context* context = NULL;
	psd_status status;

	// parse the psd file to psd_context
	status = psd_image_load(&context, (psd_char*)args.filename.c_str());
	printf("Layer count: %d\n", context->layer_count);
	std::vector<LayerImage*> layerImages;

	auto imageWidth = context->width;
	auto imageHeight = context->height;
	Rect maxRotateBounds = {0, 0, (f32)imageWidth, (f32)imageHeight};

	if (!args.rotate)
	{
		for (int i = 0; i < context->layer_count; ++i) // iterate over all layers (if desired)
		{
			uint32_t* imgdata = context->layer_records[i].image_data;
			int width = context->layer_records[i].width;
			int height = context->layer_records[i].height;
			int offset_x = context->layer_records[i].left;
			int offset_y = context->layer_records[i].top;

			LayerImage* limg = new LayerImage;
			limg->rect.set(offset_x, offset_y, width, height);
			limg->pixels = imgdata;
			limg->rotatedRect = limg->rect;
			limg->rotatedPixels = imgdata;
			layerImages.push_back(limg);
		}
	}
	else
	for (f32 angle = args.startAngle; angle <= args.endAngle; angle += args.stepAngle)
	for (int i = 0; i < context->layer_count; ++i) // iterate over all layers (if desired)
	{
		uint32_t* imgdata = context->layer_records[i].image_data;
		int width = context->layer_records[i].width;
		int height = context->layer_records[i].height;
		int offset_x = context->layer_records[i].left;
		int offset_y = context->layer_records[i].top;

		printf("Rotating %d %f\n", i, angle);
		LayerImage* limg = new LayerImage;

		limg->rect.set(0, 0, width, height);
		limg->pixels = imgdata;

		// compute pivot from center of canvas to layer center
		Rect rcLayer = { (f32)offset_x, (f32)offset_y, (f32)width, (f32)height };
		limg->pivot = rcLayer.center() - Vec2((f32)imageWidth / 2, (f32)imageHeight / 2);

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
		limg->pivot.rotateAround(Vec2(), angleRad);

		limg->rotatedRect.set(center.x, center.y, 0, 0);
		limg->rotatedRect.add(v0);
		limg->rotatedRect.add(v1);
		limg->rotatedRect.add(v2);
		limg->rotatedRect.add(v3);
		limg->rotatedRect.width = ceilf(limg->rotatedRect.width);
		limg->rotatedRect.height = ceilf(limg->rotatedRect.height);

		u32 imgSize = (u32)limg->rotatedRect.width * limg->rotatedRect.height;
		u32 imgSizeBytes = imgSize * 4;
		u32* rotatedImage = new u32[imgSize];
		memset(rotatedImage, 0, imgSizeBytes);
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
							dst[3] = i00[3];
						}
						else
						{
							forceNearest = true;
						}
					}
				}

				if ((args.neareast || forceNearest) && srcx0 >= 0 && srcx0 < limg->rect.width
					&& srcy0 >= 0 && srcy0 < limg->rect.height)
				{
					srcx0 = round(srcx0);
					srcy0 = round(srcy0);
					u8* dst = (u8*)&limg->rotatedPixels[y * (u32)limg->rotatedRect.width + x];
					u8* src = (u8*)&limg->pixels[(u32)(srcy0 * limg->rect.width + srcx0)];
					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2];
					dst[3] = src[3];
				}
			}
		}
		maxRotateBounds.add(v0);
		maxRotateBounds.add(v1);
		maxRotateBounds.add(v2);
		maxRotateBounds.add(v3);
		layerImages.push_back(limg);
	}

	maxRotateBounds.x = ceilf(maxRotateBounds.x);
	maxRotateBounds.y = ceilf(maxRotateBounds.y);
	maxRotateBounds.width = ceilf(maxRotateBounds.width);
	maxRotateBounds.height = ceilf(maxRotateBounds.height);

	// compute sheet rows/cols
	u32 sheetGridSize = ceilf(sqrtf(layerImages.size()));
	printf("Grid sheet %d %f %f\n", sheetGridSize);
	u32 sheetWidth = sheetGridSize * maxRotateBounds.width;
	u32 sheetHeight = sheetGridSize * maxRotateBounds.height;
	u32 sheetImageSize = sheetWidth * sheetHeight;
	u32 sheetImageSizeBytes = sheetWidth * sheetHeight * 4;
	u32* imagedata = new u32[sheetImageSize];

	printf("Final image sheet grid:%d sprite: %dx%d sheet:%dx%d\n",
		sheetGridSize, (u32)maxRotateBounds.width, (u32)maxRotateBounds.height, sheetWidth, sheetHeight);

	memset(imagedata, 0, sheetImageSize * 4);

    int sheetCol = 0;
    int sheetRow = 0;

    for (int i = 0; i < layerImages.size(); ++i) // iterate over all layers (if desired)
    {
		if (args.rotate)
		{
			// compute new center
			layerImages[i]->pivot.x += maxRotateBounds.width / 2;
			layerImages[i]->pivot.y += maxRotateBounds.height / 2;
		}
		else
		{
			// compute new center
			layerImages[i]->pivot.x = maxRotateBounds.width / 2;
			layerImages[i]->pivot.y = maxRotateBounds.height / 2;
		}

		for (int y = 0; y < (u32)layerImages[i]->rotatedRect.height; ++y)
        {
            for (int x = 0; x < (u32)layerImages[i]->rotatedRect.width; ++x)
            {
				u32* src = args.rotate ? layerImages[i]->rotatedPixels : layerImages[i]->pixels;
				u8* pixel = (u8*)&(src[y * (u32)layerImages[i]->rotatedRect.width + x]);

				i32 dstX;
				i32 dstY;

				if (args.rotate)
				{
					// destination is current sheet cell
					// starting at the center of it
					// with pivot added
					dstX = x + layerImages[i]->pivot.x + maxRotateBounds.width * sheetCol - layerImages[i]->rotatedRect.width / 2;
					dstY = y + layerImages[i]->pivot.y + maxRotateBounds.height * sheetRow - layerImages[i]->rotatedRect.height / 2;
				}
				else
				{
					dstX = x + layerImages[i]->rect.x + maxRotateBounds.width * sheetCol;
					dstY = y + layerImages[i]->rect.y + maxRotateBounds.height * sheetRow;
				}

				if (dstX >= 0 && dstY >= 0 && dstX < sheetWidth && dstY < sheetHeight)
				{
					u8* npixel = (u8*)&(imagedata[dstY * sheetWidth	+ dstX]);
					npixel[0] = pixel[2];
					npixel[1] = pixel[1];
					npixel[2] = pixel[0];
					npixel[3] = pixel[3];
				}
			}
        }

        sheetCol++;

        if (sheetCol >= sheetGridSize)
        {
            sheetCol = 0;
            sheetRow++;
        }
    }
    
    write_png(args.outFilename.c_str(), sheetWidth, sheetHeight, (u32*)imagedata);
    delete []imagedata;
	psd_image_free(context);

    return EXIT_SUCCESS;
}
