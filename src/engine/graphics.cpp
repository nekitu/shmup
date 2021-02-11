#include "graphics.h"
#include "game.h"
#include <string>
#include "texture_array.h"
#include "vertex_buffer.h"
#include "image_atlas.h"
#include "utils.h"
#include "resources/font_resource.h"
#include "resources/sprite_resource.h"

namespace engine
{
void ColorPalette::copyFromSprite(struct SpriteResource* spr)
{
	colors = spr->paletteInfo.colors;
}

void checkErrorGL(const char* where)
{
	GLuint err = glGetError();
	std::string str;

	switch (err)
	{
	case GL_INVALID_ENUM:
		str = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
		break;
	case GL_INVALID_VALUE:
		str = "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
		break;
	case GL_INVALID_OPERATION:
		str = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
		break;
	case GL_STACK_OVERFLOW:
		str = "GL_STACK_OVERFLOW: This command would cause a stack overflow. The offending command is ignored and has no other side effect than to set the error flag.";
		break;
	case GL_STACK_UNDERFLOW:
		str = "GL_STACK_UNDERFLOW: This command would cause a stack underflow. The offending command is ignored and has no other side effect than to set the error flag.";
		break;
	case GL_OUT_OF_MEMORY:
		str = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
		break;
	case GL_TABLE_TOO_LARGE:
		str = "GL_TABLE_TOO_LARGE: The specified table exceeds the implementation's maximum supported table size. The offending command is ignored and has no other side effect than to set the error flag.";
		break;
	};

	if (err != GL_NO_ERROR)
	{
		LOG_ERROR("[{0}] OpenGL: code#{1}: {2}", where, err, str);
	}
}

static const char* vertexShaderSource =
"\
#version 130\r\n\
\
in vec2 inPOSITION;\
in vec2 inTEXCOORD0;\
in uint inCOLOR;\
in uint inColorMode;\
in uint inAlphaMode;\
in uint inTEXINDEX;\
in uint inPALETTE;\
in uint inTRANSPARENTINDEX;\
\
uniform mat4 mvp;\
out vec2 outTEXCOORD;\
out vec4 outCOLOR;\
flat out uint outColorMode;\
flat out uint outAlphaMode;\
flat out uint outTEXINDEX;\
flat out uint outPALETTE;\
flat out uint outTRANSPARENTINDEX;\
\
void main()\
{\
    vec4 v = mvp * vec4(inPOSITION.x, inPOSITION.y, 0, 1);\
    gl_Position = v;\
    outTEXCOORD = inTEXCOORD0;\
    vec4 color = vec4(float(inCOLOR & uint(0x000000FF))/255.0, float((inCOLOR & uint(0x0000FF00)) >> uint(8))/255.0, float((inCOLOR & uint(0x00FF0000)) >> uint(16))/255.0, float((inCOLOR & uint(0xFF000000))>> uint(24))/255.0);\
    outCOLOR = color;\
    outColorMode = inColorMode;\
    outAlphaMode = inAlphaMode;\
    outTEXINDEX = inTEXINDEX;\
    outPALETTE = inPALETTE;\
    outTRANSPARENTINDEX = inTRANSPARENTINDEX;\
    return;\
}\
";

static const char* pixelShaderSource =
"\
#version 130\r\n\
#extension GL_EXT_texture_array : enable\r\n\
uniform sampler2DArray diffuseSampler;\
uniform sampler2DArray paletteSampler;\
in vec2 outTEXCOORD;\
in vec4 outCOLOR;\
flat in uint outColorMode;\
flat in uint outAlphaMode;\
flat in uint outTEXINDEX;\
flat in uint outPALETTE;\
flat in uint outTRANSPARENTINDEX;\
out vec4 finalCOLOR;\
\
void main()\
{\
	vec4 texelColor;\
	texelColor = texture2DArray(diffuseSampler, vec3(outTEXCOORD, float(outTEXINDEX)));\
	if (outPALETTE != uint(0))\
	{\
		float idx = texelColor.r;\
		texelColor = texture2DArray(paletteSampler, vec3(idx, 0, float(outPALETTE - uint(1))));\
		texelColor.a = float(uint(idx * 255) != uint(outTRANSPARENTINDEX));\
	}\
	if (outAlphaMode == 0U && texelColor.a < 1) discard;\
	if (outColorMode == 0U)\
		finalCOLOR = texelColor + outCOLOR;\
	else if (outColorMode == 1U)\
		finalCOLOR = texelColor - outCOLOR;\
	else if (outColorMode == 2U)\
		finalCOLOR = texelColor * outCOLOR;\
}\
";

static const char* blitRTPixelShaderSource =
"\
#version 130\r\n\
uniform sampler2D diffuseSampler;\
\
in vec2 outTEXCOORD;\
in vec4 outCOLOR;\
flat in uint outTEXINDEX;\
out vec4 finalCOLOR;\
\
void main()\
{\
    vec4 texelColor = texture2D(diffuseSampler, outTEXCOORD);\
	finalCOLOR = texelColor;\
}\
";

Graphics::Graphics(Game* game)
{
	this->game = game;
	createScreenRenderTarget();
	createGpuPrograms();
	atlas = new ImageAtlas(textureAtlasWidth, textureAtlasWidth);
	vertexBuffer = new VertexBuffer();
	vertexBuffer->resize(maxVertexCount);
	vertices.resize(maxVertexCount);

	palettesTexture = new TextureArray(maxPaletteCount, maxPaletteColorCount, 1);
}

void Graphics::createScreenRenderTarget()
{
	glGenFramebuffers(1, &frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
	glGenTextures(1, &renderTargetTextureId);
	glBindTexture(GL_TEXTURE_2D, renderTargetTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, videoWidth, videoHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTargetTextureId, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_INFO("Framebuffer is complete!");
	}
}

bool Graphics::createGpuPrograms()
{
	gpuProgram.create(vertexShaderSource, pixelShaderSource);
	blitRTGpuProgram.create(vertexShaderSource, blitRTPixelShaderSource);
	currentGpuProgram = &gpuProgram;
	return true;
}

void Graphics::commitRenderState()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_EQUAL, 1);
}

