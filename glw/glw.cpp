// glw.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "glw.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Physx_64.lib")
#pragma comment(lib, "PhysxFoundation_64.lib")
#pragma comment(lib, "PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "PhysxCommon_64.lib")
#pragma comment(lib, "PhysxCooking_64.lib")
#pragma comment(lib, "PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")

#define MAX_LOADSTRING 100
#define F1 1.0f
#define F0 0.0f

using namespace std;

typedef struct {
	float ox;
	float oy;
	float oz;
	float dx;
	float dy;
	float dz;
	physx::PxShape* lpPxShape;
	union {
		physx::PxRigidDynamic* lpPxRigidDynamic;
		physx::PxRigidStatic* lpPxRigidStatic;
	};
} CUBE;

typedef struct {
	GLuint VertexArrayBuffer;
	GLuint IndexArrayBuffer;
	GLuint NumIndices;
	float* Vertices;
	unsigned int* Indices;
} INDEXED_LIST;

typedef struct {
	GLuint TexId;
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint ShaderProgram;
	INDEXED_LIST* lpIdxList;
	GLuint NumIdxList;
} GLCONTEXT;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HGLRC InitOpengl(HWND hwnd, HDC hdc);

HANDLE hRenderThread = nullptr;
HANDLE hStopEvent = nullptr;

float g_ex = 15.0f, g_ey = 20.0f, g_ez = 15.0f;
float g_az = -45.0f, g_el = 0.0f;

DWORD g_KeysDown = 0;
const DWORD KEY_W = 0x01;
const DWORD KEY_A = 0x02;
const DWORD KEY_S = 0x04;
const DWORD KEY_D = 0x08;
const DWORD KEY_Q = 0x10;
const DWORD KEY_E = 0x20;

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
PFNGLTEXSTORAGE2DPROC glTexStorage2D;

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

void DumpGlErrors(const char* FunctionName)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		cout << "GL ERROR: " << FunctionName << ": " << hex << err << dec << endl;
	}
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
}

void SetupRenderingContext()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

void ReleaseDynamicActorCube(CUBE* cube)
{
	if (cube->lpPxShape) cube->lpPxShape->release();
	if (cube->lpPxRigidDynamic) cube->lpPxRigidDynamic->release();
}

void ReleaseStaticActorCube(CUBE* cube)
{
	if (cube->lpPxShape) cube->lpPxShape->release();
	if (cube->lpPxRigidStatic) cube->lpPxRigidStatic->release();
}

void AddDynamicActor(CUBE* cube, physx::PxVec3 startingPos)
{
	// add an actor for the cube

	//physx::PxVec3 position(0, 40, 0);
	//float radius = 1.0f;
	//float halfHeight = 2.0f;
	cube->lpPxRigidDynamic = mPhysics->createRigidDynamic(physx::PxTransform(startingPos));
	// transform that rotates 90 deg about the z axis
	//physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	//aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*aCapsuleActor,
		//physx::PxCapsuleGeometry(radius, halfHeight), *gMaterial);
	cube->lpPxShape = physx::PxRigidActorExt::createExclusiveShape(
		*cube->lpPxRigidDynamic,
		physx::PxBoxGeometry(3, 3, 3), *gMaterial);
	//aCapsuleShape->setLocalPose(relativePose);
	physx::PxRigidBodyExt::updateMassAndInertia(*cube->lpPxRigidDynamic, 10.0f);
	gScene->addActor(*cube->lpPxRigidDynamic);
}

void AddStaticActor(CUBE* cube)
{
	physx::PxVec3 pos(
		cube->ox + (cube->dx / 2.0f),
		cube->oy + (cube->dy / 2.0f),
		cube->oz + (cube->dz / 2.0f));
	cube->lpPxRigidStatic = mPhysics->createRigidStatic(physx::PxTransform(pos));
	cube->lpPxShape = physx::PxRigidActorExt::createExclusiveShape(
		*cube->lpPxRigidStatic,
		physx::PxBoxGeometry(cube->dx / 2.0f, cube->dy / 2.0f, cube->dz / 2.0f),
		*gMaterial);
	gScene->addActor(*cube->lpPxRigidStatic);
}

