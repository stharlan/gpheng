#pragma once

class TextureContext
{
public:
	TextureContext(const wchar_t* filename, bool Is24Bit = false);
	~TextureContext();
	void Bind(GLenum TextureUnit);
private:
	GLuint TexId;
};

