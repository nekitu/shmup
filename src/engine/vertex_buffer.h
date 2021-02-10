#pragma once
#include "types.h"
#include "vec2.h"
#define GLEW_STATIC
#include <GL/glew.h>

namespace engine
{
struct Vertex
{
	Vec2 position;
	Vec2 uv;
	u32 color;
	u32 colorMode;
	u32 alphaMode;
	u32 textureIndex = 0; /// what atlas texture index this vertex is using
	u32 paletteIndex = 0; /// if 0, no palette used, if > 0, index = paletteIndex - 1
};

struct VertexBuffer
{
	VertexBuffer();
	VertexBuffer(u32 count, Vertex* vertices);
	virtual ~VertexBuffer();
	virtual void resize(u32 count);
	virtual void updateData(Vertex* vertices, u32 startVertexIndex, u32 count);
	virtual void destroy();
	void create(u32 count);

	GLuint vbHandle = 0;
	u32 vertexCount = 0;
};

}