void Graphics::bindRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
}

void Graphics::blitRenderTarget()
{
	GLint oldFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	setupBlitRenderTargetRendering();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	drawVertexCount = 0;
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	f32 newWidth, newHeight;
	// find highest size multiple of videoHeight to fit into the window size
	// to avoid problems with scaling of the render target image
	u32 repeatHeight = game->windowHeight / videoHeight;

	newHeight = videoHeight * repeatHeight;
	newWidth = newHeight * (f32)videoWidth / videoHeight;

	Rect rc = { round((game->windowWidth - newWidth) / 2.0f),
		round((game->windowHeight - newHeight) / 2.0f),
		round(newWidth), round(newHeight) };
	drawQuad(rc, { 0, 1, 1, -1 });
	endFrame();
	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
}

void Graphics::beginFrame()
{
	drawVertexCount = 0;
	glClearColor(0.2, 0.2, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Graphics::setupProjection(f32 width, f32 height)
{
	GLint loc = glGetUniformLocation((GLuint)currentGpuProgram->program, "mvp");
	OGL_CHECK_ERROR;

	glViewport(0, 0, width, height);

	if (loc != -1)
	{
		f32 m[4][4] = { 0 };

		m[0][0] = 2.0f / width;
		m[0][1] = 0.0f;
		m[0][2] = 0.0f;
		m[0][3] = 0.0f;

		m[1][0] = 0.0f;
		m[1][1] = -2.0f / height;
		m[1][2] = 0.0f;
		m[1][3] = 0.0f;

		m[2][0] = 0.0f;
		m[2][1] = 0.0f;
		m[2][2] = 1.0f;
		m[2][3] = 0.0f;

		m[3][0] = -1;
		m[3][1] = 1;
		m[3][2] = 0.0f;
		m[3][3] = 1.0f;

		glUniformMatrix4fv(loc, 1, false, (GLfloat*)m);
		OGL_CHECK_ERROR;
	}
}

void Graphics::setupBlitRenderTargetRendering()
{
	currentGpuProgram = &blitRTGpuProgram;
	currentGpuProgram->use();
	setupProjection(game->windowWidth, game->windowHeight);
	glBindTexture(GL_TEXTURE_2D, renderTargetTextureId);
	blitRTGpuProgram.setSamplerValue(renderTargetTextureId, "diffuseSampler", 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	OGL_CHECK_ERROR;
}

void Graphics::setupRenderTargetRendering()
{
	currentGpuProgram = &gpuProgram;
	currentGpuProgram->use();
	bindRenderTarget();
	setupProjection(videoWidth, videoHeight);

	gpuProgram.setSamplerValue(atlas->textureArray->handle, "diffuseSampler", 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->textureArray->handle);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	OGL_CHECK_ERROR;

	gpuProgram.setSamplerValue(palettesTexture->handle, "paletteSampler", 1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, palettesTexture->handle);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	OGL_CHECK_ERROR;
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	OGL_CHECK_ERROR;
}

void Graphics::endFrame()
{
	#define OGL_VBUFFER_OFFSET(i) ((void*)(i))

	commitRenderState();
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vbHandle);
	OGL_CHECK_ERROR;
	vertexBuffer->updateData(vertices.data(), 0, drawVertexCount);

	u32 stride = sizeof(Vertex);
	u32 attrLoc = 0;
	u32 offsetSum = 0;

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inPOSITION");
	OGL_CHECK_ERROR;

	if (attrLoc == ~0)
	{
		return;
	}

	glEnableVertexAttribArray(attrLoc);
	OGL_CHECK_ERROR;
	glVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, stride, OGL_VBUFFER_OFFSET(offsetSum));
	OGL_CHECK_ERROR;

	if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
	OGL_CHECK_ERROR;
	offsetSum += sizeof(f32) * 2;

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inTEXCOORD0");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(f32) * 2;
	}

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inCOLOR");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribIPointer(attrLoc, 1, GL_UNSIGNED_INT, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(u32);
	}

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inColorMode");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribIPointer(attrLoc, 1, GL_UNSIGNED_INT, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(u32);
	}

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inAlphaMode");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribIPointer(attrLoc, 1, GL_UNSIGNED_INT, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(u32);
	}

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inTEXINDEX");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribIPointer(attrLoc, 1, GL_UNSIGNED_INT, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(u32);
	}

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inPALETTE");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribIPointer(attrLoc, 1, GL_UNSIGNED_INT, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(u32);
	}

	attrLoc = glGetAttribLocation(currentGpuProgram->program, "inTRANSPARENTINDEX");

	if (attrLoc != ~0)
	{
		OGL_CHECK_ERROR;
		glEnableVertexAttribArray(attrLoc);
		OGL_CHECK_ERROR;
		glVertexAttribIPointer(attrLoc, 1, GL_UNSIGNED_INT, stride, OGL_VBUFFER_OFFSET(offsetSum));
		OGL_CHECK_ERROR;

		if (glVertexAttribDivisor) glVertexAttribDivisor(attrLoc, 0);
		OGL_CHECK_ERROR;
		offsetSum += sizeof(u32);
	}

	int primType = GL_TRIANGLES;

	glDrawArrays(primType, 0, drawVertexCount);
	OGL_CHECK_ERROR;

	glUseProgram(0);
	OGL_CHECK_ERROR;
}

