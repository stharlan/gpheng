#include "stdafx.h"
#include "ColorWorldShader.h"

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

const char* g_ColorVertexShaderSource =
"#version 400\n"
"layout(location = 0) in vec3 vPosition;"
"layout(location = 1) in vec2 vTexCoord;"
"layout(location = 2) in vec3 vNormal;"
"uniform mat4 gModelMatrix;"
"uniform mat4 gViewMatrix;"
"uniform mat4 gProjMatrix;"
"uniform mat3 gNormViewMatrix;"
"uniform mat3 gNormModelMatrix;"
"uniform vec3 gLightPos;"
"out vec3 Position;"
"out vec3 Normal;"
"out vec3 LightPos;"
"out vec3 ReflectDir;"  // The direction of the reflected ray
"void main() {"
"  Normal = normalize(gNormViewMatrix * (gNormModelMatrix * vNormal));"
"  Position = vec3(gViewMatrix * (gModelMatrix * vec4(vPosition, 1.0)));"
"  LightPos = vec3(gViewMatrix * vec4(gLightPos, 1.0));"
"  gl_Position = gProjMatrix * (gViewMatrix * (gModelMatrix * vec4(vPosition, 1.0)));"
"  ReflectDir = vPosition;"
"}";

const char* g_ColorFragmentShaderSource =
"#version 400\n"
"in vec3 Position;"
"in vec3 Normal;"
"in vec3 LightPos;"
"in vec3 ReflectDir;"   // The direction of the reflected ray 
"out vec4 frag_color;"
"uniform vec3 LightIntensity;"
"uniform vec3 LightAmbient;"
"uniform vec3 LightDiffuse;"
"uniform vec3 LightSpecular;"
"uniform float LightShininess;"
"uniform vec4 MatColor;"
"void main() {"
"  vec3 n = normalize(Normal);"
"  vec3 s = normalize(LightPos - Position);"
"  vec3 v = normalize(-Position);"
"  vec3 r = reflect(-s, n);"
"  vec3 result = LightIntensity * ("
"    LightAmbient +"
"    LightDiffuse * max(dot(s,n),0.0) +"
"    LightSpecular * pow(max(dot(r,v),0.0), LightShininess));"
"  frag_color = MatColor * vec4(result, 1.0);"
"}";

ColorWorldShader::ColorWorldShader()
{
	this->BuildFromSource(g_ColorVertexShaderSource, g_ColorFragmentShaderSource);
}


ColorWorldShader::~ColorWorldShader()
{
}

void ColorWorldShader::ResolveLocations()
{
	this->gModelMatrixLoc = glGetUniformLocation(this->ShaderProgram, "gModelMatrix");
	this->gViewMatrixLoc = glGetUniformLocation(this->ShaderProgram, "gViewMatrix");
	this->gProjMatrixLoc = glGetUniformLocation(this->ShaderProgram, "gProjMatrix");
	this->gLightPosLoc = glGetUniformLocation(this->ShaderProgram, "gLightPos");
	this->gNormModelMatrixLoc = glGetUniformLocation(this->ShaderProgram, "gNormModelMatrix");
	this->gNormViewMatrixLoc = glGetUniformLocation(this->ShaderProgram, "gNormViewMatrix");

	this->LightIntensityVec3Loc = glGetUniformLocation(this->ShaderProgram, "LightIntensity");
	this->LightAmbientVec3Loc = glGetUniformLocation(this->ShaderProgram, "LightAmbient");
	this->LightDiffuseVec3Loc = glGetUniformLocation(this->ShaderProgram, "LightDiffuse");
	this->LightSpecularVec3Loc = glGetUniformLocation(this->ShaderProgram, "LightSpecular");
	this->LightShininessFloatLoc = glGetUniformLocation(this->ShaderProgram, "LightShininess");

	this->MatColorLoc = glGetUniformLocation(this->ShaderProgram, "MatColor");
}

void ColorWorldShader::SetTexture(GLint value) {
	this->SetUniformInt(this->gTextureLoc, value);
}

void ColorWorldShader::SetProjMatrix(const GLfloat* value)
{
	this->SetUniformMatrix4(this->gProjMatrixLoc, value);
}

void ColorWorldShader::SetLightPos(const GLfloat* value)
{
	this->SetUniformVector3(gLightPosLoc, value);
}

void ColorWorldShader::SetModelMatrix(const GLfloat* value)
{
	this->SetUniformMatrix4(gModelMatrixLoc, value);
}

void ColorWorldShader::SetNormalModelMatrix(const GLfloat* value)
{
	this->SetUniformMatrix3(gNormModelMatrixLoc, value);
}

void ColorWorldShader::SetViewMatrix(const GLfloat* value)
{
	this->SetUniformMatrix4(gViewMatrixLoc, value);
}

void ColorWorldShader::SetNormalViewMatrix(const GLfloat* value)
{
	this->SetUniformMatrix3(gNormViewMatrixLoc, value);
}

void ColorWorldShader::SetLightIntensity(const GLfloat* value)
{
	this->SetUniformVector3(LightIntensityVec3Loc, value);
}

void ColorWorldShader::SetLightAmbient(const GLfloat* value)
{
	this->SetUniformVector3(LightAmbientVec3Loc, value);
}

void ColorWorldShader::SetLightDiffuse(const GLfloat* value)
{
	this->SetUniformVector3(LightDiffuseVec3Loc, value);
}

void ColorWorldShader::SetLightSpecular(const GLfloat* value)
{
	this->SetUniformVector3(LightSpecularVec3Loc, value);
}

void ColorWorldShader::SetLightShininess(const GLfloat value)
{
	this->SetUniformFloat(LightShininessFloatLoc, value);
}

void ColorWorldShader::SetDrawSkyBox(GLint i)
{
	this->SetUniformInt(this->DrawSkyBoxLoc, i);
}

void ColorWorldShader::SetCubeMapTexture(GLint value) {
	this->SetUniformInt(this->CubeMapTexLoc, value);
}
