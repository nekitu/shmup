#include "vertex_buffer.h"
#include "graphics.h"
#include <string.h>

namespace engine
{
VertexBuffer::VertexBuffer()
{
	create(1);
}

VertexBuffer::VertexBuffer(u32 count, Vertex* vertices)
{
	create(count);
	updateData(vertices, 0, count);
}

VertexBuffer::~VertexBuffer()
{
	destroy();
}

void VertexBuffer::create(u32 count)
{
	glGenBuffers(1, (GLuint*)&vbHandle);
	OGL_CHECK_ERROR;
	resize(count);
}

void VertexBuffer::resize(u32 count)
{
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)vbHandle);
	OGL_CHECK_ERROR;
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Vertex) * count,
		nullptr,
		GL_DYNAMIC_DRAW);
	OGL_CHECK_ERROR;
	vertexCount = count;
}

void VertexBuffer::updateData(Vertex* vertices, u32 startVertexIndex, u32 count)
{
	if (!count || !vertices)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, vbHandle);
	OGL_CHECK_ERROR;

	u8* data = (u8*)glMapBufferRange(
		GL_ARRAY_BUFFER,
		sizeof(Vertex) * startVertexIndex,
		sizeof(Vertex) * count,
		GL_MAP_WRITE_BIT);
	OGL_CHECK_ERROR;

	if (!data)
	{
		return;
	}

	memcpy(
		data,
		&vertices[startVertexIndex],
		count * sizeof(Vertex));

	glUnmapBuffer(GL_ARRAY_BUFFER);
	OGL_CHECK_ERROR;
}

void VertexBuffer::destroy()
{
	glDeleteBuffers(1, &vbHandle);
	vbHandle = 0;
}

}