void Graphics::needToAddVertexCount(u32 count)
{
	if (drawVertexCount + count < vertices.size())
	{
		// already have space available
		return;
	}

	std::vector<Vertex> verts = vertices;

	vertices.resize(vertices.size() * vertexCountGrowFactor + count);
	vertices.insert(vertices.begin(), verts.begin(), verts.end());
	vertexBuffer->resize(vertices.size());
}

void Graphics::drawCustomQuad(const Vec2& topLeft, const Vec2& topRight, const Vec2& btmRight, const Vec2& btmLeft, const Rect& uvRect, bool rotateUv90)
{
	needToAddVertexCount(6);

	u32 i = drawVertexCount;
	Vec2 t0;
	Vec2 t1;
	Vec2 t2;
	Vec2 t3;

	if (!rotateUv90)
	{
		// t0-------t1
		//  |     /  |
		//  |  /     |
		// t3-------t2
		t0 = uvRect.topLeft();
		t1 = uvRect.topRight();
		t2 = uvRect.bottomRight();
		t3 = uvRect.bottomLeft();
	}
	else
	{
		// t3-------t0
		//  |     /  |
		//  |  /     |
		// t2-------t1
		t3 = uvRect.topLeft();
		t0 = uvRect.topRight();
		t1 = uvRect.bottomRight();
		t2 = uvRect.bottomLeft();
	}

	vertices[i].position = topLeft;
	vertices[i].uv = t0;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = topRight;
	vertices[i].uv = t1;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = btmLeft;
	vertices[i].uv = t3;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	// 2nd triangle

	vertices[i].position = topRight;
	vertices[i].uv = t1;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = btmRight;
	vertices[i].uv = t2;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = btmLeft;
	vertices[i].uv = t3;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	drawVertexCount = i;
}

