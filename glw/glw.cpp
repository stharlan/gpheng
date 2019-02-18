
#include "stdafx.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Physx_64.lib")
#pragma comment(lib, "PhysxFoundation_64.lib")
#pragma comment(lib, "PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "PhysxCommon_64.lib")
#pragma comment(lib, "PhysxCooking_64.lib")
#pragma comment(lib, "PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")

using namespace std;

typedef BOOL(*PFNwglSwapIntervalEXT)(int interval);
typedef int(*PFNwglGetSwapIntervalEXT)(void);

PFNwglSwapIntervalEXT wglSwapIntervalEXT;
PFNwglGetSwapIntervalEXT wglGetSwapIntervalEXT;

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
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLTEXSTORAGE2DPROC glTexStorage2D;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM4FVPROC glUniform4fv;

typedef BOOL(*PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc,
	const int *piAttribIList,
	const FLOAT *pfAttribFList,
	UINT nMaxFormats,
	int *piFormats,
	UINT *nNumFormats);
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

typedef const char *(*PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC hdc);
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;

typedef HGLRC(*PFNWGLCREATECONTEXTATTRIBSARB)(HDC hDC, HGLRC hshareContext, const int *attribList);
PFNWGLCREATECONTEXTATTRIBSARB wglCreateContextAttribsARB;

int g_OpenGlAttributes[] = {
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_DEPTH_BITS_ARB, 24,
	WGL_STENCIL_BITS_ARB, 8,
	WGL_ALPHA_BITS_ARB, 8,
	WGL_SAMPLE_BUFFERS_ARB, 1,
	WGL_SAMPLES_ARB, 4,                        // Check For 4x Multisampling
	0, 0
};

bool    arbMultisampleSupported = false;
int arbMultisampleFormat = 0;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcTest(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HGLRC InitOpengl(HWND hwnd, HDC hdc);
void DumpGlErrors(const char* FunctionName);
void ClearGLError();

HANDLE hRenderThread = nullptr;
HANDLE hStopEvent = nullptr;

float g_ex = 50.0f, g_ey = 110.0f, g_ez = 50.0f;
float g_az = 0.0f, g_el = 0.0f;

DWORD g_KeysDown = 0;
const DWORD KEY_W = 0x01;
const DWORD KEY_A = 0x02;
const DWORD KEY_S = 0x04;
const DWORD KEY_D = 0x08;
const DWORD KEY_Q = 0x10;
const DWORD KEY_E = 0x20;
const DWORD KEY_MOUSE_LB = 0x40;

const char* g_VtxShaderScreen = "#version 150\n"
"in vec2 position;"
"void main()"
"{"
"  gl_Position = vec4(position, 0.0, 1.0);"
"}";

const char* g_FrgShaderScreen = "#version 150\n"
"out vec4 outColor;"
"void main()"
"{"
"  outColor = vec4(1.0, 1.0, 1.0, 1.0);"
"}";

class ScreenShaderProgramContext : public ShaderProgramContext
{
public:
	ScreenShaderProgramContext() {
		this->BuildFromSource(g_VtxShaderScreen, g_FrgShaderScreen);
		this->pos = glGetAttribLocation(this->ShaderProgram, "position");
	}
	~ScreenShaderProgramContext() {}
	GLuint getPos() { return this->pos; }
private:
	GLuint pos;
};

typedef struct {
	WorldShaderProgramContext* lpWorldShader;
	ScreenShaderProgramContext* lpScreenShader;
	//GLuint TexId;
	//IndexedTriangleList* lpIdtFile;
	IndexedTriangleList* lpIdtFallingCube;
	IndexedTriangleList* lpIdtBulletBox;
	GLuint fontBase;
	GLuint CubeMapTexId;
} GLCONTEXT;

class MyAllocator : public physx::PxAllocatorCallback
{
public:
	MyAllocator() {}
	~MyAllocator() {}
	void* allocate(size_t size, const char* typeName, const char* filename,
		int line)
	{
		return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	}
	void deallocate(void* ptr)
	{
		HeapFree(GetProcessHeap(), 0, ptr);
	}
};

class UserErrorCallback : public physx::PxErrorCallback
{
public:
	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file,
		int line)
	{
		cout << "PHYSX: REPORT ERROR: CODE: " << code << " MSG: " << message << " FILE: " << file << " LINE: " << line << endl;
	}
};

struct {
	MyAllocator gAallocator;
	UserErrorCallback errcbk;
	physx::PxFoundation* mFoundation = nullptr;
	physx::PxPvd* mPvd = nullptr;
	physx::PxPvdTransport* transport = nullptr;
	physx::PxPhysics* mPhysics = nullptr;
	physx::PxScene* gScene = nullptr;
	physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;
	physx::PxMaterial* gMaterial = nullptr;
	physx::PxRigidStatic* groundPlane = nullptr;
	physx::PxControllerManager *cmanager = nullptr;
	physx::PxController *pChar = nullptr;
	physx::PxCooking* cooking = nullptr;

	physx::PxHeightField* aHeightField = nullptr;
	physx::PxRigidStatic* actor = nullptr;
	physx::PxShape* aHeightFieldShape = nullptr;
} PhysxContext;

bool WGLisExtensionSupported(const char *extension)
{
	const size_t extlen = strlen(extension);
	const char *supported = NULL;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	wglGetExtensionsStringARB =
		(PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	cout << "wglGetExtensionsStringARB addr = " << hex << wglGetExtensionsStringARB << dec << endl;

	if (wglGetExtensionsStringARB) {
		supported = wglGetExtensionsStringARB(wglGetCurrentDC());
	}

	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL) {
		supported = (char*)glGetString(GL_EXTENSIONS);
	}

	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
	{
		return false;
	}

	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{

		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL) {
			return false;                       // No Match
		}

		// Make Sure That Match Is At The Start Of The String Or That
		// The Previous Char Is A Space, Or Else We Could Accidentally
		// Match "wglFunkywglExtension" With "wglExtension"

		// Also, Make Sure That The Following Character Is Space Or NULL
		// Or Else "wglExtensionTwo" Might Match "wglExtension"
		if ((p == supported || p[-1] == ' ') && (p[extlen] == '\0' || p[extlen] == ' '))
			return true;                        // Match
	}
}

