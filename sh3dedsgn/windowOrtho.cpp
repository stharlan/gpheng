
#include "stdafx.h"

WindowOrthoContext wctx[3];
RECT tempBox = { 0 };
CUBE tempCube = { 0 };
CUBE movingCube = { 0 };
UUID SelectedCube = { 0 };

extern CRITICAL_SECTION g_csCubeList;
extern CUBE* cubeList;
extern DWORD numCubes;
extern bool CubeListChanged;

int lastx = 0, lasty = 0;
int origx = 0, origy = 0;

int MouseMoveMode = 0;
const int MMM_NONE = 0;
const int MMM_DRAW_BOX = 1;
const int MMM_MOVE_BOX = 2;
const int MMM_SIZE_BOX = 3;
const int MMM_PAN = 4;
const int MMM_MOVE_HANDLE = 5;
int GrabbedHandle = 0;

CUBE* getSelectedCube()
{
	CUBE* retval = nullptr;
	RPC_STATUS rpcStatus = 0;
	if (0 == UuidCompare(&SelectedCube, &tempCube.id, &rpcStatus))
	{
		return &tempCube;
	}
	return retval;
}

bool CommitCube()
{
	// check to make sure cube is valid
	// all dimensions must have a positive measure
	if (tempCube.dx > 0.0f && tempCube.dy > 0.0f && tempCube.dz > 0.0f)
	{

		EnterCriticalSection(&g_csCubeList);

		numCubes++;
		CUBE* tempCubeList = (CUBE*)malloc(numCubes * sizeof(CUBE));
		memset(tempCubeList, 0, numCubes * sizeof(CUBE));
		if (cubeList == nullptr) {
			cubeList = tempCubeList;
		}
		else {
			memcpy(tempCubeList, cubeList, (numCubes - 1) * sizeof(CUBE));
			free(cubeList);
			cubeList = tempCubeList;
		}

		// create a permanent cube from the temp cube
		cubeList[numCubes - 1].ox = tempCube.ox;
		cubeList[numCubes - 1].oy = tempCube.oy;
		cubeList[numCubes - 1].oz = tempCube.oz;
		cubeList[numCubes - 1].dx = tempCube.dx;
		cubeList[numCubes - 1].dy = tempCube.dy;
		cubeList[numCubes - 1].dz = tempCube.dz;
		UuidCreate(&cubeList[numCubes - 1].id);

		// reset the temp cube
		tempCube.dx = tempCube.dy = tempCube.dz = 0.0f;

		// reset the temp box after cube is committed
		// this means temp box is empty
		tempBox.left = tempBox.right = tempBox.top = tempBox.bottom = 0;

		// unselect cube
		memset(&SelectedCube, 0, sizeof(UUID));

		CubeListChanged = true;

		LeaveCriticalSection(&g_csCubeList);

		return true;
	}
	return false;
}

