// sh3dedsgn.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "sh3dedsgn.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib,"Gdiplus.lib")
#pragma comment(lib, "Rpcrt4.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
void                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hwndParent = nullptr;
HWND hwndChild3d = nullptr;
HWND hwndOrthoXY = nullptr;
HWND hwndOrthoXZ = nullptr;
HWND hwndOrthoYZ = nullptr;

CRITICAL_SECTION g_csCubeList;
CUBE* cubeList = nullptr;
DWORD numCubes = 0;
bool CubeListChanged = false;

const WPARAM HOTKEY_CTRLP = 1;

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

    // TODO: Place code here.
	// allocate a console
	AllocConsole();
	FILE* f = nullptr;
	_wfreopen_s(&f, L"CONIN$", L"r", stdin);
	_wfreopen_s(&f, L"CONOUT$", L"w", stdout);
	_wfreopen_s(&f, L"CONOUT$", L"w", stderr);
	cout << "Ready..." << endl;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SH3DEDSGN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SH3DEDSGN));

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

	FreeConsole();

	GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
void MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SH3DEDSGN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground = nullptr;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SH3DEDSGN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WndProc3DWindow;
	wcex.lpszClassName = L"SH3DEDSGN_3DWND";
	wcex.lpszMenuName = nullptr;
	wcex.cbWndExtra = sizeof(int);
	RegisterClassExW(&wcex);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcOrthoWindow;
	wcex.lpszClassName = L"SH3DEDSGN_ORTHOWND";
	wcex.cbWndExtra = sizeof(int);
	RegisterClassExW(&wcex);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   hwndParent = hWnd;

   hwndChild3d = CreateWindow(L"SH3DEDSGN_3DWND", L"3D View", WS_CHILD,
	   0, 0, 320, 240, hWnd, nullptr, hInstance, nullptr);

   hwndOrthoXY = CreateWindow(L"SH3DEDSGN_ORTHOWND", L"3D View", WS_CHILD,
	   321, 0, 320, 240, hWnd, nullptr, hInstance, (LPVOID)0);

   hwndOrthoXZ = CreateWindow(L"SH3DEDSGN_ORTHOWND", L"3D View", WS_CHILD,
	   0, 241, 320, 240, hWnd, nullptr, hInstance, (LPVOID)1);

   hwndOrthoYZ = CreateWindow(L"SH3DEDSGN_ORTHOWND", L"3D View", WS_CHILD,
	   321, 241, 320, 240, hWnd, nullptr, hInstance, (LPVOID)2);

   ShowWindow(hWnd, nCmdShow);
   ShowWindow(hwndChild3d, nCmdShow);
   ShowWindow(hwndOrthoXY, nCmdShow);
   ShowWindow(hwndOrthoXZ, nCmdShow);
   ShowWindow(hwndOrthoYZ, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void HandleKeyDown(HWND hWnd, WPARAM wParam)
{
	switch (wParam) {
		case VK_RETURN:
			if (CommitCube()) {
				InvalidateRect(hWnd, nullptr, true);
			}
			break;
		case 0x5a: // z
			// toggle the 3d walker state in 3d view
			SendMessage(hwndChild3d, DSGN_TOGGLE_WALK, 0, 0);
			break;
		default:
			break;
	}
}

BOOL CALLBACK CubePropDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CUBE* selCube = nullptr;
	BOOL bTranslated = false;
	wchar_t buffer[64];
	unsigned int nchars = 0;

	switch (message)
	{
	case WM_INITDIALOG:
		selCube = getSelectedCube();
		if (selCube) {
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_OX, (int)selCube->ox, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_OY, (int)selCube->oy, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_OZ, (int)selCube->oz, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_DX, (int)selCube->dx, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_DY, (int)selCube->dy, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_DZ, (int)selCube->dz, TRUE);
		}
		else {
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_OX, 0, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_OY, 0, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_OZ, 0, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_DX, 0, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_DY, 0, TRUE);
			SetDlgItemInt(hwndDlg, IDC_CUBE_PROP_DZ, 0, TRUE);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDB_CLOSE:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDB_APPLY:
			selCube = getSelectedCube();
			if (selCube) {
				if (GetDlgItemText(hwndDlg, IDC_CUBE_PROP_OX, buffer, 64)) selCube->ox = (float)_wtof(buffer);
				if (GetDlgItemText(hwndDlg, IDC_CUBE_PROP_OY, buffer, 64)) selCube->oy = (float)_wtof(buffer);
				if (GetDlgItemText(hwndDlg, IDC_CUBE_PROP_OZ, buffer, 64)) selCube->oz = (float)_wtof(buffer);
				if (GetDlgItemText(hwndDlg, IDC_CUBE_PROP_DX, buffer, 64)) selCube->dx = (float)_wtof(buffer);
				if (GetDlgItemText(hwndDlg, IDC_CUBE_PROP_DY, buffer, 64)) selCube->dy = (float)_wtof(buffer);
				if (GetDlgItemText(hwndDlg, IDC_CUBE_PROP_DZ, buffer, 64)) selCube->dz = (float)_wtof(buffer);
				InvalidateRect(GetParent(hwndDlg), nullptr, true);
			}
			return TRUE;
		}
	}
	return FALSE;
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
	int width = 0;
	int height = 0;
	HWND CubePropDlg = 0;

    switch (message)
    {
	case WM_CREATE:
		InitializeCriticalSection(&g_csCubeList);
		RegisterHotKey(hWnd, HOTKEY_CTRLP, MOD_CONTROL | MOD_NOREPEAT, 0x50);
			break;
	case WM_HOTKEY:
		if (wParam == HOTKEY_CTRLP) {
			CubePropDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CUBE_PROP), hWnd, (DLGPROC)CubePropDlgProc);
			ShowWindow(CubePropDlg, SW_SHOW);
		}
		break;
	case WM_KEYDOWN:
		HandleKeyDown(hWnd, wParam);
		break;
	case WM_SIZE:
		width = LOWORD(lParam) / 2;
		height = HIWORD(lParam) / 2;
		SetWindowPos(hwndChild3d, nullptr, 0, 0, width, height, 0);
		SetWindowPos(hwndOrthoXY, nullptr, width + 1, 0, width - 1, height, 0);
		SetWindowPos(hwndOrthoXZ, nullptr, 0, height + 1, width, height - 1, 0);
		SetWindowPos(hwndOrthoYZ, nullptr, width + 1, height + 1, width - 1, height - 1, 0);
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
			case ID_EDIT_PROPERTIES:
				CubePropDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CUBE_PROP), hWnd, (DLGPROC)CubePropDlgProc);
				ShowWindow(CubePropDlg, SW_SHOW);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		DeleteCriticalSection(&g_csCubeList);
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

void ErrorExit(const LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	//ExitProcess(dw);
}
