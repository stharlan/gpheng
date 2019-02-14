#pragma once

#include "resource.h"

#define MAX_LOADSTRING 100
#define F1 1.0f
#define F0 0.0f

typedef struct {
	physx::PxRigidDynamic* lpBulletDyn;
	physx::PxMat44 pose;
} BULLET_STRUCT;

typedef struct {
	float ox;
	float oy;
	float oz;
	float dx;
	float dy;
	float dz;
} CUBE;