bool InitMultisample(HWND hWnd)
{
	// See If The String Exists In WGL!
	if (!WGLisExtensionSupported("WGL_ARB_multisample"))
	{
		arbMultisampleSupported = false;
		return false;
	}

	// Get Our Pixel Format
	wglChoosePixelFormatARB =
		(PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB =
		(PFNWGLCREATECONTEXTATTRIBSARB)wglGetProcAddress("wglCreateContextAttribsARB");

	if (!wglChoosePixelFormatARB)
	{
		// We Didn't Find Support For Multisampling, Set Our Flag And Exit Out.
		arbMultisampleSupported = false;
		return false;
	}

	// Get Our Current Device Context. We Need This In Order To Ask The OpenGL Window What Attributes We Have
	HDC hDC = GetDC(hWnd);

	int pixelFormat;
	bool valid;
	UINT numFormats;
	float fAttributes[] = { 0,0 };

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC, g_OpenGlAttributes, fAttributes, 1, &pixelFormat, &numFormats);

	// if We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;
		return arbMultisampleSupported;
	}

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	g_OpenGlAttributes[21] = 2;
	valid = wglChoosePixelFormatARB(hDC, g_OpenGlAttributes, fAttributes, 1, &pixelFormat, &numFormats);
	if (valid && numFormats >= 1)
	{
		arbMultisampleSupported = true;
		arbMultisampleFormat = pixelFormat;
		return arbMultisampleSupported;
	}

	// Return The Valid Format
	return  arbMultisampleSupported;
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

void *GetAnyGLFuncAddress(const char *name)
{
	void *p = (void *)wglGetProcAddress(name);
	if (p == 0 ||
		(p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
		(p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void *)GetProcAddress(module, name);
	}

	return p;
}

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
	glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)wglGetProcAddress("glTexStorage2D");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	wglSwapIntervalEXT = (PFNwglSwapIntervalEXT)wglGetProcAddress("wglSwapIntervalEXT");
	wglGetSwapIntervalEXT  = (PFNwglGetSwapIntervalEXT)wglGetProcAddress("wglGetSwapIntervalEXT");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
}

void SetupRenderingContext()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (true == arbMultisampleSupported)
	{
		cout << "Enabling multi sample" << endl;
		glEnable(GL_MULTISAMPLE);
	}
	else {
		cout << "Disabling multi sample" << endl;
		glDisable(GL_MULTISAMPLE);
	}

	GLint val = 0;
	glGetIntegerv(SAMPLE_BUFFERS_ARB, &val);
	cout << "Sample buffers ARB = " << val << endl;

	glGetIntegerv(SAMPLES_ARB, &val);
	cout << "Samples ARB = " << val << endl;

	GLboolean isMSEnabled = glIsEnabled(GL_MULTISAMPLE);
	cout << "Multi sample enabled = " << (isMSEnabled == GL_TRUE) << endl;
}

