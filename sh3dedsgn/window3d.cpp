
#include "stdafx.h"

DWORD WINAPI RenderThread(void* parm);

Window3DContext g_dctx = { 0 };

GLCONTEXT g_ctx = { 0 };

CRITICAL_SECTION g_csResize;
bool g_IsResized = false;

CRITICAL_SECTION g_csGeometry;

extern CRITICAL_SECTION g_csCubeList;
extern CUBE* cubeList;
extern DWORD numCubes;
extern bool CubeListChanged;

float g_ex = 0.0f, g_ey = 0.0f, g_ez = 0.0f;
float g_az = 0.0f, g_el = 0.0f;

DWORD g_KeysDown = 0;
const DWORD KEY_W = 0x01;
const DWORD KEY_A = 0x02;
const DWORD KEY_S = 0x04;
const DWORD KEY_D = 0x08;
const DWORD KEY_Q = 0x10;
const DWORD KEY_E = 0x20;

bool IsWalking = false;

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;

void GetGlFuncs()
{
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	cout << "Address of glGenBuffers is 0x" << hex << glGenBuffers << dec << endl;
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)wglGetProcAddress("glValidateProgram");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
}

// to get the vector projection of a on to plane
void ProjectVecOnPlaneNormal(glm::vec3 &a, glm::vec3 &n, glm::vec3 &c)
{
	float adotn = glm::dot(a, n);
	float mag2 = powf(glm::length(n), 2.0f);
	c = a - (n * (adotn / mag2));
}

// to get the vector projection of a on to b
void ProjectVecOnVec(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c)
{
	float adotb = glm::dot(a, b);
	float mag2 = powf(glm::length(b), 2.0f);
	c = b * (adotb / mag2);
}

void SetupRenderingContext()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
}

void SetupGeometry()
{
	GLuint buffers[2];

	// can also add normal vectors
	// and colors
	// order is: pos(3), tex(2), normal(3), color(4)
	float points[] = {
		-1.0f, -1.0f, -2.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, //1.0f, 0.0f, 0.0f, 1.0f, // r
		-1.0f,  1.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, //0.0f, 1.0f, 0.0f, 1.0f, // g
		 1.0f,  1.0f, -2.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, //0.0f, 0.0f, 1.0f, 1.0f, // b
		 1.0f, -1.0f, -2.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f //1.0f, 1.0f, 0.0f, 1.0f  // y
	};
	unsigned int indexArray[] = { 0, 1, 2, 0, 2, 3 };

	glGenBuffers(2, buffers);
	g_ctx.VertexArrayBuffer = buffers[0];
	g_ctx.IndexArrayBuffer = buffers[1];
	g_ctx.NumIndices = sizeof(indexArray) / sizeof(unsigned int);

	glBindBuffer(GL_ARRAY_BUFFER, g_ctx.VertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ctx.IndexArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexArray), indexArray, GL_STATIC_DRAW);
}