void CreateCubes(unsigned int numCubes, CUBE* cubeList, INDEXED_LIST* lpList)
{

	GLuint buffers[2];

	// for testing this is only the x/y face
	// 4 * 8 = one face
	// 8 * 8 = two faces
	// 12 * 8 = three faces
	// 16 * 8 = four faces
	int CubeDataSize = 24 * 8; // 24 * 8;
	int VertDataSize = CubeDataSize * numCubes;
	lpList->Vertices = (float*)malloc(VertDataSize * sizeof(float));
	memset(lpList->Vertices, 0, VertDataSize * sizeof(float));

	// for testing this is only the x/y face
	// 6 = one face
	// 12 = two faces
	// 18 = three faces
	// 24 = four faces
	int CubeIndexSize = 36; // 36;
	int IndexDataSize = CubeIndexSize * numCubes;
	lpList->Indices = (unsigned int*)malloc(IndexDataSize * sizeof(unsigned int));
	memset(lpList->Indices, 0, IndexDataSize * sizeof(unsigned int));

	for (unsigned int c = 0; c < numCubes; c++) {
		unsigned int* iptr = (unsigned int*)(lpList->Indices + (c * CubeIndexSize));
		float* ptr = (float*)(lpList->Vertices + (c * CubeDataSize));
		CUBE* cb = cubeList + c;

		// face 1 vertices x/y neg z
		// pos x,y,z		                                                    tex u,v		                     norm i,j,k
		ptr[0] = cb->ox;           ptr[1] = cb->oy;           ptr[2] = cb->oz;  ptr[3] = 0.0f;  ptr[4] = cb->dy;   ptr[5] = 0.0f;  ptr[6] = 0.0f;  ptr[7] = 1.0f;
		ptr[8] = cb->ox;           ptr[9] = cb->oy + cb->dy;  ptr[10] = cb->oz; ptr[11] = 0.0f; ptr[12] = 0.0f;  ptr[13] = 0.0f; ptr[14] = 0.0f; ptr[15] = 1.0f;
		ptr[16] = cb->ox + cb->dx; ptr[17] = cb->oy + cb->dy; ptr[18] = cb->oz; ptr[19] = cb->dx; ptr[20] = 0.0f;  ptr[21] = 0.0f; ptr[22] = 0.0f; ptr[23] = 1.0f;
		ptr[24] = cb->ox + cb->dx; ptr[25] = cb->oy;          ptr[26] = cb->oz; ptr[27] = cb->dx; ptr[28] = cb->dy;  ptr[29] = 0.0f; ptr[30] = 0.0f; ptr[31] = 1.0f;

		// face 1 indexes
		iptr[0] = (c * 24) + 0; iptr[1] = (c * 24) + 2; iptr[2] = (c * 24) + 1;
		iptr[3] = (c * 24) + 0; iptr[4] = (c * 24) + 3; iptr[5] = (c * 24) + 2;

		// face 2 vertices x/y pos z
		// pos x,y,z
		ptr[32] = cb->ox;          ptr[33] = cb->oy;          ptr[34] = cb->oz + cb->dz; ptr[35] = 0.0f; ptr[36] = cb->dy; ptr[37] = 0.0f; ptr[38] = 0.0f; ptr[39] = -1.0f;
		ptr[40] = cb->ox;          ptr[41] = cb->oy + cb->dy; ptr[42] = cb->oz + cb->dz; ptr[43] = 0.0f; ptr[44] = 0.0f; ptr[45] = 0.0f; ptr[46] = 0.0f; ptr[47] = -1.0f;
		ptr[48] = cb->ox + cb->dx; ptr[49] = cb->oy + cb->dy; ptr[50] = cb->oz + cb->dz; ptr[51] = cb->dx; ptr[52] = 0.0f; ptr[53] = 0.0f; ptr[54] = 0.0f; ptr[55] = -1.0f;
		ptr[56] = cb->ox + cb->dx; ptr[57] = cb->oy;          ptr[58] = cb->oz + cb->dz; ptr[59] = cb->dx; ptr[60] = cb->dy; ptr[61] = 0.0f; ptr[62] = 0.0f; ptr[63] = -1.0f;

		iptr[6] = (c * 24) + 4; iptr[7] = (c * 24) + 5; iptr[8] = (c * 24) + 6;
		iptr[9] = (c * 24) + 4; iptr[10] = (c * 24) + 6; iptr[11] = (c * 24) + 7;

		// face 3 vertices x/y pos z
		ptr[64] = cb->ox;          ptr[65] = cb->oy; ptr[66] = cb->oz;          ptr[67] = 0.0f; ptr[68] = cb->dz; ptr[69] = 0.0f; ptr[70] = 1.0f; ptr[71] = 0.0f;
		ptr[72] = cb->ox;          ptr[73] = cb->oy; ptr[74] = cb->oz + cb->dz; ptr[75] = 0.0f; ptr[76] = 0.0f; ptr[77] = 0.0f; ptr[78] = 1.0f; ptr[79] = 0.0f;
		ptr[80] = cb->ox + cb->dx; ptr[81] = cb->oy; ptr[82] = cb->oz + cb->dz; ptr[83] = cb->dx; ptr[84] = 0.0f; ptr[85] = 0.0f; ptr[86] = 1.0f; ptr[87] = 0.0f;
		ptr[88] = cb->ox + cb->dx; ptr[89] = cb->oy; ptr[90] = cb->oz;          ptr[91] = cb->dx; ptr[92] = cb->dz; ptr[93] = 0.0f; ptr[94] = 1.0f; ptr[95] = 0.0f;

		// face 3 indexes
		iptr[12] = (c * 24) + 8; iptr[13] = (c * 24) + 9; iptr[14] = (c * 24) + 10;
		iptr[15] = (c * 24) + 8; iptr[16] = (c * 24) + 10; iptr[17] = (c * 24) + 11;

		// face 4 vertices x/y pos z
		ptr[96] = cb->ox;           ptr[97] = cb->oy + cb->dy;  ptr[98] = cb->oz;           ptr[99] = 0.0f;  ptr[100] = cb->dz; ptr[101] = 0.0f; ptr[102] = -1.0f; ptr[103] = 0.0f;
		ptr[104] = cb->ox;          ptr[105] = cb->oy + cb->dy; ptr[106] = cb->oz + cb->dz; ptr[107] = 0.0f; ptr[108] = 0.0f; ptr[109] = 0.0f; ptr[110] = -1.0f; ptr[111] = 0.0f;
		ptr[112] = cb->ox + cb->dx; ptr[113] = cb->oy + cb->dy; ptr[114] = cb->oz + cb->dz; ptr[115] = cb->dx; ptr[116] = 0.0f; ptr[117] = 0.0f; ptr[118] = -1.0f; ptr[119] = 0.0f;
		ptr[120] = cb->ox + cb->dx; ptr[121] = cb->oy + cb->dy; ptr[122] = cb->oz;          ptr[123] = cb->dx; ptr[124] = cb->dz; ptr[125] = 0.0f; ptr[126] = -1.0f; ptr[127] = 0.0f;

		// face 4 indexes
		iptr[18] = (c * 24) + 12; iptr[19] = (c * 24) + 14; iptr[20] = (c * 24) + 13;
		iptr[21] = (c * 24) + 12; iptr[22] = (c * 24) + 15; iptr[23] = (c * 24) + 14;

		// face 4 vertices x/y pos z
		ptr[128] = cb->ox; ptr[129] = cb->oy;          ptr[130] = cb->oz;          ptr[131] = 0.0f; ptr[132] = cb->dz; ptr[133] = -1.0f; ptr[134] = 0.0f; ptr[135] = 0.0f;
		ptr[136] = cb->ox; ptr[137] = cb->oy;          ptr[138] = cb->oz + cb->dz; ptr[139] = 0.0f; ptr[140] = 0.0f; ptr[141] = -1.0f; ptr[142] = 0.0f; ptr[143] = 0.0f;
		ptr[144] = cb->ox; ptr[145] = cb->oy + cb->dy; ptr[146] = cb->oz + cb->dz; ptr[147] = cb->dy; ptr[148] = 0.0f; ptr[149] = -1.0f; ptr[150] = 0.0f; ptr[151] = 0.0f;
		ptr[152] = cb->ox; ptr[153] = cb->oy + cb->dy; ptr[154] = cb->oz;          ptr[155] = cb->dy; ptr[156] = cb->dz; ptr[157] = -1.0f; ptr[158] = 0.0f; ptr[159] = 0.0f;

		// face 4 indexes
		iptr[24] = (c * 24) + 16; iptr[25] = (c * 24) + 18; iptr[26] = (c * 24) + 17;
		iptr[27] = (c * 24) + 16; iptr[28] = (c * 24) + 19; iptr[29] = (c * 24) + 18;

		// face 4 vertices x/y pos z
		ptr[160] = cb->ox + cb->dx; ptr[161] = cb->oy;          ptr[162] = cb->oz;          ptr[163] = 0.0f; ptr[164] = cb->dz; ptr[165] = 1.0f; ptr[166] = 0.0f; ptr[167] = 0.0f;
		ptr[168] = cb->ox + cb->dx; ptr[169] = cb->oy;          ptr[170] = cb->oz + cb->dz; ptr[171] = 0.0f; ptr[172] = 0.0f; ptr[173] = 1.0f; ptr[174] = 0.0f; ptr[175] = 0.0f;
		ptr[176] = cb->ox + cb->dx; ptr[177] = cb->oy + cb->dy; ptr[178] = cb->oz + cb->dz; ptr[179] = cb->dy; ptr[180] = 0.0f; ptr[181] = 1.0f; ptr[182] = 0.0f; ptr[183] = 0.0f;
		ptr[184] = cb->ox + cb->dx; ptr[185] = cb->oy + cb->dy; ptr[186] = cb->oz;          ptr[187] = cb->dy; ptr[188] = cb->dz; ptr[189] = 1.0f; ptr[190] = 0.0f; ptr[191] = 0.0f;

		// face 4 indexes
		iptr[30] = (c * 24) + 20; iptr[31] = (c * 24) + 21; iptr[32] = (c * 24) + 22;
		iptr[33] = (c * 24) + 20; iptr[34] = (c * 24) + 22; iptr[35] = (c * 24) + 23;
	}

	glGenBuffers(2, buffers);
	lpList->VertexArrayBuffer = buffers[0];
	lpList->IndexArrayBuffer = buffers[1];
	lpList->NumIndices = IndexDataSize;

	glBindBuffer(GL_ARRAY_BUFFER, lpList->VertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, VertDataSize * sizeof(float), lpList->Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lpList->IndexArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexDataSize * sizeof(unsigned int), lpList->Indices, GL_STATIC_DRAW);
}

