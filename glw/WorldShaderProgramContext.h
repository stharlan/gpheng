#pragma once

class WorldShaderProgramContext : public ShaderProgramContext
{
public:
	WorldShaderProgramContext();
	~WorldShaderProgramContext();
	void ResolveLocations();
	void SetTexture(GLint value);
	void SetProjMatrix(const GLfloat* value);
	void SetLightPos(const GLfloat* value);
	void SetModelMatrix(const GLfloat* value);
	void SetNormalModelMatrix(const GLfloat* value);
	void SetViewMatrix(const GLfloat* value);
	void SetNormalViewMatrix(const GLfloat* value);

	void SetLightIntensity(const GLfloat* value);
	void SetLightAmbient(const GLfloat* value);
	void SetLightDiffuse(const GLfloat* value);
	void SetLightSpecular(const GLfloat* value);
	void SetLightShininess(const GLfloat value);

	void SetDrawSkyBox(GLint i);
	void SetCubeMapTexture(GLint value);

private:
	GLuint gModelMatrixLoc;
	GLuint gViewMatrixLoc;
	GLuint gProjMatrixLoc;
	GLuint gLightPosLoc;
	GLuint gNormViewMatrixLoc;
	GLuint gNormModelMatrixLoc;
	GLuint gTextureLoc;

	GLuint LightIntensityVec3Loc;
	GLuint LightAmbientVec3Loc;
	GLuint LightDiffuseVec3Loc;
	GLuint LightSpecularVec3Loc;
	GLuint LightShininessFloatLoc;

	GLuint DrawSkyBoxLoc;
	GLuint CubeMapTexLoc;
};