BOOL SetupShaders()
{
	const char* VertexShaderSource =
		"#version 400\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec2 vTexCoord;"
		"layout(location = 2) in vec3 vNormal;"
		//"layout(location = 3) in vec4 vColor;"
		"uniform mat4 gWorld;"
		"uniform vec3 gLookDirVec;"
		"out vec2 vUV;"
		"out float vCLR;"
		//"out vec4 vVertexColor;"
		"void main() {"
		"  gl_Position = gWorld * vec4(vPosition, 1.0);"
		"  vec3 normgLookDirVec = normalize(gLookDirVec);"
		// if norm and look are facing each other, result will be negative
		// need to invert this to provide positive reflection
		"  vCLR = (clamp(dot(vNormal, normgLookDirVec) * -1.0,0.0,1.0) * 0.8) + 0.2;"
		"  vUV = vTexCoord;"
		//"  vVertexColor = vColor;"
		"}";
	const char* FragmentShaderSource =
		"#version 400\n"
		"out vec4 frag_color;"
		"in vec2 vUV;"
		"in float vCLR;"
		"uniform sampler2D texSampler;"
		"in vec4 vVertexColor;"
		"void main() {"
		"  frag_color = texture2D(texSampler, vUV.xy) * vCLR;"
		//"  frag_color = texture2D(texSampler, vUV.xy) * vVertexColor;"
		"}";

	GLint success = 0;

	g_ctx.ShaderProgram = glCreateProgram();

	g_ctx.VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(g_ctx.VertexShader, 1, &VertexShaderSource, NULL);
	glCompileShader(g_ctx.VertexShader);
	glGetShaderiv(g_ctx.VertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to compile vertex shader";
		return FALSE;
	}

	g_ctx.FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g_ctx.FragmentShader, 1, &FragmentShaderSource, NULL);
	glCompileShader(g_ctx.FragmentShader);
	glGetShaderiv(g_ctx.FragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to compile fragment shader";
		return FALSE;
	}

	glAttachShader(g_ctx.ShaderProgram, g_ctx.VertexShader);
	glAttachShader(g_ctx.ShaderProgram, g_ctx.FragmentShader);
	glLinkProgram(g_ctx.ShaderProgram);

	glGetProgramiv(g_ctx.ShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to link shader program" << endl;
		return FALSE;
	}

	glValidateProgram(g_ctx.ShaderProgram);
	glGetProgramiv(g_ctx.ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to validate shader program" << endl;
		return FALSE;
	}

	glUseProgram(g_ctx.ShaderProgram);
	return TRUE;
}

void SetupTextures()
{

	UINT iw, ih;

	Gdiplus::Bitmap *img = Gdiplus::Bitmap::FromFile(L"croxx.png", FALSE);
	iw = img->GetWidth();
	ih = img->GetHeight();
	Gdiplus::Rect r(0, 0, iw, ih);
	Gdiplus::BitmapData bmpd;
	img->LockBits(&r, Gdiplus::ImageLockModeRead, img->GetPixelFormat(), &bmpd);

	glGenTextures(1, &g_ctx.TexId);
	glBindTexture(GL_TEXTURE_2D, g_ctx.TexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexImage2D(GL_TEXTURE_2D, 0, 4, iw, ih, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, bmpd.Scan0);
	glGenerateMipmap(GL_TEXTURE_2D);

	img->UnlockBits(&bmpd);
	delete img;
}

HGLRC InitOpengl(HWND hwnd, HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 0;
	pfd.cRedShift = 0;
	pfd.cGreenBits = 0;
	pfd.cGreenShift = 0;
	pfd.cBlueBits = 0;
	pfd.cBlueShift = 0;
	pfd.cAlphaBits = 0;
	pfd.cAlphaShift = 0;
	pfd.cAccumBits = 0;
	pfd.cAccumRedBits = 0;
	pfd.cAccumGreenBits = 0;
	pfd.cAccumBlueBits = 0;
	pfd.cAccumAlphaBits = 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.bReserved = 0;
	pfd.dwLayerMask = 0;
	pfd.dwVisibleMask = 0;
	pfd.dwDamageMask = 0;

	int pixfmt = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixfmt, &pfd);

	HGLRC hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);
	const GLubyte* gl_vers = glGetString(GL_VERSION);
	if (gl_vers) {
		cout << "OPEN GL VERSION IS: " << gl_vers << endl;
	}
	const GLubyte* gl_rend = glGetString(GL_RENDERER);
	if (gl_rend) {
		cout << "OPEN GL RENDERER IS: " << gl_rend << endl;
	}
	GetGlFuncs();

	return hrc;
}

void RegisterForRawInput(HWND hWnd, bool reg)
{
	RAWINPUTDEVICE Rid[2];

	// mouse
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0; // RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = hWnd;
	if (!reg) {
		Rid[0].dwFlags = RIDEV_REMOVE;
		Rid[0].hwndTarget = 0;
	}

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = 0; // RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = hWnd;
	if (!reg) {
		Rid[1].dwFlags = RIDEV_REMOVE;
		Rid[1].hwndTarget = 0;
	}

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		//cout << "ERROR: Register for raw failed." << endl;
		ErrorExit((LPTSTR)L"RegisterForRawInput");
	}
}