//CUBE* BuildFloorCubesAndIndexedList(INDEXED_LIST* lpList, unsigned int* nCubes)
//{
//	CUBE* lpCubes = (CUBE*)malloc(20 * 20 * sizeof(CUBE));
//	memset(lpCubes, 0, 20 * 20 * sizeof(CUBE));
//	unsigned int cubeCtr = 0;
//	for (float fx = -50.0f; fx < 50.0f; fx += 5.0f)
//	{
//		for (float fy = -50.0f; fy < 50.0f; fy += 5.0f)
//		{
//			lpCubes[cubeCtr].ox = fx;
//			lpCubes[cubeCtr].oy = -1.0f;
//			lpCubes[cubeCtr].oz = fy;
//			lpCubes[cubeCtr].dx = 5.0f;
//			lpCubes[cubeCtr].dy = 1.0f;
//			lpCubes[cubeCtr].dz = 5.0f;
//			cubeCtr++;
//		}
//	}
//	CreateCubes(20 * 20, lpCubes, lpList);
//	*nCubes = 20 * 20;
//	return lpCubes;
//}

/*
void SetupGeometryX()
{
	GLuint buffers[2];

	g_ctx.idxList[0].Vertices = (float*)malloc(100 * 100 * 4 * 8 * sizeof(float));
	memset(g_ctx.idxList[0].Vertices, 0, 100 * 100 * 4 * 8 * sizeof(float));
	g_ctx.idxList[0].Indices = (unsigned int*)malloc(100 * 100 * 6 * sizeof(unsigned int));
	memset(g_ctx.idxList[0].Indices, 0, 100 * 100 * 6 * sizeof(unsigned int));
	long fdx = 0;
	long vidx = 0;
	long iidx = 0;
	for (int fx = 0; fx < 100; fx++)
	{
		for (int fy = 0; fy < 100; fy++)
		{			
			g_ctx.idxList[0].Vertices[fdx++] = (float)fx - 50.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = (float)fy - 50.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = -1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;

			g_ctx.idxList[0].Vertices[fdx++] = (float)fx - 50.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = (float)fy - 49.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = -1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;

			g_ctx.idxList[0].Vertices[fdx++] = (float)fx - 49.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = (float)fy - 49.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = -1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;

			g_ctx.idxList[0].Vertices[fdx++] = (float)fx - 49.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = (float)fy - 50.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;
			g_ctx.idxList[0].Vertices[fdx++] = -1.0f;
			g_ctx.idxList[0].Vertices[fdx++] = 0.0f;

			g_ctx.idxList[0].Indices[iidx++] = vidx;
			g_ctx.idxList[0].Indices[iidx++] = vidx + 1;
			g_ctx.idxList[0].Indices[iidx++] = vidx + 2;
			g_ctx.idxList[0].Indices[iidx++] = vidx;
			g_ctx.idxList[0].Indices[iidx++] = vidx + 2;
			g_ctx.idxList[0].Indices[iidx++] = vidx + 3;

			vidx += 4;
		}
	}

	glGenBuffers(2, buffers);
	g_ctx.idxList[0].VertexArrayBuffer = buffers[0];
	g_ctx.idxList[0].IndexArrayBuffer = buffers[1];
	g_ctx.idxList[0].NumIndices = 100 * 100 * 6;

	glBindBuffer(GL_ARRAY_BUFFER, g_ctx.idxList[0].VertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, 100 * 100 * 4 * 8 * sizeof(float), g_ctx.idxList[0].Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ctx.idxList[0].IndexArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 100 * 100 * 6 * sizeof(unsigned int), g_ctx.idxList[0].Indices, GL_STATIC_DRAW);
}
*/

