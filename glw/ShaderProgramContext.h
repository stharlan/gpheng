#pragma once

class ShaderProgramContext
{
public:
	ShaderProgramContext();
	ShaderProgramContext(const char* VertexShaderSource, const char* FragmentShaderSource);
	~ShaderProgramContext();
	void FreeResources();
	void BuildFromSource(const char* VertexShaderSource, const char* FragmentShaderSource);
	void SetAsCurrent();
	void SetUniformInt(GLuint loc, GLint value);
	void SetUniformVector3(GLuint loc, const GLfloat* value);
	void SetUniformMatrix4(GLuint loc, const GLfloat* value);
	void SetUniformMatrix3(GLuint loc, const GLfloat* value);
	void SetUniformFloat(GLuint loc, GLfloat value);
	void SetUniformVector4(GLuint loc, const GLfloat* value);
private:
	GLuint VertexShader;
	GLuint FragmentShader;

protected:
	GLuint ShaderProgram;

private:
	BOOL SetupShaders(const char* vtxss, const char* frgss);
};
