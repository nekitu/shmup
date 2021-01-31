#pragma once
#include "types.h"
#include <string>
#include <GL/glew.h>

namespace engine
{
struct GpuProgram
{
	std::string vertexShaderSource;
	std::string pixelShaderSource;
	GLuint vertexShader = 0;
	GLuint pixelShader = 0;
	GLuint program = 0;

	bool create(const std::string& vsSrc, const std::string& psSrc);
	void destroy();
	void use();
	void setSamplerValue(GLuint tex, const std::string& constName, u32 stage);
	void setUIntValue(GLuint value, const std::string& constName);
	void setIntValue(GLint value, const std::string& constName);
	void setFloatValue(GLfloat value, const std::string& constName);
	void setUIntArrayValue(u32 count, GLuint* values, const std::string& constName);
	void setIntArrayValue(u32 count, GLint* values, const std::string& constName);
	void setFloatArrayValue(u32 count, GLfloat* values, const std::string& constName);
};
}
