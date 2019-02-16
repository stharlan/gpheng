#pragma once

#include "stdafx.h"

using namespace std;

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLVALIDATEPROGRAMPROC glValidateProgram;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLTEXSTORAGE2DPROC glTexStorage2D;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
extern PFNGLUNIFORM1FPROC glUniform1f;

void DumpGlErrors(const char* FunctionName);

ShaderProgramContext::ShaderProgramContext() {
	this->VertexShader = 0;
	this->FragmentShader = 0;
	this->ShaderProgram = 0;
}

ShaderProgramContext::ShaderProgramContext(const char* VertexShaderSource, const char* FragmentShaderSource) {
	this->VertexShader = 0;
	this->FragmentShader = 0;
	this->ShaderProgram = 0;
	if (TRUE == this->SetupShaders(VertexShaderSource, FragmentShaderSource))
	{
		this->SetAsCurrent();
	}
}

ShaderProgramContext::~ShaderProgramContext() { 
}

void ShaderProgramContext::FreeResources() {
	glDeleteShader(this->VertexShader);
	glDeleteShader(this->FragmentShader);
	glDeleteProgram(this->ShaderProgram);
}

void ShaderProgramContext::BuildFromSource(const char* VertexShaderSource, const char* FragmentShaderSource) {
	if (TRUE == this->SetupShaders(VertexShaderSource, FragmentShaderSource))
	{
		this->SetAsCurrent();
	}
}

void ShaderProgramContext::SetAsCurrent() {
	glUseProgram(this->ShaderProgram);
}

void ShaderProgramContext::SetUniformInt(GLuint loc, GLint value) {
	glUniform1i(loc, value);
}

void ShaderProgramContext::SetUniformVector3(GLuint loc, const GLfloat* value)
{
	glUniform3fv(loc, 1, value);
}

void ShaderProgramContext::SetUniformMatrix4(GLuint loc, const GLfloat* value)
{
	glUniformMatrix4fv(loc, 1, GL_FALSE, value);
}

void ShaderProgramContext::SetUniformMatrix3(GLuint loc, const GLfloat* value)
{
	glUniformMatrix3fv(loc, 1, GL_FALSE, value);
}

void ShaderProgramContext::SetUniformFloat(GLuint loc, GLfloat value)
{
	glUniform1f(loc, value);
}

BOOL ShaderProgramContext::SetupShaders(const char* vtxss, const char* frgss)
{

	cout << "Validating shader" << endl;
	DumpGlErrors("SetupShaders");

	GLint success = 0;

	this->ShaderProgram = glCreateProgram();

	this->VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(this->VertexShader, 1, &vtxss, NULL);
	glCompileShader(this->VertexShader);
	glGetShaderiv(this->VertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to compile vertex shader";
		return FALSE;
	}

	this->FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(this->FragmentShader, 1, &frgss, NULL);
	glCompileShader(this->FragmentShader);
	glGetShaderiv(this->FragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to compile fragment shader";
		return FALSE;
	}

	glAttachShader(this->ShaderProgram, this->VertexShader);
	glAttachShader(this->ShaderProgram, this->FragmentShader);

	glLinkProgram(this->ShaderProgram);
	glGetProgramiv(this->ShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to link shader program" << endl;
		return FALSE;
	}
	else {
		cout << "Shader linked" << endl;
	}

	glValidateProgram(this->ShaderProgram);
	glGetProgramiv(this->ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to validate shader program" << endl;
		return FALSE;
	}
	else {
		cout << "Shader validated" << endl;
	}

	DumpGlErrors("SetupShaders");

	return TRUE;
}