void Graphics::drawQuad(const Rect& rect, const Rect& uvRect, bool rotateUv90)
{
	needToAddVertexCount(6);

	u32 i = drawVertexCount;
	Vec2 t0;
	Vec2 t1;
	Vec2 t2;
	Vec2 t3;

	if (!rotateUv90)
	{
		// t0-------t1
		//  |     /  |
		//  |  /     |
		// t3-------t2
		t0 = uvRect.topLeft();
		t1 = uvRect.topRight();
		t2 = uvRect.bottomRight();
		t3 = uvRect.bottomLeft();
	}
	else
	{
		// t3-------t0
		//  |     /  |
		//  |  /     |
		// t2-------t1
		t3 = uvRect.topLeft();
		t0 = uvRect.topRight();
		t1 = uvRect.bottomRight();
		t2 = uvRect.bottomLeft();
	}

	vertices[i].position = rect.topLeft();
	vertices[i].uv = t0;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = rect.topRight();
	vertices[i].uv = t1;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = rect.bottomLeft();
	vertices[i].uv = t3;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	// 2nd triangle

	vertices[i].position = rect.topRight();
	vertices[i].uv = t1;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = rect.bottomRight();
	vertices[i].uv = t2;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = rect.bottomLeft();
	vertices[i].uv = t3;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	drawVertexCount = i;
}