void DrawACubeA(HDC hdc, CUBE* pCube, int orthoType, glm::mat4& worldToScreen, float scale, bool IsSelected)
{
	wchar_t buffer[32];
	memset(buffer, 0, 32);
	RECT r = { 0 };
	PLOGFONT plf = (PLOGFONT)LocalAlloc(LPTR, sizeof(LOGFONT));
	HFONT hfont;
	HGDIOBJ holdfont;
	//int sul[2];
	//int slr[2];

	StringCchCopy(plf->lfFaceName, 6, TEXT("Arial"));
	plf->lfWeight = FW_NORMAL;
	plf->lfHeight = 14;

	int oldBkMode = SetBkMode(hdc, TRANSPARENT);
	COLORREF oldTextcolor = SetTextColor(hdc, 0x00ffffff);

	float sdx = pCube->dx / scale;
	float sdy = pCube->dy / scale;
	float sdz = pCube->dz / scale;

	switch (orthoType) {
	case 0: // xy
		glm::vec4 worigin0(pCube->ox, pCube->oy, 0.0f, 1.0f);
		glm::vec4 sorigin0 = worldToScreen * worigin0;
		Rectangle(hdc, (int)sorigin0[0], (int)sorigin0[1], (int)(sorigin0[0] + sdx), (int)(sorigin0[1] + sdy));

		if (IsSelected) {

			wctx[orthoType].sel.left = (int)sorigin0[0];
			wctx[orthoType].sel.right = (int)(sorigin0[0] + sdx);
			wctx[orthoType].sel.top = (int)sorigin0[1];
			wctx[orthoType].sel.bottom = (int)(sorigin0[1] + sdy);

			wsprintf(buffer, L"%i", (int)pCube->dx);
			r.left = (int)sorigin0[0];
			r.right = (int)(sorigin0[0] + sdx);
			r.top = (int)(sorigin0[1] + sdy + 1);
			r.bottom = (int)(sorigin0[1] + sdy + 2);
			plf->lfEscapement = 0;
			hfont = CreateFontIndirect(plf);
			holdfont = SelectObject(hdc, hfont);
			DrawText(hdc, buffer, (int)wcslen(buffer), &r, DT_CENTER | DT_NOCLIP);
			SelectObject(hdc, holdfont);
			DeleteObject(hfont);

			wsprintf(buffer, L"%i", (int)pCube->dy);
			r.left = (int)(sorigin0[0] - 15);
			r.right = (int)(sorigin0[0] - 16);
			r.top = (int)sorigin0[1];
			r.bottom = (int)(sorigin0[1] + sdy);
			plf->lfEscapement = 900;
			hfont = CreateFontIndirect(plf);
			holdfont = SelectObject(hdc, hfont);
			DrawText(hdc, buffer, (int)wcslen(buffer), &r, DT_VCENTER | DT_NOCLIP | DT_SINGLELINE);
			SelectObject(hdc, holdfont);
			DeleteObject(hfont);

			// draw handles around the box
			//sul[0] = (int)sorigin0[0];
			//sul[1] = (int)sorigin0[1];
			//slr[0] = (int)(sul[0] + sdx);
			//slr[1] = (int)(sul[1] + sdy);
			//Rectangle(hdc, sul[0] - 8, sul[1] - 8, sul[0] - 2, sul[1] - 2);
			//Rectangle(hdc, sul[0] - 8, slr[1] + 2, sul[0] - 2, slr[1] + 8);
			//Rectangle(hdc, slr[0] + 2, sul[1] - 8, slr[0] + 8, sul[1] - 2);
			//Rectangle(hdc, slr[0] + 2, slr[1] + 2, slr[0] + 8, slr[1] + 8);
			//_FILLRECT(wctx[orthoType].ulhndl, sul[0] - 8, sul[1] - 8, sul[0] - 2, sul[1] - 2);
			//_FILLRECT(wctx[orthoType].llhndl, sul[0] - 8, slr[1] + 2, sul[0] - 2, slr[1] + 8);
			//_FILLRECT(wctx[orthoType].urhndl, slr[0] + 2, sul[1] - 8, slr[0] + 8, sul[1] - 2);
			//_FILLRECT(wctx[orthoType].lrhndl, slr[0] + 2, slr[1] + 2, slr[0] + 8, slr[1] + 8);
		}

		break;
	case 1: // xz
		glm::vec4 worigin1(pCube->ox, pCube->oz, 0.0f, 1.0f);
		glm::vec4 sorigin1 = worldToScreen * worigin1;
		Rectangle(hdc, (int)sorigin1[0], (int)sorigin1[1], (int)(sorigin1[0] + sdx), (int)(sorigin1[1] + sdz));

		if (IsSelected) {

			wctx[orthoType].sel.left = (int)sorigin1[0];
			wctx[orthoType].sel.right = (int)(sorigin1[0] + sdx);
			wctx[orthoType].sel.top = (int)sorigin1[1];
			wctx[orthoType].sel.bottom = (int)(sorigin1[1] + sdz);

			wsprintf(buffer, L"%i", (int)pCube->dx);
			r.left = (int)sorigin1[0];
			r.right = (int)(sorigin1[0] + sdx);
			r.top = (int)(sorigin1[1] + sdz + 1);
			r.bottom = (int)(sorigin1[1] + sdz + 2);
			plf->lfEscapement = 0;
			hfont = CreateFontIndirect(plf);
			holdfont = SelectObject(hdc, hfont);
			DrawText(hdc, buffer, (int)wcslen(buffer), &r, DT_CENTER | DT_NOCLIP);
			SelectObject(hdc, holdfont);
			DeleteObject(hfont);

			wsprintf(buffer, L"%i", (int)pCube->dz);
			r.left = (int)(sorigin1[0] - 15);
			r.right = (int)(sorigin1[0] - 16);
			r.top = (int)sorigin1[1];
			r.bottom = (int)(sorigin1[1] + sdz);
			plf->lfEscapement = 900;
			hfont = CreateFontIndirect(plf);
			holdfont = SelectObject(hdc, hfont);
			DrawText(hdc, buffer, (int)wcslen(buffer), &r, DT_VCENTER | DT_NOCLIP | DT_SINGLELINE);
			SelectObject(hdc, holdfont);
			DeleteObject(hfont);

			// draw handles around the box
			//sul[0] = (int)sorigin1[0];
			//sul[1] = (int)sorigin1[1];
			//slr[0] = (int)(sul[0] + sdx);
			//slr[1] = (int)(sul[1] + sdz);
			//Rectangle(hdc, sul[0] - 8, sul[1] - 8, sul[0] - 2, sul[1] - 2);
			//Rectangle(hdc, sul[0] - 8, slr[1] + 2, sul[0] - 2, slr[1] + 8);
			//Rectangle(hdc, slr[0] + 2, sul[1] - 8, slr[0] + 8, sul[1] - 2);
			//Rectangle(hdc, slr[0] + 2, slr[1] + 2, slr[0] + 8, slr[1] + 8);
			//_FILLRECT(wctx[orthoType].ulhndl, sul[0] - 8, sul[1] - 8, sul[0] - 2, sul[1] - 2);
			//_FILLRECT(wctx[orthoType].llhndl, sul[0] - 8, slr[1] + 2, sul[0] - 2, slr[1] + 8);
			//_FILLRECT(wctx[orthoType].urhndl, slr[0] + 2, sul[1] - 8, slr[0] + 8, sul[1] - 2);
			//_FILLRECT(wctx[orthoType].lrhndl, slr[0] + 2, slr[1] + 2, slr[0] + 8, slr[1] + 8);

		}

		break;
	case 2: // yz
		glm::vec4 worigin2(pCube->oz, pCube->oy, 0.0f, 1.0f);
		glm::vec4 sorigin2 = worldToScreen * worigin2;
		Rectangle(hdc, (int)sorigin2[0], (int)sorigin2[1], (int)(sorigin2[0] + sdz), (int)(sorigin2[1] + sdy));

		if (IsSelected) {

			wctx[orthoType].sel.left = (int)sorigin2[0];
			wctx[orthoType].sel.right = (int)(sorigin2[0] + sdz);
			wctx[orthoType].sel.top = (int)sorigin2[1];
			wctx[orthoType].sel.bottom = (int)(sorigin2[1] + sdy);

			wsprintf(buffer, L"%i", (int)pCube->dz);
			r.left = (int)sorigin2[0];
			r.right = (int)(sorigin2[0] + sdz);
			r.top = (int)(sorigin2[1] + sdy + 1);
			r.bottom = (int)(sorigin2[1] + sdy + 2);
			plf->lfEscapement = 0;
			hfont = CreateFontIndirect(plf);
			holdfont = SelectObject(hdc, hfont);
			DrawText(hdc, buffer, (int)wcslen(buffer), &r, DT_CENTER | DT_NOCLIP);
			SelectObject(hdc, holdfont);
			DeleteObject(hfont);

			wsprintf(buffer, L"%i", (int)pCube->dy);
			r.left = (int)(sorigin2[0] - 15);
			r.right = (int)(sorigin2[0] - 16);
			r.top = (int)sorigin2[1];
			r.bottom = (int)(sorigin2[1] + sdy);
			plf->lfEscapement = 900;
			hfont = CreateFontIndirect(plf);
			holdfont = SelectObject(hdc, hfont);
			DrawText(hdc, buffer, (int)wcslen(buffer), &r, DT_VCENTER | DT_NOCLIP | DT_SINGLELINE);
			SelectObject(hdc, holdfont);
			DeleteObject(hfont);

			// draw handles around the box
			//sul[0] = (int)sorigin2[0];
			//sul[1] = (int)sorigin2[1];
			//slr[0] = (int)(sul[0] + sdz);
			//slr[1] = (int)(sul[1] + sdy);
			//Rectangle(hdc, sul[0] - 8, sul[1] - 8, sul[0] - 2, sul[1] - 2);
			//Rectangle(hdc, sul[0] - 8, slr[1] + 2, sul[0] - 2, slr[1] + 8);
			//Rectangle(hdc, slr[0] + 2, sul[1] - 8, slr[0] + 8, sul[1] - 2);
			//Rectangle(hdc, slr[0] + 2, slr[1] + 2, slr[0] + 8, slr[1] + 8);
			//_FILLRECT(wctx[orthoType].ulhndl, sul[0] - 8, sul[1] - 8, sul[0] - 2, sul[1] - 2);
			//_FILLRECT(wctx[orthoType].llhndl, sul[0] - 8, slr[1] + 2, sul[0] - 2, slr[1] + 8);
			//_FILLRECT(wctx[orthoType].urhndl, slr[0] + 2, sul[1] - 8, slr[0] + 8, sul[1] - 2);
			//_FILLRECT(wctx[orthoType].lrhndl, slr[0] + 2, slr[1] + 2, slr[0] + 8, slr[1] + 8);

		}

		break;
	}

	SetBkMode(hdc, oldBkMode);
	SetTextColor(hdc, oldTextcolor);
}