void HandleRawInput(LPARAM lParam)
{
	UINT dwSize;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, 
		sizeof(RAWINPUTHEADER));
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == NULL) return;

	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize,
		sizeof(RAWINPUTHEADER)) != dwSize)
	{
		OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));
	}

	RAWINPUT* raw = (RAWINPUT*)lpb;

	if (raw->header.dwType == RIM_TYPEKEYBOARD)
	{
		if (raw->data.keyboard.VKey == 87 && raw->data.keyboard.Message == WM_KEYDOWN)
		{
			g_KeysDown |= KEY_W;
		}
		else if (raw->data.keyboard.VKey == 87 && raw->data.keyboard.Message == WM_KEYUP)
		{
			g_KeysDown &= ~KEY_W;
		}

		if (raw->data.keyboard.VKey == 65 && raw->data.keyboard.Message == WM_KEYDOWN)
		{
			g_KeysDown |= KEY_A;
		}
		else if (raw->data.keyboard.VKey == 65 && raw->data.keyboard.Message == WM_KEYUP)
		{
			g_KeysDown &= ~KEY_A;
		}

		if (raw->data.keyboard.VKey == 83 && raw->data.keyboard.Message == WM_KEYDOWN)
		{
			g_KeysDown |= KEY_S;
		}
		else if (raw->data.keyboard.VKey == 83 && raw->data.keyboard.Message == WM_KEYUP)
		{
			g_KeysDown &= ~KEY_S;
		}

		if (raw->data.keyboard.VKey == 68 && raw->data.keyboard.Message == WM_KEYDOWN)
		{
			g_KeysDown |= KEY_D;
		}
		else if (raw->data.keyboard.VKey == 68 && raw->data.keyboard.Message == WM_KEYUP)
		{
			g_KeysDown &= ~KEY_D;
		}

		//q 51
		if (raw->data.keyboard.VKey == 0x51 && raw->data.keyboard.Message == WM_KEYDOWN)
		{
			g_KeysDown |= KEY_Q;
		}
		else if (raw->data.keyboard.VKey == 0x51 && raw->data.keyboard.Message == WM_KEYUP)
		{
			g_KeysDown &= ~KEY_Q;
		}
		//e 45
		if (raw->data.keyboard.VKey == 0x45 && raw->data.keyboard.Message == WM_KEYDOWN)
		{
			g_KeysDown |= KEY_E;
		}
		else if (raw->data.keyboard.VKey == 0x45 && raw->data.keyboard.Message == WM_KEYUP)
		{
			g_KeysDown &= ~KEY_E;
		}
	}
	else if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		g_az += ((float)raw->data.mouse.lLastX * 0.1f);
		if (g_az < 0.0f) g_az += 360.0f;
		if (g_az > 360.0f) g_az -= 360.0f;
		g_el += ((float)raw->data.mouse.lLastY * 0.1f);
		if (g_el < -90.0f) g_el = -90.0f;
		if (g_el > 90.0f) g_el = 90.0f;
	}

	delete[] lpb;
}

