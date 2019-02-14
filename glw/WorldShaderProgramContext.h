#pragma once

class WorldShaderProgramContext : public ShaderProgramContext
{
public:
	WorldShaderProgramContext();
	~WorldShaderProgramContext();
	void ResolveLocations();
	void SetTexture(GLint value);
	void SetProjMatrix(const GLfloat* value);
	void SetPlayerPos(const GLfloat* value);
	void SetModelMatrix(const GLfloat* value);
	void SetNormalModelMatrix(const GLfloat* value);
	void SetViewMatrix(const GLfloat* value);
	void SetNormalViewMatrix(const GLfloat* value);

private:
	GLuint gModelMatrixLoc;
	GLuint gViewMatrixLoc;
	GLuint gProjMatrixLoc;
	GLuint gPlayerPosLoc;
	GLuint gNormViewMatrixLoc;
	GLuint gNormModelMatrixLoc;
	GLuint gTextureLoc;
};
