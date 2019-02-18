#pragma once

class TextureContext
{
public:
	TextureContext(const wchar_t* filename);
	~TextureContext();
	void Bind(GLenum TextureUnit);
private:
	GLuint TexId;
};

