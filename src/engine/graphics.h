#pragma once
#include "types.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <vector>
#include "vertex_buffer.h"
#include "rect.h"
#include "gpu_program.h"
#include "resources/sprite_resource.h"

namespace engine
{
#ifndef _DEBUG
#define OGL_CHECK_ERROR { char errStr[1024] = {0}; sprintf(errStr, "File: %s, line: %d", __FILE__, __LINE__); checkErrorGL(errStr); };
#else
#define OGL_CHECK_ERROR
#endif

extern void checkErrorGL(const char* where);

struct ColorPalette
{
	std::vector<u32> colors;
	u32 paletteSlot = 0;

	void setColor(u32 index, const Color& color)
	{
		if (index < colors.size())
			colors[index] = color.getRgba();
	}

	Color getColor(u32 index)
	{
		if (index < colors.size())
			return Color(colors[index]);

		return Color();
	}

	void copyFromSprite(struct SpriteResource* spr);
};

struct Graphics
{
	static const u32 maxVertexCount = 50 * 3000;
	static const u32 textureAtlasWidth = 4096;
	static const u32 maxPaletteCount = 256;
	static const u32 maxPaletteColorCount = 256;

	struct Game* game = nullptr;
	f32 videoWidth = 240;
	f32 videoHeight = 320;
	GLuint frameBufferId = 0;
	GLuint renderTargetTextureId = 0;
	struct ImageAtlas* atlas = nullptr;
	struct VertexBuffer* vertexBuffer = nullptr;
	struct TextureArray* palettesTexture;
	std::vector<Vertex> vertices;
	u32 drawVertexCount = 0;
	f32 vertexCountGrowFactor = 1.5f;
	u32 color = 0;
	u32 colorMode = 0;
	u32 alphaMode = 0;
	u32 atlasTextureIndex = 0;
	u32 paletteIndex = 0;
	u32 transparentColorIndex = 0;
	bool shadowToggle = false; // used to toggle shadow each Nth frame to simulate transparency, like arcade did on CRT displays, taking advantage of refresh rate
	GpuProgram gpuProgram;
	GpuProgram blitRTGpuProgram;
	GpuProgram* currentGpuProgram = nullptr;
	std::vector<Color> colorStack;
	std::vector<ColorMode> colorModeStack;
	std::vector<AlphaMode> alphaModeStack;
	bool paletteSlots[maxPaletteCount] = { false };
	Rect blittedRect;

	Graphics(struct Game* game);
	void createScreenRenderTarget();
	bool createGpuPrograms();
	void commitRenderState();
	void bindRenderTarget();
	void blitRenderTarget();
	void setupBlitRenderTargetRendering();
	void setupRenderTargetRendering();
	void drawQuad(const Rect& rect, const Rect& uvRect, bool rotateUv90 = false);
	void drawCustomQuad(const Vec2& topLeft, const Vec2& topRight, const Vec2& btmRight, const Vec2& btmLeft, const Rect& uvRect, bool rotateUv90);
	void drawRotatedQuad(const Rect& rect, const Rect& uvRect, bool rotateUv90, f32 rotationAngle);
	void drawText(struct FontResource* font, const Vec2& pos, const std::string& text);
	Vec2 getTextSize(struct FontResource* font, const std::string& text);
	void drawSprite(struct SpriteResource* spr, const Rect& rc, u32 frame, f32 angle, struct ColorPalette* userPalette = nullptr);
	void drawSpriteCustomQuad(struct SpriteResource* spr, const Vec2& topLeft, const Vec2& topRight, const Vec2& btmRight, const Vec2& btmLeft, u32 frame, f32 angle, struct ColorPalette* userPalette = nullptr);
	void drawLine(const Vec2& a, const Vec2& b, f32 thickness = 1);
	void drawRect(const Rect& rc, f32 thickness = 1);
	void beginFrame();
	void setupProjection(f32 width, f32 height);
	void endFrame();
	void needToAddVertexCount(u32 count);
	void pushColor(const Color& newColor);
	void pushColorMode(ColorMode newColorMode);
	void pushAlphaMode(AlphaMode newAlphaMode);
	void popColor();
	void popColorMode();
	void popAlphaMode();
	void setupColor(const Color& newColor, ColorMode newColorMode = ColorMode::Add);
	void setupColorU32(u32 newColor = 0, ColorMode newColorMode = ColorMode::Add);
	u32 allocPaletteSlot();
	void freePaletteSlot(u32 slot);
	void uploadPalette(u32 slot, u32* paletteColors);
	ColorPalette* createUserPalette();
	void freeUserPalette(ColorPalette* pal);
	bool viewportImageFitSize(
		f32 imageWidth, f32 imageHeight,
		f32 viewWidth, f32 viewHeight,
		f32& newWidth, f32& newHeight,
		bool scaleUp,
		bool ignoreHeight, bool ignoreWidth);
};
}
