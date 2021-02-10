#pragma once
#include "types.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include "rect.h"

namespace engine
{
struct TextureArray
{
	TextureArray() {}
	TextureArray(u32 count, u32 newWidth, u32 newHeight, Rgba32* pixels);
	TextureArray(u32 count, u32 newWidth, u32 newHeight);
	~TextureArray();
	void destroy();

	void resize(u32 count, u32 newWidth, u32 newHeight);
	void updateData(Rgba32* pixels);
	void updateLayerData(u32 textureIndex, Rgba32* pixels);
	void updateRectData(u32 textureIndex, const Rect& rect, Rgba32* pixels);

	GLuint handle = 0;
	u32 width = 0;
	u32 height = 0;
	u32 textureCount = 0;
};

}
