#include "sprite.h"
#include <stb_image.h>
#include "image_atlas.h"

namespace engine
{
AtlasImage* Sprite::loadImage(const char* filename, ImageAtlas* atlas)
{
	int width = 0;
	int height = 0;
	int comp;
	stbi_uc* data = stbi_load(filename, &width, &height, &comp, 4);

	if (!data)
		return nullptr;

	auto img = atlas->addImage((Rgba32*)data, width, height);
	delete[] data;
	atlas->pack();

	return img;
}

Rect Sprite::getFrameUvRect(u32 frame)
{
	Rect rc;

	u32 cols = (f32)image->width / (f32)frameWidth;
	u32 rows = (f32)image->height / (f32)frameHeight;
	u32 col = frame % cols;
	u32 row = frame / cols;

	rc.x = image->uvRect.x + uvFrameWidth * (f32)col;
	rc.y = image->uvRect.y + uvFrameHeight * (f32)row;
	rc.width = uvFrameWidth;
	rc.height = uvFrameHeight;

	return rc;
}

}