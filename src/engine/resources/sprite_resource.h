#pragma once
#include "types.h"
#include <string>
#include "rect.h"
#include "resource.h"
#include "color.h"
#include <unordered_map>

namespace engine
{
enum class ColorMode
{
	Add,
	Sub,
	Mul
};

struct SpriteFrameAnimation
{
	enum class Type
	{
		Normal,
		Reversed,
		PingPong
	};

	std::string name;
	u32 startFrame = 0;
	u32 frameCount = 0;
	Type type = Type::Normal;
	u32 framesPerSecond = 1;
	u32 repeatCount = 0; /// zero means infinite
};

struct SpriteResource : Resource
{
	struct AtlasImage* image = nullptr;
	struct ImageAtlas* atlas = nullptr;
	
	u32 frameCount = 0;
	u32 frameWidth = 0;
	u32 frameHeight = 0;
	f32 uvFrameWidth = 0;
	f32 uvFrameHeight = 0;
	u32 rows = 0, columns = 0;
	Color color = Color::black;
	ColorMode colorMode = ColorMode::Add;
	std::unordered_map<std::string, SpriteFrameAnimation*> frameAnimations;
	std::string rotationAnimPrefix = "r";
	u32 rotationAnimCount = 0;

	struct AtlasImage* loadImage(const std::string& filename);
	Rect getFrameUvRect(u32 frame);
	Rect getFramePixelRect(u32 frame);
	Rect getSheetFramePixelRect(u32 frame);
	void computeParamsAfterAtlasGeneration();
	bool load(Json::Value& json) override;
};

}
