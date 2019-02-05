#pragma once

#include "resource.h"

#define DSGN_TOGGLE_WALK WM_USER+1

const int EDITOR_MODE_DEFAULT = 0;
const int EDITOR_MODE_BOX = 0;

typedef struct {
	HANDLE hRenderThread = nullptr;
	HANDLE hStopEvent = nullptr;
} Window3DContext;

typedef struct {
	float grid;
	float tx;
	float ty;
	float scale;
	RECT sel;
	//RECT ulhndl;
	//RECT urhndl;
	//RECT llhndl;
	//RECT lrhndl;
	HBITMAP bmpDbuf;
} WindowOrthoContext;

//const int ULHANDLE = 1;
//const int URHANDLE = 2;
//const int LLHANDLE = 3;
//const int LRHANDLE = 4;

typedef struct {
	UUID id;
	float ox;
	float oy;
	float oz;
	float dx;
	float dy;
	float dz;
} CUBE;

typedef struct {
	GLuint VertexArrayBuffer;
	GLuint IndexArrayBuffer;
	GLuint NumIndices;
	GLuint TexId;
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint ShaderProgram;
} GLCONTEXT;

void ErrorExit(LPTSTR lpszFunction);

LRESULT CALLBACK WndProc3DWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcOrthoWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool CommitCube();
CUBE* getSelectedCube();