LRESULT CALLBACK WndProc3DWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case DSGN_TOGGLE_WALK:
		IsWalking = !IsWalking;
		ShowCursor(!IsWalking);
		RegisterForRawInput(hWnd, IsWalking);
		break;
	case WM_INPUT:
		HandleRawInput(lParam);
		break;
	case WM_CREATE:
		InitializeCriticalSection(&g_csResize);
		InitializeCriticalSection(&g_csGeometry);
		g_dctx.hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		g_dctx.hRenderThread = CreateThread(nullptr, 0, RenderThread, (LPVOID)hWnd, 0, nullptr);
		break;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		break;
	case WM_SIZE:
		EnterCriticalSection(&g_csResize);
		{
			g_IsResized = true;
		}
		LeaveCriticalSection(&g_csResize);
		break;
	case WM_DESTROY:
		if (g_dctx.hStopEvent) {
			SetEvent(g_dctx.hStopEvent);
		}
		if (g_dctx.hRenderThread) {
			WaitForSingleObject(g_dctx.hRenderThread, INFINITE);
			CloseHandle(g_dctx.hRenderThread);
		}
		CloseHandle(g_dctx.hStopEvent);
		DeleteCriticalSection(&g_csResize);
		DeleteCriticalSection(&g_csGeometry);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Refresh3DGeometry()
{

	GLuint buffers[2];

	EnterCriticalSection(&g_csGeometry);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &g_ctx.VertexArrayBuffer);
	glDeleteBuffers(1, &g_ctx.IndexArrayBuffer);

	// for testing this is only the x/y face
	// 4 * 8 = one face
	// 8 * 8 = two faces
	// 12 * 8 = three faces
	// 16 * 8 = four faces
	int CubeDataSize = 24 * 8; // 24 * 8;
	int VertDataSize = CubeDataSize * numCubes;
	float* vertexDataChunk = (float*)malloc(VertDataSize * sizeof(float));
	memset(vertexDataChunk, 0, VertDataSize * sizeof(float));

	// for testing this is only the x/y face
	// 6 = one face
	// 12 = two faces
	// 18 = three faces
	// 24 = four faces
	int CubeIndexSize = 36; // 36;
	int IndexDataSize = CubeIndexSize * numCubes;
	unsigned int* indexDataChunk = (unsigned int*)malloc(IndexDataSize * sizeof(unsigned int));
	memset(indexDataChunk, 0, IndexDataSize * sizeof(unsigned int));

	// can also add normal vectors
	// and colors
	// order is: pos(3), tex(2), normal(3), color(4)
	/*
		float points[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};
		unsigned int indexArray[] = { 0, 1, 2, 0, 2, 3 };
	*/

	for (unsigned int c = 0; c < numCubes; c++) {
		unsigned int* iptr = (unsigned int*)(indexDataChunk + (c * CubeIndexSize));
		float* ptr = (float*)(vertexDataChunk + (c * CubeDataSize));
		CUBE* cb = cubeList + c;

		// face 1 vertices x/y neg z
		// pos x,y,z
		// tex u,v
		// norm i,j,k
		ptr[0] = cb->ox;           ptr[1] = cb->oy;           ptr[2] = cb->oz;  ptr[3] = 0.0f;  ptr[4] = 1.0f;   ptr[5] = 0.0f;  ptr[6] = 0.0f;  ptr[7] = 1.0f;
		ptr[8] = cb->ox;           ptr[9] = cb->oy + cb->dy;  ptr[10] = cb->oz; ptr[11] = 0.0f; ptr[12] = 0.0f;  ptr[13] = 0.0f; ptr[14] = 0.0f; ptr[15] = 1.0f;
		ptr[16] = cb->ox + cb->dx; ptr[17] = cb->oy + cb->dy; ptr[18] = cb->oz; ptr[19] = 1.0f; ptr[20] = 0.0f;  ptr[21] = 0.0f; ptr[22] = 0.0f; ptr[23] = 1.0f;
		ptr[24] = cb->ox + cb->dx; ptr[25] = cb->oy;          ptr[26] = cb->oz; ptr[27] = 1.0f; ptr[28] = 1.0f;  ptr[29] = 0.0f; ptr[30] = 0.0f; ptr[31] = 1.0f;

		// face 1 indexes
		iptr[0] = (c * 24) + 0; iptr[1] = (c * 24) + 2; iptr[2] = (c * 24) + 1;
		iptr[3] = (c * 24) + 0; iptr[4] = (c * 24) + 3; iptr[5] = (c * 24) + 2;

		// face 2 vertices x/y pos z
		// pos x,y,z
		ptr[32] = cb->ox;          ptr[33] = cb->oy;          ptr[34] = cb->oz + cb->dz; ptr[35] = 0.0f; ptr[36] = 1.0f; ptr[37] = 0.0f; ptr[38] = 0.0f; ptr[39] = -1.0f;
		ptr[40] = cb->ox;          ptr[41] = cb->oy + cb->dy; ptr[42] = cb->oz + cb->dz; ptr[43] = 0.0f; ptr[44] = 0.0f; ptr[45] = 0.0f; ptr[46] = 0.0f; ptr[47] = -1.0f;
		ptr[48] = cb->ox + cb->dx; ptr[49] = cb->oy + cb->dy; ptr[50] = cb->oz + cb->dz; ptr[51] = 1.0f; ptr[52] = 0.0f; ptr[53] = 0.0f; ptr[54] = 0.0f; ptr[55] = -1.0f;
		ptr[56] = cb->ox + cb->dx; ptr[57] = cb->oy;          ptr[58] = cb->oz + cb->dz; ptr[59] = 1.0f; ptr[60] = 1.0f; ptr[61] = 0.0f; ptr[62] = 0.0f; ptr[63] = -1.0f;
		
		iptr[6] = (c * 24) + 4; iptr[7] = (c * 24) + 5; iptr[8] = (c * 24) + 6;
		iptr[9] = (c * 24) + 4; iptr[10] = (c * 24) + 6; iptr[11] = (c * 24) + 7;

		// face 3 vertices x/y pos z
		ptr[64] = cb->ox;          ptr[65] = cb->oy; ptr[66] = cb->oz;          ptr[67] = 0.0f; ptr[68] = 1.0f; ptr[69] = 0.0f; ptr[70] = 1.0f; ptr[71] = 0.0f;
		ptr[72] = cb->ox;          ptr[73] = cb->oy; ptr[74] = cb->oz + cb->dz; ptr[75] = 0.0f; ptr[76] = 0.0f; ptr[77] = 0.0f; ptr[78] = 1.0f; ptr[79] = 0.0f;
		ptr[80] = cb->ox + cb->dx; ptr[81] = cb->oy; ptr[82] = cb->oz + cb->dz; ptr[83] = 1.0f; ptr[84] = 0.0f; ptr[85] = 0.0f; ptr[86] = 1.0f; ptr[87] = 0.0f;
		ptr[88] = cb->ox + cb->dx; ptr[89] = cb->oy; ptr[90] = cb->oz;          ptr[91] = 1.0f; ptr[92] = 1.0f; ptr[93] = 0.0f; ptr[94] = 1.0f; ptr[95] = 0.0f;

		// face 3 indexes
		iptr[12] = (c * 24) + 8; iptr[13] = (c * 24) + 9; iptr[14] = (c * 24) + 10;
		iptr[15] = (c * 24) + 8; iptr[16] = (c * 24) + 10; iptr[17] = (c * 24) + 11;

		// face 4 vertices x/y pos z
		ptr[96] = cb->ox;           ptr[97] = cb->oy + cb->dy;  ptr[98] = cb->oz;           ptr[99] = 0.0f;  ptr[100] = 1.0f; ptr[101] = 0.0f; ptr[102] = -1.0f; ptr[103] = 0.0f;
		ptr[104] = cb->ox;          ptr[105] = cb->oy + cb->dy; ptr[106] = cb->oz + cb->dz; ptr[107] = 0.0f; ptr[108] = 0.0f; ptr[109] = 0.0f; ptr[110] = -1.0f; ptr[111] = 0.0f;
		ptr[112] = cb->ox + cb->dx; ptr[113] = cb->oy + cb->dy; ptr[114] = cb->oz + cb->dz; ptr[115] = 1.0f; ptr[116] = 0.0f; ptr[117] = 0.0f; ptr[118] = -1.0f; ptr[119] = 0.0f;
		ptr[120] = cb->ox + cb->dx; ptr[121] = cb->oy + cb->dy; ptr[122] = cb->oz;          ptr[123] = 1.0f; ptr[124] = 1.0f; ptr[125] = 0.0f; ptr[126] = -1.0f; ptr[127] = 0.0f;

		// face 4 indexes
		iptr[18] = (c * 24) + 12; iptr[19] = (c * 24) + 14; iptr[20] = (c * 24) + 13;
		iptr[21] = (c * 24) + 12; iptr[22] = (c * 24) + 15; iptr[23] = (c * 24) + 14;

		// face 4 vertices x/y pos z
		ptr[128] = cb->ox; ptr[129] = cb->oy;          ptr[130] = cb->oz;          ptr[131] = 0.0f; ptr[132] = 1.0f; ptr[133] = -1.0f; ptr[134] = 0.0f; ptr[135] = 0.0f;
		ptr[136] = cb->ox; ptr[137] = cb->oy;          ptr[138] = cb->oz + cb->dz; ptr[139] = 0.0f; ptr[140] = 0.0f; ptr[141] = -1.0f; ptr[142] = 0.0f; ptr[143] = 0.0f;
		ptr[144] = cb->ox; ptr[145] = cb->oy + cb->dy; ptr[146] = cb->oz + cb->dz; ptr[147] = 1.0f; ptr[148] = 0.0f; ptr[149] = -1.0f; ptr[150] = 0.0f; ptr[151] = 0.0f;
		ptr[152] = cb->ox; ptr[153] = cb->oy + cb->dy; ptr[154] = cb->oz;          ptr[155] = 1.0f; ptr[156] = 1.0f; ptr[157] = -1.0f; ptr[158] = 0.0f; ptr[159] = 0.0f;

		// face 4 indexes
		iptr[24] = (c * 24) + 16; iptr[25] = (c * 24) + 18; iptr[26] = (c * 24) + 17;
		iptr[27] = (c * 24) + 16; iptr[28] = (c * 24) + 19; iptr[29] = (c * 24) + 18;

		// face 4 vertices x/y pos z
		ptr[160] = cb->ox + cb->dx; ptr[161] = cb->oy;          ptr[162] = cb->oz;          ptr[163] = 0.0f; ptr[164] = 1.0f; ptr[165] = 1.0f; ptr[166] = 0.0f; ptr[167] = 0.0f;
		ptr[168] = cb->ox + cb->dx; ptr[169] = cb->oy;          ptr[170] = cb->oz + cb->dz; ptr[171] = 0.0f; ptr[172] = 0.0f; ptr[173] = 1.0f; ptr[174] = 0.0f; ptr[175] = 0.0f;
		ptr[176] = cb->ox + cb->dx; ptr[177] = cb->oy + cb->dy; ptr[178] = cb->oz + cb->dz; ptr[179] = 1.0f; ptr[180] = 0.0f; ptr[181] = 1.0f; ptr[182] = 0.0f; ptr[183] = 0.0f;
		ptr[184] = cb->ox + cb->dx; ptr[185] = cb->oy + cb->dy; ptr[186] = cb->oz;          ptr[187] = 1.0f; ptr[188] = 1.0f; ptr[189] = 1.0f; ptr[190] = 0.0f; ptr[191] = 0.0f;

		// face 4 indexes
		iptr[30] = (c * 24) + 20; iptr[31] = (c * 24) + 21; iptr[32] = (c * 24) + 22;
		iptr[33] = (c * 24) + 20; iptr[34] = (c * 24) + 22; iptr[35] = (c * 24) + 23;
	}

	glGenBuffers(2, buffers);
	g_ctx.VertexArrayBuffer = buffers[0];
	g_ctx.IndexArrayBuffer = buffers[1];
	g_ctx.NumIndices = IndexDataSize;

	glBindBuffer(GL_ARRAY_BUFFER, g_ctx.VertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, VertDataSize * sizeof(float), vertexDataChunk, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ctx.IndexArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexDataSize * sizeof(unsigned int), indexDataChunk, GL_STATIC_DRAW);

	LeaveCriticalSection(&g_csGeometry);
}

