// skread.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <SketchUpAPI/slapi.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/vertex.h>
#include <vector>

#pragma comment(lib, "SketchUpAPI.lib")

using namespace std;

int main()
{
	SUInitialize();

	SUModelRef model = SU_INVALID;
	SUResult res = SUModelCreateFromFile(&model, "Building1.skp");

	if (res != SU_ERROR_NONE) return 1;

	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(model, &entities);

	// Get all the faces from the entities object
	size_t faceCount = 0;
	SUEntitiesGetNumFaces(entities, &faceCount);
	cout << "face count = " << faceCount << endl;
	if (faceCount > 0) {
		std::vector<SUFaceRef> faces(faceCount);
		SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);

		// Get all the edges in this face
		for (size_t i = 0; i < faceCount; i++) {
			size_t vertexCount = 0;
			SUFaceGetNumVertices(faces[i], &vertexCount);
			cout << vertexCount << " vertices on this face" << endl;
			vector<SUVertexRef> verts(vertexCount);
			size_t vertsGot = 0;
			SUFaceGetVertices(faces[i], vertexCount, &verts[0], &vertsGot);
			cout << "got " << vertsGot << " verts" << endl;
			for (size_t j = 0; j < vertsGot; j++) {
				
			}
		}
	}

	// Get model name
	SUStringRef name = SU_INVALID;
	SUStringCreate(&name);
	SUModelGetName(model, &name);
	size_t name_length = 0;
	SUStringGetUTF8Length(name, &name_length);
	char* name_utf8 = new char[name_length + 1];
	SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
	// Now we have the name in a form we can use
	SUStringRelease(&name);
	delete[]name_utf8;

	// Must release the model or there will be memory leaks
	SUModelRelease(&model);

	// Always terminate the API when done using it
	SUTerminate();
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