BOOL SetupShaders(GLCONTEXT* lpGlContext)
{
	const char* VertexShaderSource =
		"#version 400\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec2 vTexCoord;"
		"layout(location = 2) in vec3 vNormal;"
		//"layout(location = 3) in vec4 vColor;"
		//"uniform mat4 gWorld;"
		"uniform mat4 gModelMatrix;"
		"uniform mat4 gViewMatrix;"
		"uniform mat4 gProjMatrix;"
		"uniform vec3 gLookDirVec;"
		"out vec2 vUV;"
		"out float vCLR;"
		//"out vec4 vVertexColor;"
		"void main() {"
		"  vec4 modelPos = gModelMatrix * vec4(vPosition, 1.0);"
		"  vec4 viewPos = gViewMatrix * modelPos;"
		"  gl_Position = gProjMatrix * viewPos;"
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

	lpGlContext->ShaderProgram = glCreateProgram();

	lpGlContext->VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(lpGlContext->VertexShader, 1, &VertexShaderSource, NULL);
	glCompileShader(lpGlContext->VertexShader);
	glGetShaderiv(lpGlContext->VertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to compile vertex shader";
		return FALSE;
	}

	lpGlContext->FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(lpGlContext->FragmentShader, 1, &FragmentShaderSource, NULL);
	glCompileShader(lpGlContext->FragmentShader);
	glGetShaderiv(lpGlContext->FragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to compile fragment shader";
		return FALSE;
	}

	glAttachShader(lpGlContext->ShaderProgram, lpGlContext->VertexShader);
	glAttachShader(lpGlContext->ShaderProgram, lpGlContext->FragmentShader);
	glLinkProgram(lpGlContext->ShaderProgram);

	glGetProgramiv(lpGlContext->ShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to link shader program" << endl;
		return FALSE;
	}

	glValidateProgram(lpGlContext->ShaderProgram);
	glGetProgramiv(lpGlContext->ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Failed to validate shader program" << endl;
		return FALSE;
	}

	glUseProgram(lpGlContext->ShaderProgram);

	DumpGlErrors("SetupShaders");

	return TRUE;
}