void DrawOrtho(HWND hWnd, PAINTSTRUCT* ps, HDC hdc)
{
	RPC_STATUS rpcStatus = 0;

	int orthoType = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	FillRect(hdc, &ps->rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

	int ww = ps->rcPaint.right - ps->rcPaint.left;
	int wh = ps->rcPaint.bottom - ps->rcPaint.top;

	float scaledWidth = (float)ww * wctx[orthoType].scale;
	float scaledHeight = (float)wh * wctx[orthoType].scale;

	glm::mat4 worldToScreen = glm::scale(glm::vec3(1.0f / wctx[orthoType].scale, 1.0f / wctx[orthoType].scale, 1.0f));
	worldToScreen *= glm::translate(
		glm::vec3(
			(scaledWidth / 2.0f) - wctx[orthoType].tx,
			(scaledHeight / 2.0f) - wctx[orthoType].ty,
			0.0f));	


	HPEN grayPen = CreatePen(PS_SOLID, 1, 0x333333);
	HPEN grayPen2 = CreatePen(PS_SOLID, 1, 0x666666);
	HPEN lbluePen = CreatePen(PS_SOLID, 1, 0x663333);
	HGDIOBJ oldpen = SelectObject(hdc, grayPen);

	float xstart = (float)((int)((wctx[orthoType].tx - (scaledWidth / 2.0f)) / wctx[orthoType].grid) * wctx[orthoType].grid);
	for (float fx = xstart; fx < wctx[orthoType].tx + (scaledWidth / 2.0f); fx += wctx[orthoType].grid)
	{
		glm::vec4 p1(fx, wctx[orthoType].ty - (scaledHeight / 2.0f), 0.0f, 1.0f);
		glm::vec4 p2(fx, wctx[orthoType].ty + (scaledHeight / 2.0f), 0.0f, 1.0f);
		glm::vec4 tp1 = worldToScreen * p1;
		glm::vec4 tp2 = worldToScreen * p2;
		if (fx == 0.0f) {
			SelectObject(hdc, lbluePen);
		}
		else {
			SelectObject(hdc, grayPen);
		}
		MoveToEx(hdc, (int)tp1[0], (int)tp1[1], nullptr);
		LineTo(hdc, (int)tp2[0], (int)tp2[1]);
	}

	float ystart = (float)((int)((wctx[orthoType].ty - (scaledHeight / 2.0f)) / wctx[orthoType].grid) * wctx[orthoType].grid);
	for (float fy = ystart; fy < wctx[orthoType].ty + (scaledHeight / 2.0f); fy += wctx[orthoType].grid)
	{
		glm::vec4 p1(wctx[orthoType].tx - (scaledWidth / 2.0f), fy, 0.0f, 1.0f);
		glm::vec4 p2(wctx[orthoType].tx + (scaledWidth / 2.0f), fy, 0.0f, 1.0f);
		glm::vec4 tp1 = worldToScreen * p1;
		glm::vec4 tp2 = worldToScreen * p2;
		if (fy == 0.0f) {
			SelectObject(hdc, lbluePen);
		}
		else {
			SelectObject(hdc, grayPen);
		}
		MoveToEx(hdc, (int)tp1[0], (int)tp1[1], nullptr);
		LineTo(hdc, (int)tp2[0], (int)tp2[1]);
	}

	HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

	SelectObject(hdc, grayPen2);
	for (unsigned int c = 0; c < numCubes; c++) {
		DrawACubeA(hdc, &cubeList[c], orthoType, worldToScreen, wctx[orthoType].scale, false);
	}

	// draw the selected box
	SelectObject(hdc, GetStockObject(WHITE_PEN));

	// if the temp cube is selected - draw it
	if(0 == UuidCompare(&SelectedCube, &tempCube.id, &rpcStatus))
	{
		// the temp cube is selected, draw it as selected
		DrawACubeA(hdc, &tempCube, orthoType, worldToScreen, wctx[orthoType].scale, true);
	}
	else {
		// nothing to draw, so, empty the selected rect structure
		memset(&wctx[orthoType].sel, 0, sizeof(RECT));
		//memset(&wctx[orthoType].ulhndl, 0, sizeof(RECT));
		//memset(&wctx[orthoType].llhndl, 0, sizeof(RECT));
		//memset(&wctx[orthoType].urhndl, 0, sizeof(RECT));
		//memset(&wctx[orthoType].lrhndl, 0, sizeof(RECT));
	}

	SelectObject(hdc, oldBrush);
	SelectObject(hdc, oldpen);
	DeleteObject(grayPen);
	DeleteObject(grayPen2);
	DeleteObject(lbluePen);

	switch (orthoType) {
	case 0:;
		DrawText(hdc, L"ORTHO X/Y", 9, &ps->rcPaint, 0);
		break;
	case 1:
		DrawText(hdc, L"ORTHO X/Z", 9, &ps->rcPaint, 0);
		break;
	case 2:
		DrawText(hdc, L"ORTHO Y/Z", 9, &ps->rcPaint, 0);
		break;
	}

}

bool MouseInSelBox(int orthoType, int mx, int my)
{
	bool isIn = false;
	// the mouse is just moving
	if (wctx[orthoType].sel.right > wctx[orthoType].sel.left &&
		wctx[orthoType].sel.bottom > wctx[orthoType].sel.top)
	{
		// check if mouse is in rect
		POINT p = { mx, my };
		if (PtInRect(&wctx[orthoType].sel, p)) {
			isIn = true;
		}
	}
	return isIn;
}

//int MouseInHandle(int orthoType, int mx, int my)
//{
//	POINT p = { mx, my };
//	if (PtInRect(&wctx[orthoType].ulhndl, p)) {
//		return ULHANDLE;
//	}
//	if (PtInRect(&wctx[orthoType].urhndl, p)) {
//		return URHANDLE;
//	}
//	if (PtInRect(&wctx[orthoType].llhndl, p)) {
//		return LLHANDLE;
//	}
//	if (PtInRect(&wctx[orthoType].lrhndl, p)) {
//		return LRHANDLE;
//	}
//	return 0;
//}

void HandleMouseMove(HWND hWnd, unsigned short mx, unsigned short my, WPARAM wParam, int orthoType)
{
	int thisx = 0, thisy = 0;
	RECT client = { 0 };
	RPC_STATUS rpcStatus = 0;

	// TODO
	// each time the screen is drawn, if there is a selected
	// rect, store the coords in a special rect 
	// structure
	// then, when the mouse is moved, check that structure
	// for changing the cursor

	//if (wParam & MK_CONTROL && wParam & MK_LBUTTON) {
	if (MouseMoveMode == MMM_PAN) 
	{
		// pan the screen
		thisx = mx;
		thisy = my;
		wctx[orthoType].tx += (float)(lastx - thisx) * wctx[orthoType].scale;
		wctx[orthoType].ty += (float)(lasty - thisy) * wctx[orthoType].scale;
		lastx = thisx;
		lasty = thisy;
		InvalidateRect(hWnd, nullptr, true);
	} 
	else if (MouseMoveMode == MMM_DRAW_BOX) 
	{
		// we are drawing a new box
		// set the temp box right and bottom to the current mouse location
		tempBox.right = mx;
		tempBox.bottom = my;

		// only do something if the box is bigger than zero in both dimensions
		if (tempBox.left != tempBox.right && tempBox.top != tempBox.bottom)
		{
			GetClientRect(hWnd, &client);
			float scaledWidth = (float)(client.right - client.left) * wctx[orthoType].scale;
			float scaledHeight = (float)(client.bottom - client.top) * wctx[orthoType].scale;
			glm::mat4 screenToWorld = glm::translate(
				glm::vec3(
					-1.0f * ((scaledWidth / 2.0f) - wctx[orthoType].tx),
					-1.0f * ((scaledHeight / 2.0f) - wctx[orthoType].ty),
					0.0f));
			screenToWorld *= glm::scale(glm::vec3(wctx[orthoType].scale, wctx[orthoType].scale, 1.0f));

			// orieng the box and place the corners into two vectors: sul and slr
			glm::vec4 sul(FMIN(tempBox.left, tempBox.right), FMIN(tempBox.top, tempBox.bottom), 0.0f, 1.0f);
			glm::vec4 slr(FMAX(tempBox.left, tempBox.right), FMAX(tempBox.top, tempBox.bottom), 0.0f, 1.0f);

			glm::vec4 wul = screenToWorld * sul;
			glm::vec4 wlr = screenToWorld * slr;

			// TODO update snapping
			// right now this snaps by dividing by the grid size, taking the int,
			// and multiplying back, thereby snapping to the lowest grid
			//
			// example:
			// grid = 8, value = 15
			// 15 / 8 = 1.87
			// int = 1
			// 1 * 8 = 8
			// therefore - snaps to 8
			//
			// what I want to do is snap to the nearest grid
			// so, 15 would snap to 16
			// 

			//wul[0] = (float)((int)(wul[0] / wctx[orthoType].grid)) * wctx[orthoType].grid;
			//wul[1] = (float)((int)(wul[1] / wctx[orthoType].grid)) * wctx[orthoType].grid;
			//wlr[0] = (float)((int)(wlr[0] / wctx[orthoType].grid)) * wctx[orthoType].grid;
			//wlr[1] = (float)((int)(wlr[1] / wctx[orthoType].grid)) * wctx[orthoType].grid;

			wul[0] = roundf(wul[0] / wctx[orthoType].grid) * wctx[orthoType].grid;
			wul[1] = roundf(wul[1] / wctx[orthoType].grid) * wctx[orthoType].grid;
			wlr[0] = roundf(wlr[0] / wctx[orthoType].grid) * wctx[orthoType].grid;
			wlr[1] = roundf(wlr[1] / wctx[orthoType].grid) * wctx[orthoType].grid;

			if (wlr[0] != wul[0] && wlr[1] != wul[1])
			{
				// box has size - set temp cube dimensions according to ortho type
				if (orthoType == 0) {
					// xy
					tempCube.ox = wul[0];
					tempCube.oy = wul[1];
					tempCube.oz = 0.0f;
					tempCube.dx = wlr[0] - wul[0];
					tempCube.dy = wlr[1] - wul[1];
					tempCube.dz = wctx[orthoType].grid;
				}
				else if (orthoType == 1) {
					// xz
					tempCube.ox = wul[0];
					tempCube.oy = 0.0f;
					tempCube.oz = wul[1];
					tempCube.dx = wlr[0] - wul[0];
					tempCube.dy = wctx[orthoType].grid;
					tempCube.dz = wlr[1] - wul[1];
				}
				else if (orthoType == 2) {
					// yz
					tempCube.ox = 0.0f;
					tempCube.oy = wul[1];
					tempCube.oz = wul[0];
					tempCube.dx = wctx[orthoType].grid;
					tempCube.dy = wlr[1] - wul[1];
					tempCube.dz = wlr[0] - wul[0];
				}

				// make it the selected cube
				SelectedCube = tempCube.id;
			}
			else
			{
				// box is not big enough - zero out the temp cube
				tempCube.dx = tempCube.dy = tempCube.dz = 0.0f;

				// unselect the temp cube
				memset(&SelectedCube, 0, sizeof(UUID));
			}
			InvalidateRect(GetParent(hWnd), nullptr, true);
		}
	}
	else if (MouseMoveMode == MMM_MOVE_HANDLE)
	{
		//// calculate the movement from the origin
		//float diffx = (mx - origx) * wctx[orthoType].scale;
		//float diffy = (my - origy) * wctx[orthoType].scale;

		//// snap to grid
		//float tx = 0.0f;
		//float ty = 0.0f;
		//if (fabs(diffx) >= wctx[orthoType].grid)
		//{
		//	tx = roundf(diffx / wctx[orthoType].grid) * wctx[orthoType].grid;
		//}
		//if (fabs(diffy) >= wctx[orthoType].grid)
		//{
		//	ty = roundf(diffy / wctx[orthoType].grid) * wctx[orthoType].grid;
		//}

		//// if grid movement
		//if (tx != 0.0f || ty != 0.0f)
		//{
		//	// if the selected cube is the temp cube
		//	if (0 == UuidCompare(&SelectedCube, &tempCube.id, &rpcStatus))
		//	{

		//		switch (orthoType) {
		//		case 0: // xy
		//			switch (GrabbedHandle) {
		//			case ULHANDLE:

		//				break;
		//			case URHANDLE:
		//				break;
		//			case LLHANDLE:
		//				break;
		//			case LRHANDLE:
		//				break;
		//			}
		//			break;
		//		}

		//		// move the cube using settings that were copied to movingCube as a base
		//		//switch (orthoType) {
		//		//case 0:
		//		//	tempCube.ox = movingCube.ox + tx;
		//		//	tempCube.oy = movingCube.oy + ty;
		//		//	break;
		//		//case 1:
		//		//	tempCube.ox = movingCube.ox + tx;
		//		//	tempCube.oz = movingCube.oz + ty;
		//		//	break;
		//		//case 2:
		//		//	tempCube.oz = movingCube.oz + tx;
		//		//	tempCube.oy = movingCube.oy + ty;
		//		//	break;
		//		//}
		//		InvalidateRect(GetParent(hWnd), nullptr, true);
		//	}
		//}

	}
	else if (MouseMoveMode == MMM_MOVE_BOX)
	{

		// calculate the movement from the origin
		float diffx = (mx - origx) * wctx[orthoType].scale;
		float diffy = (my - origy) * wctx[orthoType].scale;

		// snap to grid
		float tx = 0.0f;
		float ty = 0.0f;
		if (fabs(diffx) >= wctx[orthoType].grid)
		{
			tx = roundf(diffx / wctx[orthoType].grid) * wctx[orthoType].grid;
		}
		if (fabs(diffy) >= wctx[orthoType].grid)
		{
			ty = roundf(diffy / wctx[orthoType].grid) * wctx[orthoType].grid;
		}

		// if grid movement
		if (tx != 0.0f || ty != 0.0f)
		{
			// if the selected cube is the temp cube
			if (0 == UuidCompare(&SelectedCube, &tempCube.id, &rpcStatus))
			{
				// move the cube using settings that were copied to movingCube as a base
				switch (orthoType) {
				case 0:
					tempCube.ox = movingCube.ox + tx;
					tempCube.oy = movingCube.oy + ty;
					break;
				case 1:
					tempCube.ox = movingCube.ox + tx;
					tempCube.oz = movingCube.oz + ty;
					break;
				case 2:
					tempCube.oz = movingCube.oz + tx;
					tempCube.oy = movingCube.oy + ty;
					break;
				}
				InvalidateRect(GetParent(hWnd), nullptr, true);
			}
		}
	}

	if (MouseInSelBox(orthoType, mx, my))
	{
		SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
	}
	//else if (MouseInHandle(orthoType, mx, my) > 0) {
	//	SetCursor(LoadCursor(nullptr, IDC_CROSS));
	//}
	else {
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}
}

LRESULT CALLBACK WndProcOrthoWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT* lpCS = nullptr;
	int wheelMovement = 0;
	int orthoType = -1;
	HDC tdc = 0;
	RECT cr = { 0 };
	int hndl = 0;

	switch (message)
	{
	case WM_CREATE:
		// user data is the ortho type
		// 0 is XY
		// 1 is XZ
		// 2 is YZ
		lpCS = (CREATESTRUCT*)lParam;

		// I know this is just an unsigned int < 10
		// no need to worry about truncation
		#pragma warning(disable: 4302 4311)
		orthoType = reinterpret_cast<int>(lpCS->lpCreateParams);
		#pragma warning(default: 4302 4311)

		wctx[orthoType].grid = 32;
		wctx[orthoType].scale = 1.0f;
		wctx[orthoType].tx = 0.0f;
		wctx[orthoType].ty = 0.0f;
		memset(&wctx[orthoType].sel, 0, sizeof(RECT));
		//memset(&wctx[orthoType].ulhndl, 0, sizeof(RECT));
		//memset(&wctx[orthoType].llhndl, 0, sizeof(RECT));
		//memset(&wctx[orthoType].urhndl, 0, sizeof(RECT));
		//memset(&wctx[orthoType].lrhndl, 0, sizeof(RECT));
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)orthoType);

		// zero out the temp cube and give it a uuid
		tempCube.dx = tempCube.dy = tempCube.dz = 0.0f;
		UuidCreate(&tempCube.id);

		tdc = GetDC(hWnd);
		GetClientRect(hWnd, &cr);
		wctx[orthoType].bmpDbuf = CreateCompatibleBitmap(tdc, cr.right - cr.left, cr.bottom - cr.top);
		ReleaseDC(hWnd, tdc);

		break;
	case WM_LBUTTONDOWN:
		orthoType = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		lastx = origx = LOWORD(lParam);
		lasty = origy = HIWORD(lParam);
		if (wParam & MK_CONTROL) {
			MouseMoveMode = MMM_PAN;
		}
		else if (MouseInSelBox(orthoType, origx, origy))
		{
			// right now, the only cube that can be
			// selected is the one thats being drawn
			// or, the temp cube
			// save the tempCube settings in movingCube
			memcpy(&movingCube, &tempCube, sizeof(CUBE));
			MouseMoveMode = MMM_MOVE_BOX;
		}
		//else if ((hndl = MouseInHandle(orthoType, origx, origy)) > 0)
		//{
		//	memcpy(&movingCube, &tempCube, sizeof(CUBE));
		//	MouseMoveMode = MMM_MOVE_HANDLE;
		//	GrabbedHandle = hndl;
		//}
		else 
		{
			MouseMoveMode = MMM_DRAW_BOX;
			// starting to draw a box, place the current
			// mouse location in the temp box left and top
			tempBox.left = tempBox.right = lastx;
			tempBox.top = tempBox.bottom = lasty;
		}
		break;
	case WM_LBUTTONUP:
		memset(&movingCube, 0, sizeof(CUBE));
		MouseMoveMode = MMM_NONE;
		break;
	case WM_MOUSEMOVE:
		orthoType = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		HandleMouseMove(hWnd, LOWORD(lParam), HIWORD(lParam), wParam, orthoType);
		break;
	case WM_MOUSEWHEEL:
		orthoType = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		wheelMovement = HIWORD(wParam);
		if (wheelMovement == 120) {
			wctx[orthoType].scale -= 0.1f;
		}
		else {
			wctx[orthoType].scale += 0.1f;
		}
		if (wctx[orthoType].scale < 0.1f) wctx[orthoType].scale = 0.1f;
		InvalidateRect(hWnd, nullptr, true);
		break;
	case WM_SIZE:
		if (wctx[orthoType].bmpDbuf) {
			DeleteObject(wctx[orthoType].bmpDbuf);
		}
		tdc = GetDC(hWnd);
		wctx[orthoType].bmpDbuf = CreateCompatibleBitmap(tdc, LOWORD(lParam), HIWORD(lParam));
		ReleaseDC(hWnd, tdc);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (wctx[orthoType].bmpDbuf)
		{
			HDC ddc = CreateCompatibleDC(hdc);
			HGDIOBJ oldbmp = SelectObject(ddc, wctx[orthoType].bmpDbuf);
			DrawOrtho(hWnd, &ps, ddc);
			BitBlt(hdc, 0, 0, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
				ddc, 0, 0, SRCCOPY);
			SelectObject(ddc, oldbmp);
			DeleteDC(ddc);
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		DeleteObject(wctx[orthoType].bmpDbuf);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
