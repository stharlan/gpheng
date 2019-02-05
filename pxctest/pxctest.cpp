// pxctest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <PxPhysicsAPI.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>

#pragma comment(lib, "Physx_64.lib")
#pragma comment(lib, "PhysxFoundation_64.lib")
#pragma comment(lib, "PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "PhysxCommon_64.lib")
#pragma comment(lib, "PhysxCooking_64.lib")
#pragma comment(lib, "PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")

#define PVD_HOST "127.0.0.1"

using namespace std;

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

int main()
{
	MyAllocator allocator;
	UserErrorCallback errcbk;

	physx::PxFoundation* mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errcbk);
	if (!mFoundation) {
		cout << "PxCreateFoundation failed!" << endl;
	}

	bool recordMemoryAllocations = true;

	physx::PxPvd* mPvd = physx::PxCreatePvd(*mFoundation);
	if (!mPvd) {
		cout << "failed to create pvd" << endl;
	}
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	if (!transport) {
		cout << "failed to create transport" << endl;
	}
	mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	physx::PxPhysics* mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation,
		physx::PxTolerancesScale(), recordMemoryAllocations, mPvd);
	if (!mPhysics) {
		cout << "PxCreatePhysics failed!" << endl;
	}

	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	physx::PxDefaultCpuDispatcher* gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	if (!gDispatcher) {
		cout << "failed to create dispatcher" << endl;
	}
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	physx::PxScene* gScene = mPhysics->createScene(sceneDesc);
	if (!gScene) {
		cout << "failed to create scene" << endl;
	}

	physx::PxMaterial* gMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.0f);
	physx::PxRigidStatic* groundPlane = physx::PxCreatePlane(*mPhysics, physx::PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);

	//physx::PxShape* shape = mPhysics->createShape(physx::PxSphereGeometry(1), *gMaterial);
	//physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(physx::PxTransform(physx::PxVec3(0, 40, 0)));
	//body->attachShape(*shape);
	//physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	//gScene->addActor(*body);

	physx::PxVec3 position(0, 40, 0);
	float radius = 1.0f;
	float halfHeight = 2.0f;
	physx::PxRigidDynamic* aCapsuleActor = mPhysics->createRigidDynamic(physx::PxTransform(position));
	// transform that rotates 90 deg about the z axis
	physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
	physx::PxShape* aCapsuleShape = physx::PxRigidActorExt::createExclusiveShape(*aCapsuleActor,
		physx::PxCapsuleGeometry(radius, halfHeight), *gMaterial);
	aCapsuleShape->setLocalPose(relativePose);
	physx::PxRigidBodyExt::updateMassAndInertia(*aCapsuleActor, 10.0f);
	gScene->addActor(*aCapsuleActor);

	cout << "ready to loop" << endl;
	for (int i = 0; i < 100; i++) {
		physx::PxShape* shapes[1];
		gScene->simulate(1.0f / 10.0f); // 100th of a sec
		gScene->fetchResults(true);
		physx::PxU32 n = aCapsuleActor->getNbShapes();
		aCapsuleActor->getShapes(shapes, n);
		physx::PxGeometryHolder gh = shapes[0]->getGeometry();
		const physx::PxMat44 shapePose(physx::PxShapeExt::getGlobalPose(*shapes[0], *aCapsuleActor));
		physx::PxVec3 pos = shapePose.getPosition();
		for (int i = 0; i < pos.y; i++) {
			cout << ".";
		}
		cout << "*" << endl;
		Sleep(100);
	}

	if (aCapsuleActor) aCapsuleActor->release();
	if (aCapsuleShape) aCapsuleShape->release();
	if (gMaterial) gMaterial->release();
	if (groundPlane) groundPlane->release();
	if (gScene) gScene->release();
	if (gDispatcher) gDispatcher->release();
	if (mPhysics) mPhysics->release();
	if (transport) transport->release();
	if (mPvd) mPvd->release();
	if (mFoundation) mFoundation->release();

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
