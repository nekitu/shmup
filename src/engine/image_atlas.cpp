#include <string.h>
#include "image_atlas.h"
#include "utils.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image/stb_image_write.c"
#include "texture_array.h"
#include <stb_image.h>
#include <tga.hpp>
#include <json/writer.h>
#include "game.h"
#include <filesystem>

namespace engine
{
static u32 atlasId = 0;

u32 AtlasImage::getPixelOffsetInsideAtlas(u32 x, u32 y)
{
	if (!rotated)
		return (rect.y + y) * atlasTexture->textureArray->width + rect.x + x;

	return (rect.y + x) * atlasTexture->textureArray->height + rect.x + y;
}

Rgba32 AtlasImage::getPixel(u32 x, u32 y)
{
	if (!atlasTexture || !atlasTexture->textureImage)
		return 0;

	return atlasTexture->textureImage[getPixelOffsetInsideAtlas(x, y)];
}

u8* AtlasImage::getPixelAddr(u32 x, u32 y)
{
	if (!atlasTexture || !atlasTexture->textureImage)
		return nullptr;

	return (u8*)(atlasTexture->textureImage + getPixelOffsetInsideAtlas(x, y));
}

void AtlasImage::setPixel(u32 x, u32 y, Rgba32 color)
{
	auto addr = getPixelAddr(x, y);
	if (addr) (Rgba32&)addr = color;
}

ImageAtlas::ImageAtlas(u32 textureWidth, u32 textureHeight)
{
	id = atlasId++;
	create(textureWidth, textureHeight);
	addWhiteImage();
}

ImageAtlas::~ImageAtlas()
{
	for (auto& at : atlasTextures)
	{
		delete[] at->textureImage;
	}

	for (auto& img : images)
	{
		delete[] img.second->cachedImageData;
		delete img.second;
	}

	for (auto& pps : pendingPackImages)
	{
		delete[] pps.imageData;
	}

	delete textureArray;
}

void ImageAtlas::create(u32 textureWidth, u32 textureHeight)
{
	width = textureWidth;
	height = textureHeight;

	for (auto& at : atlasTextures)
	{
		delete [] at->textureImage;
		delete at;
	}

	atlasTextures.clear();
	clearImages();
	delete textureArray;
	textureArray = new TextureArray();
	textureArray->resize(1, textureWidth, textureHeight);
}

AtlasImage* ImageAtlas::getImageById(AtlasImageId id) const
{
	auto iter = images.find(id);

	if (iter == images.end())
	{
		return nullptr;
	}

	return iter->second;
}

AtlasImage* ImageAtlas::addImage(const std::string& path, const Rgba32* imageData, u32 width, u32 height, bool addBleedOut)
{
	return addImageInternal(path, lastImageId++, imageData, width, height, addBleedOut);
}

AtlasImage* ImageAtlas::addImageInternal(const std::string& path, AtlasImageId imgId, const Rgba32* imageData, u32 imageWidth, u32 imageHeight, bool addBleedOut)
{
	if (prebaked)
	{
		auto iter = pathImageLUT.find(path);

		if (iter == pathImageLUT.end())
		{
			LOG_ERROR("Atlas: Cannot find image with path {} in prebaked atlas", path);
			return nullptr;
		}

		return iter->second;
	}

	PackImageData psd;

	u32 imageSize = imageWidth * imageHeight;
	psd.path = path;
	psd.imageData = new Rgba32[imageSize];
	memcpy(psd.imageData, imageData, imageSize * 4);
	psd.id = imgId;
	psd.width = imageWidth;
	psd.height = imageHeight;
	psd.packedRect.set(0, 0, 0, 0);
	psd.atlas = this;
	psd.bleedOut = addBleedOut;
	pendingPackImages.push_back(psd);

	AtlasImage* image = new AtlasImage();

	image->id = psd.id;
	image->width = imageWidth;
	image->height = imageHeight;
	image->atlas = this;
	image->path = path;
	images.insert(std::make_pair(psd.id, image));
	pathImageLUT[path] = image;

	return image;
}

void ImageAtlas::updateImageData(AtlasImageId imgId, const Rgba32* imageData, u32 width, u32 height)
{
	assert(false);
	//TODO
}

void ImageAtlas::deleteImage(AtlasImage* image)
{
	auto iter = images.find(image->id);

	if (iter == images.end())
		return;

	delete[] image->cachedImageData;
	delete image;

	//TODO: we would need to recreate the atlas texture where this image was in
	images.erase(iter);
}

AtlasImage* ImageAtlas::addWhiteImage(u32 width)
{
	u32 whiteImageSize = width * width;
	Rgba32* whiteImageData = new Rgba32[whiteImageSize];

	memset(whiteImageData, 0xff, whiteImageSize * 4);
	whiteImage = addImage("white_image", whiteImageData, width, width, true);
	delete[] whiteImageData;

	return whiteImage;
}

bool ImageAtlas::pack(
	u32 spacing,
	const Color& bgColor,
	AtlasPackPolicy packPolicy)
{
	if (pendingPackImages.empty())
		return true;

	lastUsedBgColor = bgColor;
	lastUsedPolicy = packPolicy;
	lastUsedSpacing = spacing;

	u32 border2 = spacing * 2;
	::Rect packedRect;
	bool rotated = false;
	bool allTexturesDirty = false;
	std::vector<PackImageData> acceptedImages;

	while (!pendingPackImages.empty())
	{
		// search some place to put the image
		for (auto& atlasTex : atlasTextures)
		{
			switch (atlasTex->packPolicy)
			{
			case AtlasPackPolicy::Guillotine:
			{
				auto& guillotineBinPack = atlasTex->guillotineBinPack;
				auto iter = pendingPackImages.begin();

				while (iter != pendingPackImages.end())
				{
					auto& packImage = *iter;

					//TODO: if image is bigger than the atlas size, then resize or just skip
					if (packImage.width >= width || packImage.height >= height)
					{
						LOG_ERROR("Image '{}' is too big for the atlas, skipping", packImage.path);
						delete[] packImage.imageData;
						iter = pendingPackImages.erase(iter);
						continue;
					}

					packedRect = guillotineBinPack.Insert(
						packImage.width + border2,
						packImage.height + border2,
						true,
						GuillotineBinPack::FreeRectChoiceHeuristic::RectBestShortSideFit,
						GuillotineBinPack::GuillotineSplitHeuristic::SplitLongerAxis);

					if (packedRect.height <= 0)
					{
						++iter;
						continue;
					}

					packImage.packedRect.x = packedRect.x;
					packImage.packedRect.y = packedRect.y;
					packImage.packedRect.width = packedRect.width;
					packImage.packedRect.height = packedRect.height;
					packImage.atlas = this;
					packImage.atlasTexture = atlasTex;
					acceptedImages.push_back(packImage);
					iter = pendingPackImages.erase(iter);
				}

				break;
			}
			case AtlasPackPolicy::MaxRects:
			{
				MaxRectsBinPack& maxRectsBinPack = atlasTex->maxRectsBinPack;
				auto iter = pendingPackImages.begin();

				while (iter != pendingPackImages.end())
				{
					auto& packImage = *iter;

					packedRect = maxRectsBinPack.Insert(
						packImage.width + border2,
						packImage.height + border2,
						MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestAreaFit);

					if (packedRect.height <= 0)
					{
						++iter;
						continue;
					}

					packImage.packedRect.x = packedRect.x;
					packImage.packedRect.y = packedRect.y;
					packImage.packedRect.width = packedRect.width;
					packImage.packedRect.height = packedRect.height;
					packImage.atlas = this;
					packImage.atlasTexture = atlasTex;
					acceptedImages.push_back(packImage);
					iter = pendingPackImages.erase(iter);
				}

				break;
			}
			case AtlasPackPolicy::ShelfBin:
			{
				ShelfBinPack& shelfBinPack = atlasTex->shelfBinPack;
				auto iter = pendingPackImages.begin();

				while (iter != pendingPackImages.end())
				{
					auto& packImage = *iter;

					packedRect = shelfBinPack.Insert(
						packImage.width + border2,
						packImage.height + border2,
						ShelfBinPack::ShelfChoiceHeuristic::ShelfBestAreaFit);

					if (packedRect.height <= 0)
					{
						++iter;
						continue;
					}

					packImage.packedRect.x = packedRect.x;
					packImage.packedRect.y = packedRect.y;
					packImage.packedRect.width = packedRect.width;
					packImage.packedRect.height = packedRect.height;
					packImage.atlas = this;
					packImage.atlasTexture = atlasTex;
					acceptedImages.push_back(packImage);
					iter = pendingPackImages.erase(iter);
				}

				break;
			}
			case AtlasPackPolicy::Skyline:
			{
				SkylineBinPack& skylineBinPack = atlasTex->skylineBinPack;
				auto iter = pendingPackImages.begin();

				while (iter != pendingPackImages.end())
				{
					auto& packImage = *iter;

					packedRect = skylineBinPack.Insert(
						packImage.width + border2,
						packImage.height + border2,
						SkylineBinPack::LevelChoiceHeuristic::LevelMinWasteFit);

					if (packedRect.height <= 0)
					{
						++iter;
						continue;
					}

					packImage.packedRect.x = packedRect.x;
					packImage.packedRect.y = packedRect.y;
					packImage.packedRect.width = packedRect.width;
					packImage.packedRect.height = packedRect.height;
					packImage.atlas = this;
					packImage.atlasTexture = atlasTex;
					acceptedImages.push_back(packImage);
					iter = pendingPackImages.erase(iter);
				}

				break;
			}
			default:
				break;
			}
		}

		if (!pendingPackImages.empty())
		{
			AtlasTexture* newTexture = new AtlasTexture();

			newTexture->packPolicy = packPolicy;
			newTexture->textureImage = new Rgba32[width * height];
			memset(newTexture->textureImage, 0, width * height * sizeof(Rgba32));
			newTexture->textureIndex = atlasTextures.size();
			newTexture->textureArray = textureArray;

			switch (packPolicy)
			{
			case AtlasPackPolicy::Guillotine:
				newTexture->guillotineBinPack.Init(width, height);
				break;
			case AtlasPackPolicy::MaxRects:
				newTexture->maxRectsBinPack.Init(width, height);
				break;
			case AtlasPackPolicy::ShelfBin:
				newTexture->shelfBinPack.Init(width, height, useWasteMap);
				break;
			case AtlasPackPolicy::Skyline:
				newTexture->skylineBinPack.Init(width, height, useWasteMap);
				break;
			default:
				break;
			}

			atlasTextures.push_back(newTexture);

			// resize the texture array
			textureArray->resize(atlasTextures.size(), width, height);
			allTexturesDirty = true;
		}
	}

	// we have now the rects inside the atlas
	for (auto& packImage : acceptedImages)
	{
		auto image = images[packImage.id];

		assert(image);
		// take out the border from final image rect
		packImage.packedRect.x += spacing;
		packImage.packedRect.y += spacing;
		packImage.packedRect.width -= border2;
		packImage.packedRect.height -= border2;

		// if bleedOut, then limit/shrink the rect so we sample from within the image
		if (packImage.bleedOut)
		{
			const int bleedOutSize = 3;
			packImage.packedRect.x += bleedOutSize;
			packImage.packedRect.y += bleedOutSize;
			packImage.packedRect.width -= bleedOutSize * 2;
			packImage.packedRect.height -= bleedOutSize * 2;
		}

		// init image
		// if not prebaked, pass the ownership of the image data ptr
		// used for repacking the atlas at runtime again
		if (!prebaked)
		{
			image->cachedImageData = packImage.imageData;
		}

		image->bleedOut = packImage.bleedOut;
		image->width = packImage.width;
		image->height = packImage.height;
		image->atlasTexture = packImage.atlasTexture;
		assert(image->atlasTexture);
		image->rect = { (f32)packImage.packedRect.x, (f32)packImage.packedRect.y, (f32)packImage.width, (f32)packImage.height };
		image->rotated = packImage.width != packImage.packedRect.width;
		image->uvRect.set(
			(f32)packImage.packedRect.x / (f32)width,
			(f32)packImage.packedRect.y / (f32)height,
			(f32)packImage.packedRect.width / (f32)width,
			(f32)packImage.packedRect.height / (f32)height);

		// copy image to the atlas image buffer
		if (image->rotated)
		{
			// rotation is clockwise
			for (u32 y = 0; y < packImage.height; y++)
			{
				for (u32 x = 0; x < packImage.width; x++)
				{
					u32 destOffset =
						packImage.packedRect.x + y + (packImage.packedRect.y + x) * width;
					u32 srcOffset = y * packImage.width + (packImage.width - 1) - x;
					image->atlasTexture->textureImage[destOffset] = ((Rgba32*)packImage.imageData)[srcOffset];
				}
			}
		}
		else
			for (u32 y = 0; y < packImage.height; y++)
			{
				for (u32 x = 0; x < packImage.width; x++)
				{
					u32 destIndex =
						packImage.packedRect.x + x + (packImage.packedRect.y + y) * width;
					u32 srcIndex = x + y * packImage.width;
					image->atlasTexture->textureImage[destIndex] = ((Rgba32*)packImage.imageData)[srcIndex];
				}
			}

		image->atlasTexture->dirty = true;
	}

	for (auto& atlasTex : atlasTextures)
	{
		if (atlasTex->dirty || allTexturesDirty)
		{
			atlasTex->textureArray->updateLayerData(atlasTex->textureIndex, atlasTex->textureImage);
			atlasTex->dirty = false;
		}
	}

	assert(pendingPackImages.empty());

	return pendingPackImages.empty();
}

void ImageAtlas::repackImages()
{
	if (prebaked)
	{
		LOG_ERROR("Repacking atlas images is only allowed on 'prebakedAtlas' config var set to false");
		return;
	}

	deletePackerImages();

	for (auto& img : images)
	{
		PackImageData packImg;

		packImg.id = img.first;

		// pass over the data ptr, it will be passed again to the image
		if (img.second->cachedImageData)
			packImg.imageData = img.second->cachedImageData;

		packImg.width = img.second->width;
		packImg.height = img.second->height;
		packImg.packedRect.set(0, 0, 0, 0);
		packImg.atlas = this;
		packImg.bleedOut = img.second->bleedOut;
		pendingPackImages.push_back(packImg);
	}

	packWithLastUsedParams();
}

void ImageAtlas::deletePackerImages()
{
	// initialize the atlas textures
	for (auto& atlasTex : atlasTextures)
	{
		switch (atlasTex->packPolicy)
		{
		case AtlasPackPolicy::Guillotine:
			atlasTex->guillotineBinPack = GuillotineBinPack(width, height);
			break;
		case AtlasPackPolicy::MaxRects:
			atlasTex->maxRectsBinPack = MaxRectsBinPack(width, height);
			break;
		case AtlasPackPolicy::ShelfBin:
			atlasTex->shelfBinPack = ShelfBinPack(width, height, useWasteMap);
			break;
		case AtlasPackPolicy::Skyline:
			atlasTex->skylineBinPack = SkylineBinPack(width, height, useWasteMap);
			break;
		default:
			break;
		}

		// clear texture
		memset(atlasTex->textureImage, lastUsedBgColor.getRgba(), width * height * sizeof(Rgba32));
	}
}

void ImageAtlas::clearImages()
{
	deletePackerImages();

	for (auto& image : images)
	{
		delete [] image.second->cachedImageData;
		delete image.second;
	}

	for (auto& image : pendingPackImages)
	{
		delete [] image.imageData;
	}

	images.clear();
	pathImageLUT.clear();
	pendingPackImages.clear();
}

bool isPathTGA(const std::string& path)
{
	return strstr(path.c_str(), ".tga") || strstr(path.c_str(), ".TGA");
}

AtlasImage* ImageAtlas::loadImageToAtlas(const std::string& path, PaletteInfo* paletteInfo)
{
	int width = 0;
	int height = 0;
	int comp = 0;
	u8* data = 0;
	bool isTGA = isPathTGA(path);

	if (prebaked)
	{
		if (isTGA && paletteInfo)
		{
			*paletteInfo = tgaPalettes[path];
		}
	}
	else
	{
		if (isTGA)
		{
			tga::TGA tgaFile;

			if (tgaFile.Load(path))
			{
				width = tgaFile.GetWidth();
				height = tgaFile.GetHeight();
				data = tgaFile.GetData();
				auto format = tgaFile.GetFormat();
				auto imgSize = width * height;

				if (paletteInfo)
				{
					paletteInfo->isPaletted = tgaFile.GetIndexedData() != nullptr;
					paletteInfo->bitsPerPixel = tgaFile.GetBpp();
					paletteInfo->colors.resize(tgaFile.GetColorPaletteLength());

					auto pal = tgaFile.GetColorPalette();
					u8* color;

					// copy palette
					for (int i = 0; i < paletteInfo->colors.size(); i++)
					{
						color = &pal[i * 3];

						if (tgaFile.GetFormat() == tga::ImageFormat::RGB)
						{
							paletteInfo->colors[i] =
								packRGBA(color[2], color[1], color[0], 0xff);
						}
						else if (tgaFile.GetFormat() == tga::ImageFormat::RGBA)
						{
							paletteInfo->colors[i] = packRGBA(color[3], color[2], color[1], color[0]);
						}
					}

					// we only support 256 color palettes
					if (paletteInfo->isPaletted && paletteInfo->bitsPerPixel == 8)
					{
						u32* rgbaData = new u32[imgSize];
						u8 index = 0;

						for (int y = 0; y < height; y++)
						{
							for (int x = 0; x < width; x++)
							{
								auto offs = y * width + x;
								index = *(u8*)(tgaFile.GetIndexedData() + offs);
								u32 c = packRGBA(index, 0, 0, 255);
								auto offsFlipped = (height - 1 - y) * width + x;
								rgbaData[offsFlipped] = c;
							}
						}

						data = (u8*)rgbaData;
					}

					tgaPalettes[path] = *paletteInfo;
				}
			}
		}
		else
		{
			data = (u8*)stbi_load(path.c_str(), &width, &height, &comp, 4);
		}

		LOG_INFO("Loaded image: {0} {1}x{2}", path, width, height);

		if (!data)
			return nullptr;
	}

	auto img = addImage(path, (Rgba32*)data, width, height);

	delete[] data;

	return img;
}

bool ImageAtlas::save(const std::string& path)
{
	std::filesystem::create_directories(Game::instance->makeFullDataPath(path));

	int i = 0;

	Json::Value doc;
	Json::StyledWriter writer;

	doc["version"] = 1;
	doc["textureCount"] = atlasTextures.size();
	doc["textureWidth"] = width;
	doc["textureHeight"] = height;

	Json::Value imgJsonArray(Json::ValueType::arrayValue);

	for (auto& img : images)
	{
		Json::Value imgJson;

		imgJson["id"] = img.second->id;
		imgJson["path"] = img.second->path;
		imgJson["width"] = img.second->width;
		imgJson["height"] = img.second->height;
		imgJson["textureIndex"] = img.second->atlasTexture->textureIndex;
		imgJson["bleedOut"] = img.second->bleedOut;
		imgJson["rectX"] = img.second->rect.x;
		imgJson["rectY"] = img.second->rect.y;
		imgJson["rectW"] = img.second->rect.width;
		imgJson["rectH"] = img.second->rect.height;
		imgJson["rotated"] = img.second->rotated;
		imgJson["uvRectX"] = img.second->uvRect.x;
		imgJson["uvRectY"] = img.second->uvRect.y;
		imgJson["uvRectW"] = img.second->uvRect.width;
		imgJson["uvRectH"] = img.second->uvRect.height;

		if (isPathTGA(img.second->path))
		{
			auto& pal = tgaPalettes[img.second->path];

			imgJson["paletteBpp"] = pal.bitsPerPixel;
			imgJson["isPaletted"] = pal.isPaletted;
			imgJson["paletteSlot"] = pal.paletteSlot;
			imgJson["paletteTransparentColorIndex"] = pal.transparentColorIndex;

			Json::Value paletteColorsJson(Json::ValueType::arrayValue);

			for (auto j = 0; j < pal.colors.size(); j++)
			{
				paletteColorsJson.append(pal.colors[j]);
			}

			imgJson["palette"] = paletteColorsJson;
		}

		imgJsonArray.append(imgJson);
	}

	doc["images"] = imgJsonArray;

	for (auto& tex : atlasTextures)
	{
		auto pngFile = Game::instance->makeFullDataPath(path + "/page" + std::to_string(i++) + ".png");

		if (!stbi_write_png(pngFile.c_str(), tex->textureArray->width, tex->textureArray->height, 4, tex->textureImage, 0))
		{
			return false;
		}
	}

	auto str = writer.write(doc);
	writeTextFile(Game::instance->makeFullDataPath(path + "/atlas.json"), str);

	return true;
}

bool ImageAtlas::load(const std::string& path)
{
	prebaked = true;
	Json::Value doc;

	loadJson(Game::instance->makeFullDataPath(path + "/atlas.json"), doc);

	width = doc["textureWidth"].asInt();
	height = doc["textureHeight"].asInt();
	auto texCount = doc["textureCount"].asInt();
	create(width, height);
	textureArray->resize(texCount, width, height);

	for (u32 i = 0; i < texCount; i++)
	{
		AtlasTexture* tex = new AtlasTexture();

		tex->textureIndex = i;
		int w, h, comp;
		auto texData = stbi_load(Game::instance->makeFullDataPath(path + "/page" + std::to_string(i) + ".png").c_str(), &w, &h, &comp, 4);
		tex->textureImage = (Rgba32*)texData;
		tex->textureArray = textureArray;
		tex->textureArray->updateLayerData(i, (Rgba32*)texData);
		atlasTextures.push_back(tex);
	}

	auto imagesJsonArray = doc["images"];

	for (auto& imgJson : imagesJsonArray)
	{
		AtlasImage* img = new AtlasImage();

		img->atlas = this;
		img->atlasTexture = atlasTextures[imgJson["textureIndex"].asInt()];
		img->bleedOut = imgJson["bleedOut"].asBool();
		img->width = imgJson["width"].asInt();
		img->height = imgJson["height"].asInt();
		img->id = imgJson["id"].asInt();
		img->path = imgJson["path"].asString();
		img->rect.x = imgJson["rectX"].asInt();
		img->rect.y = imgJson["rectY"].asInt();
		img->rect.width = imgJson["rectW"].asInt();
		img->rect.height = imgJson["rectH"].asInt();
		img->rotated = imgJson["rotated"].asBool();
		img->uvRect.x = imgJson["uvRectX"].asFloat();
		img->uvRect.y = imgJson["uvRectY"].asFloat();
		img->uvRect.width = imgJson["uvRectW"].asFloat();
		img->uvRect.height = imgJson["uvRectH"].asFloat();

		if (isPathTGA(img->path))
		{
			auto& pal = tgaPalettes[img->path];

			pal.bitsPerPixel = imgJson["paletteBpp"].asInt();
			pal.isPaletted = imgJson["isPaletted"].asBool();
			pal.paletteSlot = imgJson["paletteSlot"].asInt();
			pal.transparentColorIndex = imgJson["paletteTransparentColorIndex"].asInt();

			Json::Value paletteColorsJson = imgJson["palette"];

			for (auto& col : paletteColorsJson)
			{
				pal.colors.push_back(col.asUInt());
			}
		}

		images.insert(std::make_pair(img->id, img));
		pathImageLUT[img->path] = img;
	}

	addWhiteImage();

	return true;
}

}