void ApplyKeys(float FramesPerSecond)
{
	float px = 0.0f, pz = 0.0f, py = 0.0f;
	float EyeAzimuthInRadians = 0.0f;

	float WalkingStride = 200.0f / FramesPerSecond;

	// using standard wsad for movement
	// fwd bck strafe left and strafe right
	if (g_KeysDown & KEY_W) {
		if (g_KeysDown & KEY_A) {
			EyeAzimuthInRadians = DEG2RAD(g_az);
			px = sinf(EyeAzimuthInRadians) - cosf(EyeAzimuthInRadians);
			pz = -cosf(EyeAzimuthInRadians) - sinf(EyeAzimuthInRadians);
		}
		else if (g_KeysDown & KEY_D) {
			EyeAzimuthInRadians = DEG2RAD(g_az);
			px = sinf(EyeAzimuthInRadians) + cosf(EyeAzimuthInRadians);
			pz = -cosf(EyeAzimuthInRadians) + sinf(EyeAzimuthInRadians);
		}
		else {
			EyeAzimuthInRadians = DEG2RAD(g_az);
			px = sinf(EyeAzimuthInRadians);
			pz = -cosf(EyeAzimuthInRadians);
			py = -1.0f * sinf(DEG2RAD(g_el));
		}
	}
	else if (g_KeysDown & KEY_S) {
		if (g_KeysDown & KEY_A) {
			EyeAzimuthInRadians = DEG2RAD(g_az);
			px = -sinf(EyeAzimuthInRadians) - cosf(EyeAzimuthInRadians);
			pz = cosf(EyeAzimuthInRadians) - sinf(EyeAzimuthInRadians);
		}
		else if (g_KeysDown & KEY_D) {
			EyeAzimuthInRadians = DEG2RAD(g_az);
			px = -sinf(EyeAzimuthInRadians) + cosf(EyeAzimuthInRadians);
			pz = cosf(EyeAzimuthInRadians) + sinf(EyeAzimuthInRadians);
		}
		else {
			EyeAzimuthInRadians = DEG2RAD(g_az);
			px = -sinf(EyeAzimuthInRadians);
			pz = cosf(EyeAzimuthInRadians);
			py = sinf(DEG2RAD(g_el));
		}
	}
	else if (g_KeysDown & KEY_A) {
		EyeAzimuthInRadians = DEG2RAD(g_az);
		px = -cosf(EyeAzimuthInRadians);
		pz = -sinf(EyeAzimuthInRadians);
	}
	else if (g_KeysDown & KEY_D) {
		EyeAzimuthInRadians = DEG2RAD(g_az);
		px = cosf(EyeAzimuthInRadians);
		pz = sinf(EyeAzimuthInRadians);
	}

	g_ex += WalkingStride * px;
	g_ey += WalkingStride * py;
	g_ez += WalkingStride * pz;

}

