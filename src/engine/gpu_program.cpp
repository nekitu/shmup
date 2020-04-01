#include "gpu_program.h"
#include "graphics.h"
#include <string.h>

namespace engine
{
bool GpuProgram::create(const std::string& vsSrc, const std::string& psSrc)
{
	vertexShaderSource = vsSrc;
	pixelShaderSource = psSrc;
	program = glCreateProgram();

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* srcVsCode = vertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &srcVsCode, nullptr);
	OGL_CHECK_ERROR;
	glCompileShader(vertexShader);
	OGL_CHECK_ERROR;
	glAttachShader((GLuint)program, vertexShader);
	OGL_CHECK_ERROR;

	pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* srcPsCode = pixelShaderSource.c_str();
	glShaderSource(pixelShader, 1, &srcPsCode, nullptr);
	OGL_CHECK_ERROR;
	glCompileShader(pixelShader);
	OGL_CHECK_ERROR;
	glAttachShader((GLuint)program, pixelShader);
	OGL_CHECK_ERROR;

	glLinkProgram((GLuint)program);
	OGL_CHECK_ERROR;

	{
		GLchar errorLog[1024] = { 0 };
		glGetProgramInfoLog((GLuint)program, 1024, NULL, errorLog);
		OGL_CHECK_ERROR;

		if (strcmp(errorLog, ""))
		{
			LOG_INFO("Linking program: {0}", errorLog);
		}
	}

	if (!glIsProgram((GLuint)program))
	{
		LOG_INFO("Program ID: {0} is not a valid OpenGL program", program);
	}
	OGL_CHECK_ERROR;

	GLint err;
	glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &err);
	OGL_CHECK_ERROR;

	if (!err)
	{
		GLchar errorLog[1024] = { 0 };
		glGetShaderInfoLog((GLuint)pixelShader, 1024, NULL, errorLog);
		OGL_CHECK_ERROR;
		LOG_ERROR("Error validating pixel shader: '{0}'", errorLog);
		return false;
	}

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &err);
	OGL_CHECK_ERROR;

	if (!err)
	{
		GLchar errorLog[1024] = { 0 };
		glGetShaderInfoLog((GLuint)vertexShader, 1024, NULL, errorLog);
		OGL_CHECK_ERROR;
		LOG_ERROR("Error validating vertex shader: '{0}'", errorLog);
		return false;
	}

	LOG_INFO("Validating program id: {0}", program);
	glValidateProgram((GLuint)program);
	OGL_CHECK_ERROR;

	GLint success = GL_FALSE;
	glGetProgramiv((GLuint)program, GL_VALIDATE_STATUS, &success);
	OGL_CHECK_ERROR;

	if (success == GL_FALSE)
	{
		GLchar errorLog[1024] = { 0 };
		glGetProgramInfoLog((GLuint)program, 1024, NULL, errorLog);
		OGL_CHECK_ERROR;
		LOG_ERROR("Error validating program: '{0}'", errorLog);
		return false;
	}

	GLint numAttrs;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttrs);
	OGL_CHECK_ERROR;

	for (int n = 0; n < numAttrs; n++)
	{
		GLsizei len;
		GLint size;
		GLenum type;
		GLchar name[100];
		glGetActiveAttrib(program, n, 100, &len, &size, &type, name);
		OGL_CHECK_ERROR;
		LOG_INFO("{0}", name);
	}

	return true;
}

void GpuProgram::destroy()
{
	glDetachShader(program, vertexShader);
	glDetachShader(program, pixelShader);
	glDeleteShader(vertexShader);
	glDeleteShader(pixelShader);
	glDeleteProgram(program);
}

void GpuProgram::use()
{
	glUseProgram(program);
}

void GpuProgram::setSamplerValue(GLuint tex, const std::string& constName, u32 stage)
{
	GLint loc = glGetUniformLocation(program, constName.c_str());
	OGL_CHECK_ERROR;

	if (loc == -1)
	{
		return;
	}

	glActiveTexture(GL_TEXTURE0 + stage);
	OGL_CHECK_ERROR;
	glUniform1i(loc, stage);
	OGL_CHECK_ERROR;
}

void GpuProgram::setIntValue(GLuint value, const std::string& constName)
{
	GLint loc = glGetUniformLocation(program, constName.c_str());
	OGL_CHECK_ERROR;

	if (loc == -1)
	{
		return;
	}

	glUniform1i(loc, value);
	OGL_CHECK_ERROR;
}

}
