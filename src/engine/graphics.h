#pragma once
#include "types.h"
#include <GL/glew.h>
#include <vector>
#include "vertex_buffer.h"
#include "rect.h"
#include "gpu_program.h"

namespace engine
{
#ifndef _DEBUG
#define OGL_CHECK_ERROR { char errStr[1024] = {0}; sprintf(errStr, "File: %s, line: %d", __FILE__, __LINE__); checkErrorGL(errStr); };
#else
#define OGL_CHECK_ERROR
#endif

extern void checkErrorGL(const char* where);

struct Graphics
{
	static const u32 maxVertexCount = 6 * 3000;

	struct Game* game = nullptr;
	f32 videoWidth = 240;
	f32 videoHeight = 320;
	GLuint frameBufferId = 0;
	GLuint renderTargetTextureId = 0;
	struct ImageAtlas* atlas = nullptr;
	struct VertexBuffer* vertexBuffer = nullptr;
	std::vector<Vertex> vertices;
	u32 drawVertexCount = 0;
	f32 vertexCountGrowFactor = 1.5f;
	u32 currentColor = 0;
	u32 currentColorMode = 0;
	u32 atlasTextureIndex = 0;
	GpuProgram gpuProgram;
	GpuProgram blitRTGpuProgram;
	GpuProgram* currentGpuProgram = nullptr;

	Graphics(struct Game* game);
	void createScreenRenderTarget();
	bool createGpuPrograms();
	void commitRenderState();
	void bindRenderTarget();
	void blitRenderTarget();
	void setupBlitRenderTargetRendering();
	void setupRenderTargetRendering();
	void drawQuad(const Rect& rect, const Rect& uvRect);
	void drawRotatedQuad(const Rect& rect, const Rect& uvRect, f32 rotationAngle);
	void drawQuadWithTexCoordRotated90(const Rect& rect, const Rect& uvRect);
	void drawRotatedQuadWithTexCoordRotated90(const Rect& rect, const Rect& uvRect, f32 rotationAngle);
	void beginFrame();
	void setupProjection(f32 width, f32 height);
	void endFrame();
	void needToAddVertexCount(u32 count);
	bool viewportImageFitSize(
		f32 imageWidth, f32 imageHeight,
		f32 viewWidth, f32 viewHeight,
		f32& newWidth, f32& newHeight,
		bool scaleUp,
		bool ignoreHeight, bool ignoreWidth);
};
}