DWORD WINAPI RenderThread(void* parm)
{
	HWND hWnd = (HWND)parm;
	HDC hdc = nullptr;
	HGLRC hglrc = nullptr;
	RECT clientRect = { 0 };
	GLuint gWorldMatrixLoc = 0;
	GLuint gLookDirVecLoc = 0;
	GLuint gTextureLoc = 0;
	LARGE_INTEGER perfFreq = { 0 };
	LARGE_INTEGER perfCount = { 0 };
	LONGLONG lastCount = 0;
	int ctr = 0;
	float FramesPerSecond = 0.0f;

	QueryPerformanceFrequency(&perfFreq);

	hdc = GetDC(hWnd);
	hglrc = InitOpengl(hWnd, hdc);

	SetupRenderingContext();
	SetupGeometry();
	SetupShaders();
	SetupTextures();

	GetClientRect(hWnd, &clientRect);
	glm::mat4 proj = glm::perspective(45.0f,
		(float)(clientRect.right - clientRect.left) / (float)(clientRect.bottom - clientRect.top),
		0.1f, 1000.0f);
	glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	gWorldMatrixLoc = glGetUniformLocation(g_ctx.ShaderProgram, "gWorld");
	gLookDirVecLoc = glGetUniformLocation(g_ctx.ShaderProgram, "gLookDirVec");
	gTextureLoc = glGetUniformLocation(g_ctx.ShaderProgram, "texSampler");
	glUniform1i(gTextureLoc, 0);

	while (TRUE) {

		QueryPerformanceCounter(&perfCount);
		FramesPerSecond = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		lastCount = perfCount.QuadPart;

		if (WAIT_OBJECT_0 != WaitForSingleObject(g_dctx.hStopEvent, 0)) {

			ApplyKeys(FramesPerSecond);

			if(TryEnterCriticalSection(&g_csResize) > 0)
			{
				if (true == g_IsResized)
				{
					GetClientRect(hWnd, &clientRect);
					proj = glm::perspective(45.0f,
						(float)(clientRect.right - clientRect.left) / (float)(clientRect.bottom - clientRect.top),
						0.1f, 1000.0f);
					glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
					g_IsResized = false;
				}
				LeaveCriticalSection(&g_csResize);
			}

			if (TryEnterCriticalSection(&g_csCubeList)) {
				if (CubeListChanged == true) {
					Refresh3DGeometry();
					CubeListChanged = false;
				}
				LeaveCriticalSection(&g_csCubeList);
			}

			if (IsWalking) {
				glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
			}
			else {
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::vec3 lookDirVec(sinf(DEG2RAD(g_az)), sinf(DEG2RAD(g_el)), cosf(DEG2RAD(g_az)));
			glUniform3fv(gLookDirVecLoc, 1, &lookDirVec[0]);

			glm::mat4 r1 = glm::rotate(DEG2RAD(g_az), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 r2 = glm::rotate(DEG2RAD(g_el), glm::vec3(cosf(DEG2RAD(g_az)), 0.0f, sinf(DEG2RAD(g_az))));
			glm::mat4 t = glm::translate(glm::vec3(-g_ex, -g_ey, -g_ez));
			glm::mat4 modelmat = r1 * r2 * t;
			glm::mat4 final = proj * modelmat;

			glUniformMatrix4fv(gWorldMatrixLoc, 1, GL_FALSE, &final[0][0]);

			//glm::vec3 player(g_ex, g_ey, g_ez);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_ctx.TexId);

			// only do if vertex data
			if(TryEnterCriticalSection(&g_csGeometry) > 0)
			{
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				//glEnableVertexAttribArray(3);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
				//glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float)));
				glDrawElements(GL_TRIANGLES, g_ctx.NumIndices, GL_UNSIGNED_INT, 0);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
				//glDisableVertexAttribArray(3);
				LeaveCriticalSection(&g_csGeometry);
			}
			// end stuff

			SwapBuffers(hdc);

		}
		else {
			break;
		}

		ctr++;
		if (ctr == 1000)
		{
			ctr = 0;
			cout << "\r" << FramesPerSecond << " fps     " << flush;
		}
	}

	glDeleteShader(g_ctx.VertexShader);
	glDeleteShader(g_ctx.FragmentShader);
	glDeleteProgram(g_ctx.ShaderProgram);
	glDeleteBuffers(1, &g_ctx.VertexArrayBuffer);
	glDeleteBuffers(1, &g_ctx.IndexArrayBuffer);
	glDeleteTextures(1, &g_ctx.TexId);

	if (hglrc) {
		wglMakeCurrent(hdc, nullptr);
		wglDeleteContext(hglrc);
	}
	if (hdc) ReleaseDC(hWnd, hdc);

	cout << "Quitting render thread" << endl;
	return 0;
}

