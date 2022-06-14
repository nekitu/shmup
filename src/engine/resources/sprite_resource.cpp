#include "sprite_resource.h"
#include <stb_image.h>
#include "image_atlas.h"
#include "game.h"
#include "graphics.h"
#include <chrono>
#include <filesystem>

namespace engine
{
AtlasImage* SpriteResource::loadImage(const std::string& path)
{
	if (!atlas) atlas = Game::instance->graphics->atlas;

	auto img = atlas->loadImageToAtlas(path, &paletteInfo);

	return img;
}

Rect SpriteResource::getFrameUvRect(u32 frame)
{
	Rect rc;

	if (frame >= frameCount)
	{
		LOG_ERROR("Frame out of bounds, frame: {0} max: {1}", frame, frameCount);
	}

	u32 col = frame % columns;
	u32 row = frame / columns;

	if (image->rotated)
	{
		rc.x = image->uvRect.x + uvFrameWidth * (f32)row;
		rc.y = image->uvRect.y + uvFrameHeight * (f32)(columns - col - 1);
		rc.width = uvFrameWidth;
		rc.height = uvFrameHeight;
	}
	else
	{
		rc.x = image->uvRect.x + uvFrameWidth * (f32)col;
		rc.y = image->uvRect.y + uvFrameHeight * (f32)row;
		rc.width = uvFrameWidth;
		rc.height = uvFrameHeight;
	}

	return rc;
}

Rect SpriteResource::getFramePixelRect(u32 frame)
{
	Rect rc;

	u32 col = frame % columns;
	u32 row = frame / columns;

	if (image->rotated)
	{
		rc.x = image->rect.x + frameWidth * row;
		rc.y = image->rect.y + frameHeight * col;
		rc.width = frameWidth;
		rc.height = frameHeight;
	}
	else
	{
		rc.x = image->rect.x + frameWidth * col;
		rc.y = image->rect.y + frameHeight * row;
		rc.width = frameWidth;
		rc.height = frameHeight;
	}

	return rc;
}

Rect SpriteResource::getSheetFramePixelRect(u32 frame)
{
	Rect rc;

	u32 col = frame % columns;
	u32 row = frame / columns;

	rc.x = floorf(frameWidth * col);
	rc.y = floorf(frameHeight * row);
	rc.width = floorf(frameWidth);
	rc.height = floorf(frameHeight);

	return rc;
}

bool SpriteResource::load(Json::Value& json)
{
	auto isPalettedTga = json.get("isPaletted", false).asBool();
	std::string imagePath;

	if (!isPalettedTga)
		imagePath = path + ".png";
	else
		imagePath = path + ".tga";

	// check if there are asset compile params
	if (!Game::instance->prebakedAtlas && json.isMember("assetCompiler"))
	{
		auto asset = json.get("assetCompiler", Json::Value()).get("asset", "").asString();

		bool mustCompile = false;
		auto imgExists = std::filesystem::exists(Game::makeFullDataPath(imagePath));
		auto assetExists = std::filesystem::exists(asset);

		std::filesystem::file_time_type timeAsset, timeData;
		timeAsset = std::filesystem::last_write_time(asset);

		if (assetExists)
		{
			if (imgExists)
			{
				timeData = std::filesystem::last_write_time(Game::makeFullDataPath(imagePath));

				if (timeAsset != timeData)
				{
					mustCompile = true;
				}
			}
			else
			{
				mustCompile = true;
			}
		}

		if (mustCompile)
		{
			LOG_INFO("Asset {} changed, recompiling...", asset);
			system((std::string("psd2sheet ") + asset + " " + Game::makeFullDataPath(imagePath)).c_str());
			std::filesystem::last_write_time(Game::makeFullDataPath(imagePath), timeAsset);
		}
	}

	frameWidth = json.get("frameWidth", Json::Value(0)).asInt();
	frameHeight = json.get("frameHeight", Json::Value(0)).asInt();
	color.parse(json.get("color", color.toString()).asString());
	auto colMode = json.get("colorMode", "Add").asString();

	image = loadImage(imagePath);

	if (image)
	{
		if (!frameWidth) frameWidth = image->width;
		if (!frameHeight) frameHeight = image->height;
	}

	assert(image);

	columns = image->width / frameWidth;
	rows = image->height / frameHeight;
	frameCount = rows * columns;

	if (colMode == "Add") colorMode = ColorMode::Add;
	if (colMode == "Sub") colorMode = ColorMode::Sub;
	if (colMode == "Mul") colorMode = ColorMode::Mul;

	Json::Value rotAnimsJson = json.get("rotationAnims", Json::Value());

	if (rotAnimsJson.isObject())
	{
		rotationAnimPrefix = rotAnimsJson.get("prefix", rotationAnimPrefix).asString();
		auto frmCount = rotAnimsJson.get("frameCount", 0).asInt();
		rotationAnimCount = rotAnimsJson.get("count", 0).asInt();
		auto fps = rotAnimsJson.get("fps", Json::Value(0)).asInt();
		auto repeatCount = rotAnimsJson.get("repeat", Json::Value(0)).asInt();
		std::string type = rotAnimsJson.get("type", Json::Value("normal")).asString();
		SpriteFrameAnimation::Type animType = SpriteFrameAnimation::Type::Normal;

		if (type == "Normal") animType = SpriteFrameAnimation::Type::Normal;
		if (type == "Reversed") animType = SpriteFrameAnimation::Type::Reversed;
		if (type == "PingPong") animType = SpriteFrameAnimation::Type::PingPong;

		for (size_t i = 0; i < rotationAnimCount; i++)
		{
			SpriteFrameAnimation* anim = new SpriteFrameAnimation();

			anim->frameCount = frmCount ? frmCount : frameCount;
			anim->framesPerSecond = fps;
			anim->name = rotationAnimPrefix + std::to_string(i);
			anim->repeatCount = repeatCount;
			anim->startFrame = i * frmCount;
			anim->type = animType;
			frameAnimations[anim->name] = anim;
		}
	}

	Json::Value animationsJson = json.get("animations", Json::Value());
	auto animNames = animationsJson.getMemberNames();

	for (auto& animName : animNames)
	{
		auto animJson = animationsJson.get(animName, Json::Value());
		SpriteFrameAnimation* anim = new SpriteFrameAnimation();

		anim->name = animName;
		anim->startFrame = animJson.get("start", Json::Value(0)).asInt();
		anim->frameCount = animJson.get("frameCount", Json::Value(0)).asInt();
		if (!anim->frameCount) anim->frameCount = frameCount;
		anim->framesPerSecond = animJson.get("fps", Json::Value(0)).asInt();
		anim->repeatCount = animJson.get("repeat", Json::Value(0)).asInt();
		frameAnimations[animName] = anim;

		std::string type = animJson.get("type", Json::Value("normal")).asString();
		if (type == "Normal") anim->type = SpriteFrameAnimation::Type::Normal;
		if (type == "Reversed") anim->type = SpriteFrameAnimation::Type::Reversed;
		if (type == "PingPong") anim->type = SpriteFrameAnimation::Type::PingPong;
	}

	if (isPalettedTga)
	{
		paletteInfo.paletteSlot = Game::instance->graphics->allocPaletteSlot();
		paletteInfo.transparentColorIndex = json.get("transparentColorIndex", ~0).asInt();

		if (paletteInfo.transparentColorIndex == ~0)
		{
			auto transparentColor = json.get("transparentColor", "1 0 1 1").asString();
			Color color;
			u32 i = 0;
			color.parse(transparentColor);
			u32 color32 = color.getRgba();

			for (auto& c : paletteInfo.colors)
			{
				if (color32 == c)
				{
					paletteInfo.transparentColorIndex = i;
					break;
				}

				i++;
			}
		}
	}

	return true;
}

void SpriteResource::computeParamsAfterAtlasGeneration()
{
	if (image->rotated)
	{
		uvFrameWidth = image->uvRect.width / (f32)(image->rect.height / frameHeight);
		uvFrameHeight = image->uvRect.height / (f32)(image->rect.width / frameWidth);
	}
	else
	{
		uvFrameWidth = image->uvRect.width / (f32)(image->rect.width / frameWidth);
		uvFrameHeight = image->uvRect.height / (f32)(image->rect.height / frameHeight);
	}

	columns = image->width / frameWidth;
	rows = image->height / frameHeight;
	frameCount = rows * columns;
}

void SpriteResource::unload()
{
	image = nullptr;
	atlas = nullptr;
	frameCount = 0;
	frameWidth = 0;
	frameHeight = 0;
	uvFrameWidth = 0;
	uvFrameHeight = 0;
	rows = columns = 0;
	color = Color::black;
	colorMode = ColorMode::Add;

	for (auto& frmAnim : frameAnimations)
	{
		delete frmAnim.second;
	}

	frameAnimations.clear();

	rotationAnimPrefix = "r";
	rotationAnimCount = 0;
}

}