physx::PxRigidDynamic* createDynamic(const physx::PxTransform& t, 
	const physx::PxGeometry& geometry, 
	const physx::PxVec3& velocity = physx::PxVec3(0))
{
	physx::PxRigidDynamic* dynamic = PxCreateDynamic(*PhysxContext.mPhysics, t, geometry, *PhysxContext.gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	PhysxContext.gScene->addActor(*dynamic);
	return dynamic;
}

void AddDynamicActor(IndexedTriangleList& trilist, physx::PxVec3 pos, physx::PxVec3 size)
{
	// add an actor for the cube

	//physx::PxVec3 position(0, 40, 0);
	//float radius = 1.0f;
	//float halfHeight = 2.0f;
	physx::PxRigidDynamic* lpDyn = PhysxContext.mPhysics->createRigidDynamic(physx::PxTransform(pos));
	// transform that rotates 90 deg about the z axis
	//physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	//aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*aCapsuleActor,
		//physx::PxCapsuleGeometry(radius, halfHeight), *gMaterial);
	physx::PxShape* lpShp = physx::PxRigidActorExt::createExclusiveShape(
		*lpDyn,
		//physx::PxBoxGeometry(3, 3, 3), *gMaterial);
		physx::PxBoxGeometry(size), *PhysxContext.gMaterial);
	//aCapsuleShape->setLocalPose(relativePose);
	physx::PxRigidBodyExt::updateMassAndInertia(*lpDyn, 10.0f);
	PhysxContext.gScene->addActor(*lpDyn);
	trilist.SetRigidDynamic(lpDyn, lpShp);
}

void AddStaticActor(IndexedTriangleList& trilist, physx::PxVec3 pos, physx::PxVec3 size)
{
	//physx::PxVec3 pos(
		//cube->ox + (cube->dx / 2.0f),
		//cube->oy + (cube->dy / 2.0f),
		//cube->oz + (cube->dz / 2.0f));
	physx::PxRigidStatic* lpStatic = PhysxContext.mPhysics->createRigidStatic(physx::PxTransform(pos));
	physx::PxShape* lpShp = physx::PxRigidActorExt::createExclusiveShape(
		*lpStatic,
		//physx::PxBoxGeometry(cube->dx / 2.0f, cube->dy / 2.0f, cube->dz / 2.0f),
		physx::PxBoxGeometry(size),
		*PhysxContext.gMaterial);
	PhysxContext.gScene->addActor(*lpStatic);
	trilist.SetRigidStatic(lpStatic, lpShp);
}


void SetupCubeMap(GLCONTEXT* lpGlContext)
{
	// gen and bind
	glGenTextures(1, &lpGlContext->CubeMapTexId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, lpGlContext->CubeMapTexId);
	const wchar_t* files[] = {
		L"skyr.png",L"skyl.png",L"skyu.png",L"skyd.png",L"skyc.png",L"skyrr.png"
	};
	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	for (int i = 0; i < 6; i++) {
		Gdiplus::Bitmap img(files[i]);
		UINT iw = img.GetWidth();
		UINT ih = img.GetHeight();
		Gdiplus::Rect r(0, 0, iw, ih);
		Gdiplus::BitmapData bmpd;
		img.LockBits(&r, Gdiplus::ImageLockModeRead, img.GetPixelFormat(), &bmpd);
		glTexImage2D(targets[i], 0, GL_RGB, iw, ih, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
		img.UnlockBits(&bmpd);
		DumpGlErrors("SetupCubeMap");
	}

	// Typical cube map settings
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Set the CubeMapTex uniform to texture unit 0
	//uniloc = glGetUniformLocation(programHandle, "CubeMapTex");
	//if (uniloc >= 0) glUniform1i(uniloc, 0);
}

//void SetupTextures(GLCONTEXT* lpGlContext)
//{
//	UINT iw, ih;
//	const GLsizei numMipMaps = 3;
//
//	//Gdiplus::Bitmap *img = Gdiplus::Bitmap::FromFile(L"kitchtilec.png", FALSE);
//	Gdiplus::Bitmap img(L"kitchtilec.png");
//	iw = img.GetWidth();
//	ih = img.GetHeight();
//	Gdiplus::Rect r(0, 0, iw, ih);
//	Gdiplus::BitmapData bmpd;
//	img.LockBits(&r, Gdiplus::ImageLockModeRead, img.GetPixelFormat(), &bmpd);
//
//	// 1. gen and bind
//	glGenTextures(1, &lpGlContext->TexId);
//	glBindTexture(GL_TEXTURE_2D, lpGlContext->TexId);
//	glTexStorage2D(GL_TEXTURE_2D, numMipMaps, GL_RGBA8, iw, ih);
//	// 32 bit png
//	//glTexImage2D(GL_TEXTURE_2D, 0, 4, iw, ih, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
//	// 24 bit png
//	//glTexImage2D(GL_TEXTURE_2D, 0, 3, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, bmpd.Scan0);
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iw, ih, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
//	glGenerateMipmap(GL_TEXTURE_2D);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//
//	img.UnlockBits(&bmpd);
//
//	DumpGlErrors("SetupTextures");
//}

void ApplyKeys(float FramesPerSecond, float* mx, float* my, float* mz)
{
	float px = 0.0f, pz = 0.0f, py = 0.0f;
	float EyeAzimuthInRadians = 0.0f;

	float WalkingStride = 25.0f / FramesPerSecond;

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

	*mx = WalkingStride * px;
	// this falling displacement causes
	// the player to fall at a constant (non-accelerating) rate
	// it's calculated based on gravity being -9.81 m/s
	// where, over a period of 1/60th sec, the player will
	// accelerate to a velocity of -0.1635f per 60th of a second
	*my = -0.1635f;
	*mz = WalkingStride * pz;
}

bool InitializePhysx()
{
	PhysxContext.mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, PhysxContext.gAallocator, PhysxContext.errcbk);
	if (!PhysxContext.mFoundation) {
		cout << "PxCreateFoundation failed!" << endl;
	}

	bool recordMemoryAllocations = true;

	PhysxContext.mPvd = physx::PxCreatePvd(*PhysxContext.mFoundation);
	if (!PhysxContext.mPvd) {
		cout << "failed to create pvd" << endl;
	}
	PhysxContext.transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	if (!PhysxContext.transport) {
		cout << "failed to create transport" << endl;
	}
	PhysxContext.mPvd->connect(*PhysxContext.transport, physx::PxPvdInstrumentationFlag::eALL);

	PhysxContext.mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *PhysxContext.mFoundation,
		physx::PxTolerancesScale(), recordMemoryAllocations, PhysxContext.mPvd);
	if (!PhysxContext.mPhysics) {
		cout << "PxCreatePhysics failed!" << endl;
	}

	if (!PxInitExtensions(*PhysxContext.mPhysics, PhysxContext.mPvd))
	{
		cout << "PxInitExtensions failed!" << endl;
	}

	physx::PxSceneDesc sceneDesc(PhysxContext.mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	PhysxContext.gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	if (!PhysxContext.gDispatcher) {
		cout << "failed to create dispatcher" << endl;
	}
	sceneDesc.cpuDispatcher = PhysxContext.gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	PhysxContext.gScene = PhysxContext.mPhysics->createScene(sceneDesc);
	if (!PhysxContext.gScene) {
		cout << "failed to create scene" << endl;
	}

	PhysxContext.gMaterial = PhysxContext.mPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PhysxContext.cmanager = PxCreateControllerManager(*PhysxContext.gScene);
	physx::PxCapsuleControllerDesc desc;
	desc.height = 4.0f;
	desc.material = PhysxContext.gMaterial;
	desc.position = physx::PxExtendedVec3(g_ex, g_ey + 5.0f, g_ez);
	desc.radius = 1.0f;
	PhysxContext.pChar = PhysxContext.cmanager->createController(desc);

	PhysxContext.cooking = PxCreateCooking(PX_PHYSICS_VERSION, 
		*PhysxContext.mFoundation, 
		physx::PxCookingParams(physx::PxTolerancesScale()));
	if (!PhysxContext.cooking) {
		cout << "PxCreateCooking failed!" << endl;
	}

	return true;
}

physx::PxMat44 PhysxSimulate(physx::PxRigidDynamic* pFallingCubeDynamic, BULLET_STRUCT* bullets)
{
	physx::PxShape* shapes[1];
	PhysxContext.gScene->simulate(1.0f / 60.0f); // 60th of a sec (60 fps)
	PhysxContext.gScene->fetchResults(true);
	physx::PxU32 n = pFallingCubeDynamic->getNbShapes();
	pFallingCubeDynamic->getShapes(shapes, n);
	const physx::PxMat44 shapePose(physx::PxShapeExt::getGlobalPose(*shapes[0], *pFallingCubeDynamic));

	for (int b = 0; b < 10; b++) {
		if (bullets[b].lpBulletDyn)
		{
			bullets[b].lpBulletDyn->getShapes(shapes, 1);
			bullets[b].pose = physx::PxShapeExt::getGlobalPose(*shapes[0], *bullets[b].lpBulletDyn);
		}
	}

	return shapePose;
}

void DisposePhysx()
{
	if (PhysxContext.aHeightFieldShape) PhysxContext.aHeightFieldShape->release();
	if (PhysxContext.actor) PhysxContext.actor->release();
	if (PhysxContext.aHeightField) PhysxContext.aHeightField->release();
	if (PhysxContext.cooking) PhysxContext.cooking->release();
	if (PhysxContext.pChar) PhysxContext.pChar->release();
	if (PhysxContext.cmanager) PhysxContext.cmanager->release();
	if (PhysxContext.gMaterial) PhysxContext.gMaterial->release();
	if (PhysxContext.groundPlane) PhysxContext.groundPlane->release();
	if (PhysxContext.gScene) PhysxContext.gScene->release();
	if (PhysxContext.gDispatcher) PhysxContext.gDispatcher->release();
	if (PhysxContext.mPhysics) PhysxContext.mPhysics->release();
	if (PhysxContext.transport) PhysxContext.transport->release();
	if (PhysxContext.mPvd) PhysxContext.mPvd->release();
	if (PhysxContext.mFoundation) PhysxContext.mFoundation->release();
}


BOOL InitGlFont(HDC hdc, GLCONTEXT *lpContext)
{
	ClearGLError();
	lpContext->fontBase = glGenLists(96);
	if (glGetError()) return FALSE;
	return wglUseFontBitmaps(hdc, 32, 96, lpContext->fontBase);
}

void FreeGlFont(GLCONTEXT *lpContext)
{
	glDeleteLists(lpContext->fontBase, 96);
}


IndexedTriangleList CreateFallingCube()
{
	CUBE fallingCube = { -3.0f, -3.0f, -3.0f, 6.0f, 6.0f, 6.0f };

	IndexedTriangleList triList = IndexedTriangleList::CreateCubes(1, &fallingCube);
	AddDynamicActor(triList, physx::PxVec3(0, 40, 0), physx::PxVec3(3, 3, 3));

	return triList;
}

CUBE* LoadGeometryJson(const char* filename, unsigned int* numCubes)
{

	CUBE* lpCubes = nullptr;
	unsigned int nCubes = 0;

	FILE* jsonFile = nullptr;
	fopen_s(&jsonFile, filename, "r");

	fseek(jsonFile, 0, SEEK_END);
	long jsonFileLength = ftell(jsonFile);
	fseek(jsonFile, 0, SEEK_SET);

	char* jsonBuffer = (char*)malloc(jsonFileLength + 1);
	memset(jsonBuffer, 0, jsonFileLength + 1);
	fread(jsonBuffer, jsonFileLength, 1, jsonFile);
	fclose(jsonFile);

	rapidjson::Document jsonDoc;
	jsonDoc.Parse(jsonBuffer);

	rapidjson::Value &boxesValue = jsonDoc["boxes"];

	if (boxesValue.IsArray()) {

		rapidjson::Value::Array boxesArray = boxesValue.GetArray();

		if (boxesArray.Size() > 0) {

			lpCubes = (CUBE*)malloc(boxesArray.Size() * sizeof(CUBE));
			memset(lpCubes, 0, boxesArray.Size() * sizeof(CUBE));
			nCubes = boxesArray.Size();

			for (unsigned int BoxId = 0; BoxId < boxesArray.Size(); BoxId++) {

				rapidjson::Value &BoxParmsValue = boxesArray[BoxId];

				if (BoxParmsValue.IsArray()) {

					rapidjson::Value::Array ParmsArray = BoxParmsValue.GetArray();
					if (ParmsArray.Size() == 6) {
						lpCubes[BoxId].ox = ParmsArray[0].GetFloat();
						lpCubes[BoxId].oy = ParmsArray[1].GetFloat();
						lpCubes[BoxId].oz = ParmsArray[2].GetFloat();
						lpCubes[BoxId].dx = ParmsArray[3].GetFloat();
						lpCubes[BoxId].dy = ParmsArray[4].GetFloat();
						lpCubes[BoxId].dz = ParmsArray[5].GetFloat();
					}
				}
			}
		}
	}

	free(jsonBuffer);

	*numCubes = nCubes;
	return lpCubes;
}

IndexedTriangleList LoadAndProcessGeometryFile()
{
	unsigned int jsonCubeNum = 0;
	CUBE* jsonCubes = nullptr;

	jsonCubes = LoadGeometryJson("mzf.json", &jsonCubeNum);
	IndexedTriangleList triList = IndexedTriangleList::CreateCubes(jsonCubeNum, jsonCubes);

	for (unsigned int c = 0; c < jsonCubeNum; c++) {
		AddStaticActor(triList,
			physx::PxVec3(
				jsonCubes[c].ox + (jsonCubes[c].dx / 2.0f),
				jsonCubes[c].oy + (jsonCubes[c].dy / 2.0f),
				jsonCubes[c].oz + (jsonCubes[c].dz / 2.0f)
			),
			physx::PxVec3(jsonCubes[c].dx / 2.0f, jsonCubes[c].dy / 2.0f, jsonCubes[c].dz / 2.0f));
	}
	if (jsonCubes) free(jsonCubes);
	return triList;
}

IndexedTriangleList MakeHeightField()
{
	UINT32 hfw = 0;
	UINT32 hfh = 0;
	float HeightScale = 4.0f;
	float HorizScale = 8.0f;

	Gdiplus::Bitmap img(L"heightfield.png");
	cout << "height field width x height = " << img.GetWidth() << " x " << img.GetHeight() << endl;
	hfw = img.GetWidth();
	hfh = img.GetHeight();
	Gdiplus::Rect r(0, 0, img.GetWidth(), img.GetHeight());
	Gdiplus::BitmapData bmpd;
	//cout << "pixfmt = " << img.GetPixelFormat() << endl;
	//if (img.GetPixelFormat() == PixelFormat32bppARGB) cout << "32 bit rgb" << endl;
	//pixfmt = 2498570
	//Gdiplus::PixelFormat
	img.LockBits(&r, Gdiplus::ImageLockModeRead, img.GetPixelFormat(), &bmpd);
	UINT32* ptr = (UINT32*)bmpd.Scan0;
	UINT32 max = 0;
	UINT32 min = 0xffffffff;
	UINT32 nvals = img.GetWidth() * img.GetHeight();
	for (UINT32 i = 0; i < nvals; i++) {
		if (ptr[i] > max) max = ptr[i];
		if (ptr[i] < min) min = ptr[i];
	}
	cout << "max " << max << endl;
	cout << "min " << min << endl;
	cout << "diff " << max - min << endl;

	physx::PxHeightFieldSample* samples = (physx::PxHeightFieldSample*)malloc(sizeof(physx::PxHeightFieldSample) * nvals);
	for (UINT y = 0; y < hfh; y++) {
		for (UINT x = 0; x < hfw; x++) {
			UINT i = (y * hfw) + x;
			samples[i].height = MulDiv(ptr[i] - min, 10, max - min);
			samples[i].materialIndex0 = 0;
			samples[i].materialIndex1 = 1;
			samples[i].clearTessFlag();
		}
	}

	img.UnlockBits(&bmpd);

	// ** make hight field
	physx::PxHeightFieldDesc hfDesc;
	hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns = hfw;
	hfDesc.nbRows = hfh;
	hfDesc.samples.data = samples;
	hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

	PhysxContext.aHeightField = PhysxContext.cooking->createHeightField(hfDesc,
		PhysxContext.mPhysics->getPhysicsInsertionCallback());
	if (!PhysxContext.aHeightField) cout << "ERROR: Failed to create height field" << endl;

	physx::PxHeightFieldGeometry hfGeom(PhysxContext.aHeightField, physx::PxMeshGeometryFlags(), HeightScale, HorizScale, HorizScale);

	// create the actor for heightfield
	PhysxContext.actor = PhysxContext.mPhysics->createRigidStatic(physx::PxTransform(physx::PxIdentity));
	if (!PhysxContext.actor) cout << "ERROR: Failed to create actor" << endl;

	PhysxContext.aHeightFieldShape = physx::PxRigidActorExt::createExclusiveShape(*PhysxContext.actor,
		hfGeom, *PhysxContext.gMaterial);
	if (!PhysxContext.aHeightFieldShape) cout << "ERROR: Failed to create height field shape" << endl;

	PhysxContext.aHeightFieldShape->setLocalPose(physx::PxTransform(physx::PxVec3(
		((float)hfw * HorizScale) / -2.0f, 
		0.0f, 
		((float)hfh * HorizScale) / -2.0f
	)));

	PhysxContext.gScene->addActor(*PhysxContext.actor);
	
	// make itl
	// ** make an ITL
	IndexedTriangleList itl;
	GLuint buffers[2];

	for (UINT y = 0; y < hfh; y++) {
		for (UINT x = 0; x < hfw; x++) {
			//int index = (y * hfw) + x;
			int index = (x * hfh) + y;
			short s00 = samples[index].height;
			itl.AddVertex((float)x * HorizScale, (float)s00 * HeightScale, (float)y * HorizScale, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	for (UINT y = 0; y < hfh-1; y++) {
		for (UINT x = 0; x < hfw-1; x++) {
			//int index0 = (y * hfw) + x;
			//int index1 = (y * hfw) + x + 1;
			//int index2 = ((y + 1) * hfw) + x + 1;
			//int index3 = ((y + 1) * hfw) + x;
			int index0 = (x * hfh) + y;
			int index1 = ((x+1) * hfh) + y;
			int index2 = ((x+1) * hfh) + (y+1);
			int index3 = (x * hfh) + (y+1);
			itl.AddTriIndices(index0, index2, index1);
			itl.AddTriIndices(index0, index3, index2);
		}
	}

	itl.CalculateVertexNormals();
	glGenBuffers(2, buffers);
	itl.SetGLBufferIds(buffers[0], buffers[1]);
	itl.BindArrays();
	glm::mat4 modelMatrix = glm::translate(glm::vec3(
		((float)hfw * HorizScale) / -2.0f,
		0.0f,
		((float)hfh * HorizScale) / -2.0f
	));
	itl.SetModelMatrix(modelMatrix);

	free(samples);

	return itl;
}

DWORD WINAPI RenderThread(void* parm)
{
	HWND hWnd = (HWND)parm;
	HDC hdc = nullptr;
	HGLRC hglrc = nullptr;
	RECT clientRect = { 0 };
	LARGE_INTEGER perfFreq = { 0 };
	LARGE_INTEGER perfCount = { 0 };
	LARGE_INTEGER perfCountEnd = { 0 };
	LONGLONG lastCount = 0;
	float FramesPerSecond = 0.0f;
	float ctsPerFrame = 0.0f; // at 60 fps
	float prc = 0.0f;
	GLCONTEXT lpContext = { 0 };
	BULLET_STRUCT bullets[10];
	unsigned int nextbullet = 0;
	unsigned int bulletWait = 0;
	char TextBuffer[80];
	ScreenBuffer sb;

	cout << "Begin render thread" << endl;

	memset(&bullets, 0, 10 * sizeof(BULLET_STRUCT));
	memset(&lpContext, 0, sizeof(GLCONTEXT));

	// get performance counter frequency info
	// computer counts per frame at 60 fps
	QueryPerformanceFrequency(&perfFreq);
	ctsPerFrame = (float)perfFreq.QuadPart / 60.0f;

	// get device context for window and init opengl
	hdc = GetDC(hWnd);
	hglrc = InitOpengl(hWnd, hdc);

	// init font
	if (FALSE == InitGlFont(hdc, &lpContext))
	{
		cout << "ERROR: Failed to init font" << endl;
	}
	else {
		cout << "Font successfully initialized" << endl;
	}

	// initialize physx
	if (false == InitializePhysx())
	{
		cout << "ERROR: Failed to init physx" << endl;
	}
	else {
		cout << "Physx successfully initialized" << endl;
	}

	// setup the open gl rendering context
	// clear color, other global settings
	SetupRenderingContext();

	// create triangle lists
	//lpContext.lpIdtFile = new IndexedTriangleList(LoadAndProcessGeometryFile());	// geometry json file
	lpContext.lpIdtFallingCube = new IndexedTriangleList(CreateFallingCube());		// a falling cube
	lpContext.lpIdtBulletBox = new IndexedTriangleList(IndexedTriangleList::CreateSphere(1.0f, 16, 16));
	IndexedTriangleList SkyBox = IndexedTriangleList::CreateSkyBox();
	IndexedTriangleList cyl = IndexedTriangleList::CreateCylinder(1.0f, 6.0f, 10, 16);
	IndexedTriangleList cone = IndexedTriangleList::CreateCone(10.0f, 20.0f, 16);
	IndexedTriangleList terrain = MakeHeightField();

	// load a texture
	//SetupTextures(&lpContext);
	TextureContext tileTexture(L"kitchtilec.png");
	TextureContext grassTexture(L"grass1.png");
	SetupCubeMap(&lpContext);

	// create the shaders
	lpContext.lpWorldShader = new WorldShaderProgramContext();
	lpContext.lpScreenShader = new ScreenShaderProgramContext();

	// configure the world shader (main shader)
	//   first - set it as currrent
	lpContext.lpWorldShader->SetAsCurrent();

	// this will contain a 2d texture
	// it will change below
	lpContext.lpWorldShader->SetTexture(0); // set uniform

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, lpContext.CubeMapTexId);
	lpContext.lpWorldShader->SetCubeMapTexture(1); // set uniform

	//   set the projection matrix
	GetClientRect(hWnd, &clientRect);
	glm::mat4 proj = glm::perspective(45.0f,
		(float)(clientRect.right - clientRect.left) / (float)(clientRect.bottom - clientRect.top),
		0.1f, 1000.0f);
	lpContext.lpWorldShader->SetProjMatrix(&proj[0][0]);

	// while we have a reference to the client size, set the viewport
	glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	glm::vec3 lIntensity(1.0f, 1.0f, 1.0f);
	glm::vec3 lDiffuse(0.3f, 0.3f, 0.3f);
	glm::vec3 lAmbient(0.7f, 0.7f, 0.7f);
	glm::vec3 lSpecular(1.0f, 1.0f, 1.0f);
	glm::vec3 lSpecularNone(0.0f, 0.0f, 0.0f);
	GLfloat lShininess = 16.0f;

	lpContext.lpWorldShader->SetLightIntensity(&lIntensity[0]);
	lpContext.lpWorldShader->SetLightDiffuse(&lDiffuse[0]);
	lpContext.lpWorldShader->SetLightAmbient(&lAmbient[0]);
	lpContext.lpWorldShader->SetLightSpecular(&lSpecular[0]);
	lpContext.lpWorldShader->SetLightShininess(lShininess);
	lpContext.lpWorldShader->SetDrawSkyBox(0);

	glm::vec3 lightPos(-100.0f, 400.0f, -100.0f);
	lpContext.lpWorldShader->SetLightPos(&lightPos[0]);
	lpContext.lpWorldShader->SetUseMatColor(0);

	sb.AddString("Ready...");

	while (TRUE) {

		// get the current perf count, calc FPS and save count
		QueryPerformanceCounter(&perfCount);
		FramesPerSecond = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		lastCount = perfCount.QuadPart;

		// check for thread stop event
		if (WAIT_OBJECT_0 != WaitForSingleObject(hStopEvent, 0))
		{
			lpContext.lpWorldShader->SetAsCurrent();

			// get user movement based on keys
			float mx = 0, my = 0, mz = 0;
			ApplyKeys(FramesPerSecond, &mx, &my, &mz);

			// move the user
			physx::PxControllerCollisionFlags collisionFlags =
				PhysxContext.pChar->move(physx::PxVec3(mx, my, mz), 0.0f, 1.0f / 60.0f, physx::PxControllerFilters());
			//if (collisionFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
			//{
				//sb.AddString("Collission!!");
				//cout << "collision!" << endl;
				//cout << "x:" << g_ex << " y:" << g_ey << " z:" << g_ez << endl;
				//cout << "hit the floor" << endl;
			//}

			// simulate physx
			physx::PxMat44 blockPose = PhysxSimulate(lpContext.lpIdtFallingCube->get_RigidDynamic(), bullets);

			// get the user location after simulation
			physx::PxExtendedVec3 ppos = PhysxContext.pChar->getPosition();
			g_ex = (float)ppos.x;
			g_ey = (float)ppos.y;
			g_ez = (float)ppos.z;

			if (bulletWait > 0) {
				bulletWait++;
				if (bulletWait == 30) bulletWait = 0;
			}
			if (g_KeysDown & KEY_MOUSE_LB && bulletWait == 0) {
				sb.AddString("Fire!!");
				physx::PxTransform player(physx::PxVec3(
					g_ex + sinf(DEG2RAD(g_az)),
					g_ey, 
					g_ez + -cosf(DEG2RAD(g_az))));
				physx::PxVec3 velVec(sinf(DEG2RAD(g_az)), -sinf(DEG2RAD(g_el)), -cosf(DEG2RAD(g_az)));
				velVec.normalize();
				if (bullets[nextbullet].lpBulletDyn)
				{
					bullets[nextbullet].lpBulletDyn->release();
				}
				bullets[nextbullet].lpBulletDyn = createDynamic(player,
					physx::PxSphereGeometry(1.0f),
					player.rotate(velVec) * 25);
				nextbullet++;
				if (nextbullet == 10) nextbullet = 0;
				bulletWait = 1;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// model matrix
			glm::mat4 ModelMatrix(1.0f);
			lpContext.lpWorldShader->SetModelMatrix(&ModelMatrix[0][0]);
			glm::mat3 normModelMatrix = glm::inverse(glm::transpose(glm::mat3(ModelMatrix)));
			lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);

			// view matrix
			glm::mat4 r1 = glm::rotate(DEG2RAD(g_az), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 r2 = glm::rotate(DEG2RAD(g_el), glm::vec3(cosf(DEG2RAD(g_az)), 0.0f, sinf(DEG2RAD(g_az))));
			glm::mat4 t = glm::translate(glm::vec3(-g_ex, -g_ey, -g_ez));
			glm::mat4 ViewMatrix = r1 * r2 * t;
			lpContext.lpWorldShader->SetViewMatrix(&ViewMatrix[0][0]);
			glm::mat3 normViewMatrix = glm::inverse(glm::transpose(glm::mat3(ViewMatrix)));
			lpContext.lpWorldShader->SetNormalViewMatrix(&normViewMatrix[0][0]);

			grassTexture.Bind(GL_TEXTURE0);

			// BEGIN draw the json cubes
			lpContext.lpWorldShader->SetModelMatrix(terrain.GetModelMatrixPointer());
			normModelMatrix = terrain.GetNormalModelMatrix();
			lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);
			lpContext.lpWorldShader->SetLightSpecular(&lSpecularNone[0]);
			terrain.BindBuffers();
			terrain.BindAttribs();
			terrain.DrawElements();
			terrain.UnbindAttribs();
			lpContext.lpWorldShader->SetLightSpecular(&lSpecular[0]);
			// END draw the json cubes

			// draw skybox
			ModelMatrix = glm::mat4(1.0);
			lpContext.lpWorldShader->SetModelMatrix(&ModelMatrix[0][0]);
			normModelMatrix = glm::inverse(glm::transpose(glm::mat3(ModelMatrix)));
			lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);
			lpContext.lpWorldShader->SetDrawSkyBox(1);
			SkyBox.BindBuffers();
			SkyBox.BindAttribs();
			SkyBox.DrawElements();
			SkyBox.UnbindAttribs();
			lpContext.lpWorldShader->SetDrawSkyBox(0);
			// end draw skybox

			// cylinder
			ModelMatrix = glm::translate(glm::vec3(10.0f, 3.0f, 10.0f));
			lpContext.lpWorldShader->SetModelMatrix(&ModelMatrix[0][0]);
			normModelMatrix = glm::inverse(glm::transpose(glm::mat3(ModelMatrix)));
			lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);
			lpContext.lpWorldShader->SetUseMatColor(1);
			glm::vec4 matColor(0.7f, 0.5f, 0.3f, 1.0f);
			lpContext.lpWorldShader->SetMatColor(&matColor[0]);
			cyl.BindBuffers();
			cyl.BindAttribs();
			cyl.DrawElements();
			cyl.UnbindAttribs();
			lpContext.lpWorldShader->SetUseMatColor(0);
			// end cylinder

			// cone
			ModelMatrix = glm::translate(glm::vec3(10.0f, 16.0f, 10.0f));
			lpContext.lpWorldShader->SetModelMatrix(&ModelMatrix[0][0]);
			normModelMatrix = glm::inverse(glm::transpose(glm::mat3(ModelMatrix)));
			lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);
			lpContext.lpWorldShader->SetUseMatColor(1);
			matColor = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
			lpContext.lpWorldShader->SetMatColor(&matColor[0]);
			cone.BindBuffers();
			cone.BindAttribs();
			cone.DrawElements();
			cone.UnbindAttribs();
			lpContext.lpWorldShader->SetUseMatColor(0);
			// end cylinder

			// model matrix (view matrix is the same)
			lpContext.lpWorldShader->SetModelMatrix(&blockPose[0][0]);
			glm::mat3 glmNormModelMatrix;
			glmNormModelMatrix[0][0] = blockPose[0][0]; glmNormModelMatrix[0][1] = blockPose[0][1]; glmNormModelMatrix[0][2] = blockPose[0][2];
			glmNormModelMatrix[1][0] = blockPose[1][0]; glmNormModelMatrix[1][1] = blockPose[1][1]; glmNormModelMatrix[1][2] = blockPose[1][2];
			glmNormModelMatrix[2][0] = blockPose[2][0]; glmNormModelMatrix[2][1] = blockPose[2][1]; glmNormModelMatrix[2][2] = blockPose[2][2];
			normModelMatrix = glm::inverse(glm::transpose(glmNormModelMatrix));
			lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);

			tileTexture.Bind(GL_TEXTURE0);
			//lpContext.lpWorldShader->SetTexture(0); // set uniform

			// BEGIN draw the falling block
			lpContext.lpIdtFallingCube->BindBuffers();
			lpContext.lpIdtFallingCube->BindAttribs();
			lpContext.lpIdtFallingCube->DrawElements();
			lpContext.lpIdtFallingCube->UnbindAttribs();
			// END draw the falling block

			// draw the bullets
			lpContext.lpIdtBulletBox->BindBuffers();
			lpContext.lpIdtBulletBox->BindAttribs();
			for (int b = 0; b < 10; b++) {
				if (bullets[b].lpBulletDyn) {
					// model matrix (view matrix is the same)
					lpContext.lpWorldShader->SetModelMatrix(&bullets[b].pose[0][0]);
					glmNormModelMatrix[0][0] = bullets[b].pose[0][0]; glmNormModelMatrix[0][1] = bullets[b].pose[0][1]; glmNormModelMatrix[0][2] = bullets[b].pose[0][2];
					glmNormModelMatrix[1][0] = bullets[b].pose[1][0]; glmNormModelMatrix[1][1] = bullets[b].pose[1][1]; glmNormModelMatrix[1][2] = bullets[b].pose[1][2];
					glmNormModelMatrix[2][0] = bullets[b].pose[2][0]; glmNormModelMatrix[2][1] = bullets[b].pose[2][1]; glmNormModelMatrix[2][2] = bullets[b].pose[2][2];
					normModelMatrix = glm::inverse(glm::transpose(glmNormModelMatrix));
					lpContext.lpWorldShader->SetNormalModelMatrix(&normModelMatrix[0][0]);
					lpContext.lpIdtBulletBox->DrawElements();
				}
			}
			lpContext.lpIdtBulletBox->UnbindAttribs();
			// end draw the bullets

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			// here we change the shader program
			lpContext.lpScreenShader->SetAsCurrent();
			GLfloat crosshair_vertices[8] = {
				-0.05f, 0.0f, 0.05f, 0.0f,
				0.0f, -0.1f, 0.0f, 0.1f
			};
			GLuint chvbo = 0;
			GLuint chvao = 0;
			glGenVertexArrays(1, &chvao);
			glBindVertexArray(chvao);
			glGenBuffers(1, &chvbo);
			glBindBuffer(GL_ARRAY_BUFFER, chvbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_vertices), crosshair_vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(lpContext.lpScreenShader->getPos(), 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(lpContext.lpScreenShader->getPos());
			glDrawArrays(GL_LINES, 0, 4);

			for (int i = 0; i < 20; i++) {
				const char* thisLine = sb.GetString(i);
				if (thisLine) {
					glRasterPos2f(-0.99f, 0.95f - ((float)i * 0.05f));
					glPushAttrib(GL_LIST_BIT);
					glListBase(lpContext.fontBase - 32);
					glCallLists((GLsizei)strlen(thisLine), GL_UNSIGNED_BYTE, thisLine);
					glPopAttrib();
				}
			}
			glRasterPos2f(-0.99f, -0.99f);
			glPushAttrib(GL_LIST_BIT);
			glListBase(lpContext.fontBase - 32);
			glCallLists((GLsizei)strlen(TextBuffer), GL_UNSIGNED_BYTE, TextBuffer);
			glPopAttrib();

			SwapBuffers(hdc);
		}
		else {
			// if thread stop, then, break the loop
			break;
		}

		// this code forces the loop to 60 fps using
		// the sleep function
		// numbers show we have lots of free time
		// for calcuations, at least 90% free time
		// between frames
		QueryPerformanceCounter(&perfCountEnd);
		LONGLONG elapsed = perfCountEnd.QuadPart - perfCount.QuadPart;
		float remainingCounts = ctsPerFrame - (float)elapsed;
		prc = remainingCounts * 100.0f / ctsPerFrame;
		memset(TextBuffer, 0, 80);
		sprintf_s(TextBuffer, 80, "%i_fps;_free_time_%i_percent", (int)FramesPerSecond, (int)prc);
		LONGLONG msToWait = (LONGLONG)remainingCounts * 1000 / perfFreq.QuadPart;
		//if (msToWait > 0) Sleep((DWORD)msToWait);
	}

	// clean up

	FreeGlFont(&lpContext);

	lpContext.lpWorldShader->FreeResources();
	delete lpContext.lpWorldShader;
	lpContext.lpScreenShader->FreeResources();
	delete lpContext.lpScreenShader;

	//glDeleteTextures(1, &lpContext.TexId);
	glDeleteTextures(1, &lpContext.CubeMapTexId);

	//lpContext.lpIdtFile->FreeResources();
	//delete lpContext.lpIdtFile;
	lpContext.lpIdtFallingCube->FreeResources();
	delete lpContext.lpIdtFallingCube;
	lpContext.lpIdtBulletBox->FreeResources();
	delete lpContext.lpIdtBulletBox;

	if (hglrc) {
		wglMakeCurrent(hdc, nullptr);
		wglDeleteContext(hglrc);
	}
	if (hdc) ReleaseDC(hWnd, hdc);

	DisposePhysx();

	return 0;
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

	//int pixfmt = ChoosePixelFormat(hdc, &pfd);

	int pixfmt = 0;
	HGLRC hrc = nullptr;
	if (!arbMultisampleSupported)
	{
		cout << "Using standard pixel format" << endl;
		pixfmt = ChoosePixelFormat(hdc, &pfd);     // Find A Compatible Pixel Format
		SetPixelFormat(hdc, pixfmt, &pfd);
		hrc = wglCreateContext(hdc);
	}
	else
	{
		cout << "Using multi-sample pixel format" << endl;
		pixfmt = arbMultisampleFormat;
		SetPixelFormat(hdc, pixfmt, &pfd);
		hrc = wglCreateContextAttribsARB(hdc, hrc, g_OpenGlAttributes);
	}

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

	if (wglSwapIntervalEXT(1)) {
		cout << "wglSwapIntervalEXT is success" << endl;
	}
	else {
		cout << "INFO: wglSwapIntervalEXT FAILED" << endl;
	}

	return hrc;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // allocate a console
	AllocConsole();
	FILE* f = nullptr;
	_wfreopen_s(&f, L"CONIN$", L"r", stdin);
	_wfreopen_s(&f, L"CONOUT$", L"w", stdout);
	_wfreopen_s(&f, L"CONOUT$", L"w", stderr);
	cout << "Ready..." << endl;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GLW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
	if(!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GLW));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(DEVMODE));
	dmScreenSettings.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
	ChangeDisplaySettings(&dmScreenSettings, CDS_RESET);

	ShowCursor(true);
	CloseHandle(hRenderThread);

	FreeConsole();

	GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

	// test window
	memset(&wcex, 0, sizeof(WNDCLASSEXW));

	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcTest;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLW));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr; // MAKEINTRESOURCEW(IDC_GLW);
	wcex.lpszClassName = L"GLWTestForMultiSampling";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// real window
	memset(&wcex, 0, sizeof(WNDCLASSEXW));

    wcex.cbSize = sizeof(WNDCLASSEXW);

    wcex.style          = CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr; // MAKEINTRESOURCEW(IDC_GLW);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hwndTest = CreateWindowEx(WS_EX_APPWINDOW, L"GLWTestForMultiSampling", L"GLWTestForMultiSampling",
		WS_OVERLAPPEDWINDOW^WS_THICKFRAME,
		0, 0, 640, 480,
		nullptr, nullptr, hInstance, nullptr);
	HDC hdcTest = GetDC(hwndTest);
	HGLRC hrcTest = InitOpengl(hwndTest, hdcTest);
	cout << "TEST hwnd = " << hex << hwndTest << dec << endl;
	cout << "TEST hdc = " << hex << hdcTest << dec << endl;
	cout << "TEST hrc = " << hex << hrcTest << dec << endl;
	if (!arbMultisampleSupported)
	{
		if (InitMultisample(hwndTest))
		{
			cout << "OpenGL supported multi-sample" << endl;
		}
		else {
			cout << "OpenGL does *not* support multi-sample" << endl;
		}
	}
	wglMakeCurrent(hdcTest, nullptr);
	wglDeleteContext(hrcTest);
	ReleaseDC(hwndTest, hdcTest);
	DestroyWindow(hwndTest);

	// look at display settings
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(DEVMODE));
	dmScreenSettings.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
	cout << dmScreenSettings.dmPelsWidth << endl;
	cout << dmScreenSettings.dmPelsHeight << endl;
	cout << dmScreenSettings.dmBitsPerPel << endl;

	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

	HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW, szWindowClass, szTitle, 
		WS_POPUP, //WS_OVERLAPPEDWINDOW^WS_THICKFRAME,
		0, 0, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight, 
		//WS_OVERLAPPEDWINDOW^WS_THICKFRAME,
		//0, 0, 640, 480,
		nullptr, nullptr, hInstance, nullptr);

   if (!hwnd)
   {
      return FALSE;
   }

   ShowCursor(false);

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   return TRUE;
}