void Graphics::drawRotatedQuad(const Rect& rect, const Rect& uvRect, bool rotateUv90, f32 rotationAngle)
{
	needToAddVertexCount(6);

	u32 i = drawVertexCount;

	Vec2 t0;
	Vec2 t1;
	Vec2 t2;
	Vec2 t3;

	if (!rotateUv90)
	{
		// t0-------t1
		//  |     /  |
		//  |  /     |
		// t3-------t2
		t0 = uvRect.topLeft();
		t1 = uvRect.topRight();
		t2 = uvRect.bottomRight();
		t3 = uvRect.bottomLeft();
	}
	else
	{
		// t3-------t0
		//  |     /  |
		//  |  /     |
		// t2-------t1
		t3 = uvRect.topLeft();
		t0 = uvRect.topRight();
		t1 = uvRect.bottomRight();
		t2 = uvRect.bottomLeft();
	}

	// v0------v1
	//  |    /  |
	//  |  /    |
	// v3------v2

	Vec2 v0(rect.topLeft());
	Vec2 v1(rect.topRight());
	Vec2 v2(rect.bottomRight());
	Vec2 v3(rect.bottomLeft());

	Vec2 center = rect.center();
	auto angle = deg2rad(rotationAngle);

	v0.rotateAround(center, angle);
	v1.rotateAround(center, angle);
	v2.rotateAround(center, angle);
	v3.rotateAround(center, angle);

	vertices[i].position = v0;
	vertices[i].uv = t0;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = v1;
	vertices[i].uv = t1;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = v3;
	vertices[i].uv = t3;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	// 2nd triangle

	vertices[i].position = v1;
	vertices[i].uv = t1;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = v2;
	vertices[i].uv = t2;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	vertices[i].position = v3;
	vertices[i].uv = t3;
	vertices[i].color = color;
	vertices[i].colorMode = colorMode;
	vertices[i].alphaMode = alphaMode;
	vertices[i].textureIndex = atlasTextureIndex;
	vertices[i].paletteIndex = paletteIndex;
	vertices[i].transparentColorIndex = transparentColorIndex;
	i++;

	drawVertexCount = i;
}

void Graphics::drawText(struct FontResource* font, const Vec2& pos, const std::string& text)
{
	UnicodeString ustr;

	utf8ToUtf32(text.c_str(), ustr);
	Vec2 crtPos = pos;
	u32 i = 0;

	for (auto chr : ustr)
	{
		auto frame = font->getGlyphSpriteFrame(chr);
		auto frameUvRect = font->charsSprite->getFrameUvRect(frame);
		auto framePixRect = font->charsSprite->getFramePixelRect(frame);
		auto rc = Rect(crtPos.x, crtPos.y, font->charsSprite->frameWidth, font->charsSprite->frameHeight);

		drawQuad(rc, frameUvRect, font->charsSprite->image->rotated);
		i32 kern = 0;

		if (i < ustr.size() - 1)
		{
			auto kern = font->kernings[std::make_pair(chr, ustr[i + 1])];
		}

		crtPos.x += rc.width + kern;
		i++;
	}
}

bool Graphics::viewportImageFitSize(
	f32 imageWidth, f32 imageHeight,
	f32 viewWidth, f32 viewHeight,
	f32& newWidth, f32& newHeight, bool scaleUp,
	bool ignoreHeight, bool ignoreWidth)
{
	f32 aspectRatio = 1.0f;

	newWidth = imageWidth;
	newHeight = imageHeight;

	if (imageWidth <= viewWidth
		&& imageHeight <= viewHeight)
	{
		if (!scaleUp)
			return false;

		newHeight = viewHeight;
		aspectRatio = (f32)imageWidth / imageHeight;
		newWidth = aspectRatio * newHeight;
	}

	if (newWidth >= viewWidth && !ignoreWidth)
	{
		if (newWidth < 0.0001f)
			newWidth = 0.0001f;

		aspectRatio = (f32)viewWidth / newWidth;
		newWidth = viewWidth;
		newHeight *= aspectRatio;
	}

	if (newHeight >= viewHeight && !ignoreHeight)
	{
		if (newHeight < 0.0001f)
			newHeight = 0.0001f;

		aspectRatio = (f32)viewHeight / newHeight;
		newHeight = viewHeight;
		newWidth *= aspectRatio;
	}

	return true;
}

void Graphics::pushColor(const Color& newColor)
{
	colorStack.push_back(color);
	color = newColor.getRgba();
}

void Graphics::pushColorMode(ColorMode newColorMode)
{
	colorModeStack.push_back((ColorMode)colorMode);
	colorMode = (u32)newColorMode;
}