void SetupTextures(GLCONTEXT* lpGlContext)
{

	UINT iw, ih;
	const GLsizei numMipMaps = 3;

	Gdiplus::Bitmap *img = Gdiplus::Bitmap::FromFile(L"croxx.png", FALSE);
	iw = img->GetWidth();
	ih = img->GetHeight();
	Gdiplus::Rect r(0, 0, iw, ih);
	Gdiplus::BitmapData bmpd;
	img->LockBits(&r, Gdiplus::ImageLockModeRead, img->GetPixelFormat(), &bmpd);

	glGenTextures(1, &lpGlContext->TexId);
	glBindTexture(GL_TEXTURE_2D, lpGlContext->TexId);
	glTexStorage2D(GL_TEXTURE_2D, numMipMaps, GL_RGBA8, iw, ih);
	// 32 bit png
	//glTexImage2D(GL_TEXTURE_2D, 0, 4, iw, ih, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmpd.Scan0);
	// 24 bit png
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, bmpd.Scan0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iw, ih, GL_RGB, GL_UNSIGNED_BYTE, bmpd.Scan0);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	img->UnlockBits(&bmpd);

	DumpGlErrors("SetupTextures");

	delete img;
}

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
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAallocator, errcbk);
	if (!mFoundation) {
		cout << "PxCreateFoundation failed!" << endl;
	}

	bool recordMemoryAllocations = true;

	mPvd = physx::PxCreatePvd(*mFoundation);
	if (!mPvd) {
		cout << "failed to create pvd" << endl;
	}
	transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	if (!transport) {
		cout << "failed to create transport" << endl;
	}
	mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation,
		physx::PxTolerancesScale(), recordMemoryAllocations, mPvd);
	if (!mPhysics) {
		cout << "PxCreatePhysics failed!" << endl;
	}

	if (!PxInitExtensions(*mPhysics, mPvd))
	{
		cout << "PxInitExtensions failed!" << endl;
	}

	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	if (!gDispatcher) {
		cout << "failed to create dispatcher" << endl;
	}
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	gScene = mPhysics->createScene(sceneDesc);
	if (!gScene) {
		cout << "failed to create scene" << endl;
	}

	gMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// ground plane
	//groundPlane = physx::PxCreatePlane(*mPhysics, physx::PxPlane(0, 1, 0, 0), *gMaterial);
	//gScene->addActor(*groundPlane);

	//physx::PxShape* shape = mPhysics->createShape(physx::PxSphereGeometry(1), *gMaterial);
	//physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(physx::PxTransform(physx::PxVec3(0, 40, 0)));
	//body->attachShape(*shape);
	//physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	//gScene->addActor(*body);

	/*
	physx::PxVec3 position(0, 40, 0);
	//float radius = 1.0f;
	//float halfHeight = 2.0f;
	aCapsuleActor = mPhysics->createRigidDynamic(physx::PxTransform(position));
	// transform that rotates 90 deg about the z axis
	//physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	//aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*aCapsuleActor,
		//physx::PxCapsuleGeometry(radius, halfHeight), *gMaterial);
	aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*aCapsuleActor,
		physx::PxBoxGeometry(3, 3, 3), *gMaterial);
	//aCapsuleShape->setLocalPose(relativePose);
	physx::PxRigidBodyExt::updateMassAndInertia(*aCapsuleActor, 10.0f);
	gScene->addActor(*aCapsuleActor);
	aCapsuleShape->release();
	*/

	cmanager = PxCreateControllerManager(*gScene);
	physx::PxCapsuleControllerDesc desc;
	desc.height = 4.0f;
	desc.material = gMaterial;
	desc.position = physx::PxExtendedVec3(g_ex, g_ey + 5.0f, g_ez);
	desc.radius = 1.0f;
	pChar = cmanager->createController(desc);

	return true;
}