void InitRawInput()
{
	RAWINPUTDEVICE rid[2];
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = 0; // RIDEV_NOLEGACY;
	rid[0].hwndTarget = 0;

	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;
	rid[1].dwFlags = 0; // RIDEV_NOLEGACY;
	rid[1].hwndTarget = 0;

	if (RegisterRawInputDevices(rid, 2, sizeof(rid[0])) == FALSE)
	{
		// reg failed
		cout << "ERROR: Registration of raw devices failed" << endl;
	}
}

bool MouseLook = true;

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
		if (true == MouseLook) {
			g_az += ((float)raw->data.mouse.lLastX * 0.1f);
			if (g_az < 0.0f) g_az += 360.0f;
			if (g_az > 360.0f) g_az -= 360.0f;
			g_el += ((float)raw->data.mouse.lLastY * 0.1f);
			if (g_el < -90.0f) g_el = -90.0f;
			if (g_el > 90.0f) g_el = 90.0f;
		}

		if (raw->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			g_KeysDown |= KEY_MOUSE_LB;
		}
		else {
			g_KeysDown &= ~KEY_MOUSE_LB;
		}
	}

	delete[] lpb;
}

LRESULT CALLBACK WndProcTest(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			ValidateRect(hWnd, nullptr);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UINT dwSize = 0;
	//UINT result = 0;
	//LPBYTE lpb = nullptr;
	//RECT rcClip;           // new area for ClipCursor
	//RECT rcOldClip;        // previous area for ClipCursor

	switch (message)
    {
	case WM_INPUT:
		HandleRawInput(lParam);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			//ShowCursor(TRUE);
			//ClipCursor(&rcOldClip);
			SetEvent(hStopEvent);
			WaitForSingleObject(hRenderThread, INFINITE);
			PostQuitMessage(0);
		}
		else if (wParam == 0x5a)
		{
			MouseLook = !MouseLook;
		}
		break;
	case WM_CREATE:
		//ShowCursor(FALSE);
		//GetClipCursor(&rcOldClip);
		//GetWindowRect(hWnd, &rcClip);
		//cout << "clip cursor " << ClipCursor(&rcClip) << endl;
		InitRawInput();
		hRenderThread = CreateThread(nullptr, 0, RenderThread, (LPVOID)hWnd, 0, nullptr);
		if (hRenderThread) {
			hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			ValidateRect(hWnd, nullptr);
        }
        break;
    case WM_DESTROY:
		//ClipCursor(&rcOldClip); 		
		SetEvent(hStopEvent);
		WaitForSingleObject(hRenderThread, INFINITE);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void DumpGlErrors(const char* FunctionName)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		cout << "GL ERROR: " << FunctionName << ": " << hex << err << dec << endl;
	}
}

void ClearGLError()
{
	while (glGetError()) {}
}