void Graphics::pushAlphaMode(AlphaMode newAlphaMode)
{
	alphaModeStack.push_back((AlphaMode)alphaMode);
	alphaMode = (u32)newAlphaMode;
}

void Graphics::popColor()
{
	color = colorStack.back().getRgba();
	colorStack.pop_back();
}

void Graphics::popColorMode()
{
	colorMode = (u32)colorModeStack.back();
	colorModeStack.pop_back();
}

void Graphics::popAlphaMode()
{
	alphaMode = (u32)alphaModeStack.back();
	alphaModeStack.pop_back();
}

u32 Graphics::allocPaletteSlot()
{
	for (int i = 0; i < maxPaletteCount; i++)
	{
		if (!paletteSlots[i])
		{
			paletteSlots[i] = true;
			return i;
		}
	}

	return 0xffffffff;
}

void Graphics::freePaletteSlot(u32 slot)
{
	paletteSlots[slot] = false;
}

void Graphics::uploadPalette(u32 slot, u32* paletteColors)
{
	palettesTexture->updateLayerData(slot, paletteColors);
}

ColorPalette* Graphics::createUserPalette()
{
	ColorPalette* pal = new ColorPalette();

	pal->paletteSlot = allocPaletteSlot();

	if (pal->paletteSlot == ~0)
	{
		return pal;
	}

	pal->colors.resize(maxPaletteColorCount);

	return pal;
}

void Graphics::freeUserPalette(ColorPalette* pal)
{
	freePaletteSlot(pal->paletteSlot);
	delete pal;
}

void Graphics::drawSprite(SpriteResource* spr, const Rect& rc, u32 frame, f32 angle, ColorPalette* userPalette)
{
	colorMode = (int)ColorMode::Add;
	color = 0;

	atlasTextureIndex = spr->image->atlasTexture->textureIndex;
	auto usePalette = spr->paletteInfo.isPaletted;

	if (usePalette)
	{
		// palette index is inc by 1, 0 means no palette in shader
		paletteIndex = userPalette ? userPalette->paletteSlot + 1 : spr->paletteInfo.paletteSlot + 1;
		transparentColorIndex = spr->paletteInfo.transparentColorIndex;

		if (userPalette)
		{
			uploadPalette(userPalette->paletteSlot, userPalette->colors.data());
		}
		else
		{
			uploadPalette(spr->paletteInfo.paletteSlot, spr->paletteInfo.colors.data());
		}
	}
	else
	{
		paletteIndex = 0;
	}

	drawRotatedQuad(rc, spr->getFrameUvRect(frame), spr->image->rotated, angle);

	if (usePalette)
	{
		paletteIndex = 0;
	}
}

void Graphics::drawSpriteCustomQuad(struct SpriteResource* spr, const Vec2& topLeft, const Vec2& topRight, const Vec2& btmRight, const Vec2& btmLeft, u32 frame, f32 angle, struct ColorPalette* userPalette)
{
	colorMode = (int)ColorMode::Add;
	color = 0;

	atlasTextureIndex = spr->image->atlasTexture->textureIndex;
	auto usePalette = spr->paletteInfo.isPaletted;
	
	if (usePalette)
	{
		// palette index is inc by 1, 0 means no palette in shader
		paletteIndex = userPalette ? userPalette->paletteSlot + 1 : spr->paletteInfo.paletteSlot + 1;
		transparentColorIndex = spr->paletteInfo.transparentColorIndex;
		if (userPalette)
		{
			uploadPalette(userPalette->paletteSlot, userPalette->colors.data());
		}
		else
		{
			uploadPalette(spr->paletteInfo.paletteSlot, spr->paletteInfo.colors.data());
		}
	}
	else
	{
		paletteIndex = 0;
	}

	drawCustomQuad(topLeft, topRight, btmRight, btmLeft, spr->getFrameUvRect(frame), spr->image->rotated);

	if (usePalette)
	{
		paletteIndex = 0;
	}
}


}