physx::PxMat44 PhysxSimulate(CUBE* pCube)
{
	physx::PxShape* shapes[1];
	gScene->simulate(1.0f / 60.0f); // 60th of a sec (60 fps)
	gScene->fetchResults(true);
	physx::PxU32 n = pCube->lpPxRigidDynamic->getNbShapes();
	pCube->lpPxRigidDynamic->getShapes(shapes, n);
	physx::PxGeometryHolder gh = shapes[0]->getGeometry();
	const physx::PxMat44 shapePose(physx::PxShapeExt::getGlobalPose(*shapes[0], *pCube->lpPxRigidDynamic));
	return shapePose;
}

void DisposePhysx()
{
	if (cooking) cooking->release();
	if (pChar) pChar->release();
	if (cmanager) cmanager->release();
	if (gMaterial) gMaterial->release();
	if (groundPlane) groundPlane->release();
	if (gScene) gScene->release();
	if (gDispatcher) gDispatcher->release();
	if (mPhysics) mPhysics->release();
	if (transport) transport->release();
	if (mPvd) mPvd->release();
	if (mFoundation) mFoundation->release();
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

DWORD WINAPI RenderThread(void* parm)
{
	HWND hWnd = (HWND)parm;
	HDC hdc = nullptr;
	HGLRC hglrc = nullptr;
	RECT clientRect = { 0 };
	//GLuint gWorldMatrixLoc = 0;
	GLuint gModelMatrixLoc = 0;
	GLuint gViewMatrixLoc = 0;
	GLuint gProjMatrixLoc = 0;
	GLuint gLookDirVecLoc = 0;
	GLuint gTextureLoc = 0;
	LARGE_INTEGER perfFreq = { 0 };
	LARGE_INTEGER perfCount = { 0 };
	LARGE_INTEGER perfCountEnd = { 0 };
	LONGLONG lastCount = 0;
	float FramesPerSecond = 0.0f;
	float ctsPerFrame = 0.0f; // at 60 fps
	CUBE fallingCube = { -3.0f, -3.0f, -3.0f, 6.0f, 6.0f, 6.0f, nullptr, nullptr };
	//CUBE* lpFloorCubes = nullptr;
	//unsigned int nFloorCubes = 0;
	float prc = 0.0f;
	CUBE* jsonCubes = nullptr;
	unsigned int jsonCubeNum = 0;
	GLCONTEXT lpContext;

	QueryPerformanceFrequency(&perfFreq);
	ctsPerFrame = (float)perfFreq.QuadPart / 60.0f;

	hdc = GetDC(hWnd);
	hglrc = InitOpengl(hWnd, hdc);

	// initialize physx
	if (false == InitializePhysx())
	{
		cout << "ERROR: Failed to init physx" << endl;
	}
	else {
		cout << "Physx successfully initialized" << endl;
	}

	cout << "Begin render thread" << endl;
	SetupRenderingContext();

	memset(&lpContext, 0, sizeof(GLCONTEXT));
	lpContext.lpIdxList = (INDEXED_LIST*)malloc(2 * sizeof(INDEXED_LIST));
	memset(lpContext.lpIdxList, 0, 2 * sizeof(INDEXED_LIST));
	lpContext.NumIdxList = 2;

	cout << "loading geometry file" << endl;
	jsonCubes = LoadGeometryJson("mz.json", &jsonCubeNum);
	CreateCubes(jsonCubeNum, jsonCubes, &lpContext.lpIdxList[0]);
	for (unsigned int c = 0; c < jsonCubeNum; c++) {
		AddStaticActor(&jsonCubes[c]);
	}
	cout << "geometry file stuff done" << endl;

	CreateCubes(1, &fallingCube, &lpContext.lpIdxList[1]);
	AddDynamicActor(&fallingCube, physx::PxVec3(0, 40, 0));
	SetupShaders(&lpContext);
	SetupTextures(&lpContext);

	//gWorldMatrixLoc = glGetUniformLocation(g_ctx.ShaderProgram, "gWorld");
	gModelMatrixLoc = glGetUniformLocation(lpContext.ShaderProgram, "gModelMatrix");
	gViewMatrixLoc = glGetUniformLocation(lpContext.ShaderProgram, "gViewMatrix");
	gProjMatrixLoc = glGetUniformLocation(lpContext.ShaderProgram, "gProjMatrix");

	GetClientRect(hWnd, &clientRect);
	glm::mat4 proj = glm::perspective(45.0f,
		(float)(clientRect.right - clientRect.left) / (float)(clientRect.bottom - clientRect.top),
		0.1f, 1000.0f);
	glUniformMatrix4fv(gProjMatrixLoc, 1, GL_FALSE, &proj[0][0]);

	glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	gLookDirVecLoc = glGetUniformLocation(lpContext.ShaderProgram, "gLookDirVec");
	gTextureLoc = glGetUniformLocation(lpContext.ShaderProgram, "texSampler");
	glUniform1i(gTextureLoc, 0);

	while (TRUE) {

		QueryPerformanceCounter(&perfCount);
		cout << "\r" << (int)FramesPerSecond << " fps; free time " << (int)prc << "%                    " << flush;
		FramesPerSecond = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		lastCount = perfCount.QuadPart;

		if (WAIT_OBJECT_0 != WaitForSingleObject(hStopEvent, 0))
		{

			// get user movement based on keys
			float mx = 0, my = 0, mz = 0;
			ApplyKeys(FramesPerSecond, &mx, &my, &mz);

			// move the user
			physx::PxControllerCollisionFlags collisionFlags =
				pChar->move(physx::PxVec3(mx, my, mz), 0.0f, 1.0f / 60.0f, physx::PxControllerFilters());
			//if (collisionFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
			//{
				//cout << "collision!" << endl;
				//cout << "x:" << g_ex << " y:" << g_ey << " z:" << g_ez << endl;
				//cout << "hit the floor" << endl;
			//}

			// simulate physx
			physx::PxMat44 blockPose = PhysxSimulate(&fallingCube);

			// get the user location after simulation
			physx::PxExtendedVec3 ppos = pChar->getPosition();
			g_ex = (float)ppos.x;
			g_ey = (float)ppos.y;
			g_ez = (float)ppos.z;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// for lighting
			glm::vec3 lookDirVec(sinf(DEG2RAD(g_az)), sinf(DEG2RAD(g_el)), cosf(DEG2RAD(g_az)));
			glUniform3fv(gLookDirVecLoc, 1, &lookDirVec[0]);

			// projection matrix
			glm::mat4 r1 = glm::rotate(DEG2RAD(g_az), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 r2 = glm::rotate(DEG2RAD(g_el), glm::vec3(cosf(DEG2RAD(g_az)), 0.0f, sinf(DEG2RAD(g_az))));
			glm::mat4 t = glm::translate(glm::vec3(-g_ex, -g_ey, -g_ez));
			glm::mat4 view = r1 * r2 * t;
			//glm::mat4 final = proj * modelmat;
			glUniformMatrix4fv(gViewMatrixLoc, 1, GL_FALSE, &view[0][0]);

			// model matrix is identity at this point
			glm::mat4 ModelMatrix(1.0f);
			glUniformMatrix4fv(gModelMatrixLoc, 1, GL_FALSE, &ModelMatrix[0][0]);

			// set the active texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, lpContext.TexId);

			// BEGIN draw the floor
			//glBindBuffer(GL_ARRAY_BUFFER, g_ctx.idxList[0].VertexArrayBuffer);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ctx.idxList[0].IndexArrayBuffer);

			//glEnableVertexAttribArray(0);
			//glEnableVertexAttribArray(1);
			//glEnableVertexAttribArray(2);

			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
			//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

			//glDrawElements(GL_TRIANGLES, g_ctx.idxList[0].NumIndices, GL_UNSIGNED_INT, 0);

			//glDisableVertexAttribArray(0);
			//glDisableVertexAttribArray(1);
			//glDisableVertexAttribArray(2);
			// END draw the floor

			// BEGIN draw the json cubes
			glBindBuffer(GL_ARRAY_BUFFER, lpContext.lpIdxList[0].VertexArrayBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lpContext.lpIdxList[0].IndexArrayBuffer);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

			glDrawElements(GL_TRIANGLES, lpContext.lpIdxList[0].NumIndices, GL_UNSIGNED_INT, 0);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			// END draw the json cubes

			// BEGIN draw the falling block
			glUniformMatrix4fv(gModelMatrixLoc, 1, GL_FALSE, &blockPose[0][0]);

			glBindBuffer(GL_ARRAY_BUFFER, lpContext.lpIdxList[1].VertexArrayBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lpContext.lpIdxList[1].IndexArrayBuffer);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

			glDrawElements(GL_TRIANGLES, lpContext.lpIdxList[1].NumIndices, GL_UNSIGNED_INT, 0);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			// END draw the falling block

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			SwapBuffers(hdc);
		}
		else {
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
		//cout << FramesPerSecond << " fps; pct free time " << (int)prc << "                    " << flush;
		LONGLONG msToWait = (LONGLONG)remainingCounts * 1000 / perfFreq.QuadPart;
		if (msToWait > 0) Sleep((DWORD)msToWait);
	}

	glDeleteShader(lpContext.VertexShader);
	glDeleteShader(lpContext.FragmentShader);
	glDeleteProgram(lpContext.ShaderProgram);
	glDeleteTextures(1, &lpContext.TexId);
	for (unsigned int i = 0; i < lpContext.NumIdxList; i++) {
		glDeleteBuffers(1, &lpContext.lpIdxList[i].VertexArrayBuffer);
		glDeleteBuffers(1, &lpContext.lpIdxList[i].IndexArrayBuffer);
		free(lpContext.lpIdxList[i].Vertices);
		free(lpContext.lpIdxList[i].Indices);
	}

	if (hglrc) {
		wglMakeCurrent(hdc, nullptr);
		wglDeleteContext(hglrc);
	}
	if (hdc) ReleaseDC(hWnd, hdc);

	if (jsonCubes)
	{
		for (unsigned int c = 0; c < jsonCubeNum; c++) {
			ReleaseStaticActorCube(&jsonCubes[c]);
		}
		free(jsonCubes);
	}

	//for (unsigned int c = 0; c < nFloorCubes; c++) {
		//ReleaseStaticActorCube(&lpFloorCubes[c]);
	//}

	ReleaseDynamicActorCube(&fallingCube);

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

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GLW);
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

   HWND hwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW^WS_THICKFRAME,
      CW_USEDEFAULT, 720, CW_USEDEFAULT, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hwnd)
   {
      return FALSE;
   }

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
	UINT dwSize = 0;
	UINT result = 0;
	LPBYTE lpb = nullptr;
    switch (message)
    {
	case WM_INPUT:
		HandleRawInput(lParam);
		break;
	case WM_CREATE:
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
