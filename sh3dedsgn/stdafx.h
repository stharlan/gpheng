// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// reference additional headers your program requires here
#include <gl/GL.h>
#include <iostream>
#include <objidl.h>
#include <gdiplus.h>
#include <strsafe.h>

// vectors and matrices
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/vec3.hpp"
#include "glm/geometric.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/norm.hpp"

#include "glcorearb.h"

#include "sh3dedsgn.h"

#define FMIN(a,b) (a < b ? a : b)
#define FMAX(a,b) (a > b ? a : b)
#define CONSTPI 3.14159f
#define DEG2RAD(x) (x / 180.0f * CONSTPI)
#define _FILLRECT(rect,l,t,r,b) rect.left=l;rect.top=t;rect.right=r;rect.bottom=b;

using namespace std;
using namespace Gdiplus;