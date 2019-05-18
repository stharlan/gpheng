#include "stdafx.h"
#include "TextureContext.h"

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


TextureContext::TextureContext(const wchar_t* filename, bool Is24Bit)
{
	UINT iw, ih;
	const GLsizei numMipMaps = 3;

	Gdiplus::Bitmap img(filename);
	iw = img.GetWidth();
	ih = img.GetHeight();
	Gdiplus::Rect r(0, 0, iw, ih);
	Gdiplus::BitmapData bmpd;
	img.LockBits(&r, Gdiplus::ImageLockModeRead, img.GetPixelFormat(), &bmpd);

	// 1. gen and bind
	glGenTextures(1, &this->TexId);
	glBindTexture(GL_TEXTURE_2D, this->TexId);
	glTexStorage2D(GL_TEXTURE_2D, numMipMaps, GL_RGBA8, iw, ih);
	// 32 bit png
	//glTexImage2D(GL_TEXTURE_2D, 0, 4, iw, ih, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
	// 24 bit png
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, bmpd.Scan0);
	if (true == Is24Bit) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iw, ih, GL_BGR, GL_UNSIGNED_BYTE, bmpd.Scan0);
	}
	else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iw, ih, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	img.UnlockBits(&bmpd);
}


TextureContext::~TextureContext()
{
	glDeleteTextures(1, &this->TexId);
}

void TextureContext::Bind(GLenum TextureUnit)
{
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_2D, this->TexId);
}