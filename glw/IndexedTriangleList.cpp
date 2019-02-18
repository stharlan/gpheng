#pragma once

#include "stdafx.h"

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLVALIDATEPROGRAMPROC glValidateProgram;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLTEXSTORAGE2DPROC glTexStorage2D;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;

IndexedTriangleList::IndexedTriangleList() { }

IndexedTriangleList::IndexedTriangleList(const IndexedTriangleList& p) 
{
	this->VertexArrayBuffer = p.VertexArrayBuffer;
	this->IndexArrayBuffer = p.IndexArrayBuffer;
	this->Vertices = p.Vertices;
	this->Indices = p.Indices;
	this->pxRigidStatic = nullptr;
	this->pxRigidDynamic = nullptr;
	this->pxShape = nullptr;
}

IndexedTriangleList& IndexedTriangleList::operator=(IndexedTriangleList& p)
{
	this->VertexArrayBuffer = p.VertexArrayBuffer;
	this->IndexArrayBuffer = p.IndexArrayBuffer;
	this->Vertices = p.Vertices;
	this->Indices = p.Indices;
	this->pxRigidStatic = p.pxRigidStatic;
	this->pxRigidDynamic = p.pxRigidDynamic;
	this->pxShape = p.pxShape;
	return *this;
}

IndexedTriangleList::~IndexedTriangleList() {
}

void IndexedTriangleList::FreeResources() {
	glDeleteBuffers(1, &this->VertexArrayBuffer);
	glDeleteBuffers(1, &this->IndexArrayBuffer);
	if (this->pxShape) this->pxShape->release();
	if (this->pxRigidStatic) this->pxRigidStatic->release();
	if (this->pxRigidDynamic) this->pxRigidDynamic->release();
}

void IndexedTriangleList::AddVertex(float vx, float vy, float vz, float tx, float ty, float ni, float nj, float nk, bool dbg)
{
	this->Vertices.push_back(vx);
	this->Vertices.push_back(vy);
	this->Vertices.push_back(vz);
	this->Vertices.push_back(tx);
	this->Vertices.push_back(ty);
	this->Vertices.push_back(ni);
	this->Vertices.push_back(nj);
	this->Vertices.push_back(nk);
	if (true == dbg) std::cout << "tex " << tx << ", " << ty << std::endl;
}

void IndexedTriangleList::AddTriIndices(unsigned int idx1, unsigned int idx2, unsigned int idx3, bool dbg)
{
	this->Indices.push_back(idx1);
	this->Indices.push_back(idx2);
	this->Indices.push_back(idx3);
	if(true == dbg) std::cout << "Adding indices " << idx1 << ", " << idx2 << ", " << idx3 << std::endl;
}

void IndexedTriangleList::SetGLBufferIds(GLuint vertex, GLuint index)
{
	this->VertexArrayBuffer = vertex;
	this->IndexArrayBuffer = index;
}

void IndexedTriangleList::BindArrays()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->VertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, this->Vertices.size() * sizeof(float), this->Vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->Indices.size() * sizeof(unsigned int), this->Indices.data(), GL_STATIC_DRAW);
}

void IndexedTriangleList::BindBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, VertexArrayBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexArrayBuffer);
}

void IndexedTriangleList::BindAttribs()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
}

void IndexedTriangleList::UnbindAttribs()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void IndexedTriangleList::DrawElements()
{
	glDrawElements(GL_TRIANGLES, (GLsizei)Indices.size(), GL_UNSIGNED_INT, 0);
}

void IndexedTriangleList::SetRigidDynamic(physx::PxRigidDynamic* lpDyn, physx::PxShape* lpShp)
{
	this->pxRigidDynamic = lpDyn;
	this->pxShape = lpShp;
}

void IndexedTriangleList::SetRigidStatic(physx::PxRigidStatic* lpStatic, physx::PxShape* lpShp)
{
	this->pxRigidStatic = lpStatic;
	this->pxShape = lpShp;
}

physx::PxRigidDynamic* IndexedTriangleList::get_RigidDynamic() { return this->pxRigidDynamic; }

physx::PxRigidStatic* IndexedTriangleList::get_RigidStatic() { return this->pxRigidStatic; }

unsigned int IndexedTriangleList::GetFloatCount()
{
	return (unsigned int)this->Vertices.size();
}

unsigned int IndexedTriangleList::GetIntCount()
{
	return (unsigned int)this->Indices.size();
}

void IndexedTriangleList::ReverseWinding()
{
	unsigned int nidx = (unsigned int)this->Indices.size();
	unsigned int tempi = 0;
	for (unsigned int i = 0; i < nidx; i += 3)
	{
		tempi = this->Indices[i + 1];
		this->Indices[i + 1] = this->Indices[i + 2];
		this->Indices[i + 2] = tempi;
	}
}

void IndexedTriangleList::CalculateVertexNormals()
{
	glm::vec3 sumCross;
	unsigned int nvrt = (unsigned int)this->Vertices.size() / 8;
	unsigned int nidx = (unsigned int)this->Indices.size();
	glm::vec3 va;
	glm::vec3 vb;
	glm::vec3 vc;
	glm::vec3 vab;
	glm::vec3 vac;
	glm::vec3 xabc;
	glm::vec3 sumNormal;

	for (unsigned int v = 0; v < nvrt; v++) {
		sumCross = glm::vec3(0.0f, 0.0f, 0.0f);
		for (unsigned int i = 0; i < nidx; i += 3)
		{

			if (this->Indices[i] == v || this->Indices[i + 1] == v || this->Indices[i + 2] == v)
			{
				// calculate a face normal
				va = glm::vec3(
					this->Vertices[this->Indices[i] * 8],
					this->Vertices[(this->Indices[i] * 8) + 1],
					this->Vertices[(this->Indices[i] * 8) + 2]);

				vb = glm::vec3(
					this->Vertices[this->Indices[i + 1] * 8],
					this->Vertices[(this->Indices[i + 1] * 8) + 1],
					this->Vertices[(this->Indices[i + 1] * 8) + 2]);

				vc = glm::vec3(
					this->Vertices[this->Indices[i + 2] * 8],
					this->Vertices[(this->Indices[i + 2] * 8) + 1],
					this->Vertices[(this->Indices[i + 2] * 8) + 2]);

				vab = vb - va;
				vac = vc - va;
				xabc = glm::cross(vac, vab);
				sumCross += xabc;
			}
		}

		sumNormal = glm::normalize(sumCross);
		this->Vertices[(v * 8) + 5] = sumNormal[0];
		this->Vertices[(v * 8) + 6] = sumNormal[1];
		this->Vertices[(v * 8) + 7] = sumNormal[2];
	}
}

IndexedTriangleList IndexedTriangleList::CreateSkyBox()
{
	IndexedTriangleList itl = IndexedTriangleList::CreateSphere(710.0f, 4, 4);
	itl.ReverseWinding();
	itl.CalculateVertexNormals();
	itl.BindArrays();
	return itl;
}

IndexedTriangleList IndexedTriangleList::CreateSphere(float radius, int rings, int slices)
{
	IndexedTriangleList itl;
	GLuint buffers[2];

	// number of triangles
	// first and last ring have 1 tri per slice
	// the rest have two tri's per slice
	int ntri = (2 * slices) + ((rings - 2) * slices * 2);
	int nvrt = ((rings - 1) * slices) + 2;
	int trisPerRing = 0;
	if (rings > 2) trisPerRing = 2 * slices;
	float theta, phi;
	float sliceInterval = 360.0f / slices;
	float phiInterval = 180.0f / rings;

	// first vertex
	itl.AddVertex(0, radius, 0, 0.0f, 0.0f, 0, 0, 0);

	for (phi = phiInterval; phi < 180.0f; phi += phiInterval)
	{
		float phir = phi / 180.0f * CONSTPI;
		for (theta = 0; theta < 360.0f; theta += sliceInterval)
		{
			float thetar = theta / 180.0f * CONSTPI;
			itl.AddVertex(radius * cosf(thetar) * sinf(phir), radius * cosf(phir), radius * sinf(thetar) * sinf(phir),
				theta / 360.0f, phi / 180.0f,
				0, 0, 0);
		}
	}

	// last vertex
	itl.AddVertex(0, -radius, 0, 0.0f, 1.0f, 0, 0, 0);


	// 0, 1, 2
	// slices - 1 = 2
	for (int i = 0; i < slices; i++) {

		if (i == (slices - 1)) { // last slice
			itl.AddTriIndices(0, i + 1, 1);
		}
		else {
			itl.AddTriIndices(0, i + 1, i + 2);
		}

		if (i == (slices - 1)) { // last slice
			itl.AddTriIndices(nvrt - 1, (nvrt - slices) - 1, nvrt - 2);
		}
		else {
			itl.AddTriIndices(nvrt - 1, (nvrt - slices) + i, ((nvrt - slices) + i) - 1);
		}
	}

	for (int r = 1; r < (rings - 1); r++) {
		for (int s = 0; s < slices; s++) {

			int vertsToAdd = (r - 1) * slices;

			if (s == (slices - 1)) {
				// last slice
				itl.AddTriIndices(s + 1 + vertsToAdd, s + 1 + slices + vertsToAdd, (s - slices) + 2 + vertsToAdd);
			}
			else {
				itl.AddTriIndices(s + 1 + vertsToAdd, s + 1 + slices + vertsToAdd, s + 2 + vertsToAdd);
			}

			if (s == (slices - 1)) {
				// last slice
				itl.AddTriIndices((s - slices) + 2 + vertsToAdd, s + 1 + slices + vertsToAdd, s + 2 + vertsToAdd);
			}
			else {
				itl.AddTriIndices(s + 2 + vertsToAdd, s + 1 + slices + vertsToAdd, s + 2 + slices + vertsToAdd);
			}

		}
	}

	//itl.ReverseWinding();
	itl.CalculateVertexNormals();

	glGenBuffers(2, buffers);
	itl.SetGLBufferIds(buffers[0], buffers[1]);
	itl.BindArrays();

	return itl;

}

IndexedTriangleList IndexedTriangleList::CreateCylinder(float radius, float height, int rings, int slices)
{
	IndexedTriangleList itl;
	GLuint buffers[2];
	float halfHeight = height / 2.0f;
	float rad = 0.0f;
	float hgt = 0.0f;

	// top
	itl.AddVertex(0.0f, halfHeight, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	float thstep = 360.0f / (float)slices;
	for (int s = 0; s < slices; s++) {
		rad = DEG2RAD(thstep * s);
		itl.AddVertex(radius * cosf(rad), halfHeight, radius * sinf(rad), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	}
	for (int s = 0; s < slices; s++) {
		if (s == (slices - 1)) {
			itl.AddTriIndices(0, s + 1, slices - s);
		}
		else {
			itl.AddTriIndices(0, s + 1, s + 2);
		}
	}

	// bottom
	int base = itl.GetFloatCount() / 8;
	itl.AddVertex(0.0f, -halfHeight, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
	for (int s = 0; s < slices; s++) {
		rad = DEG2RAD(thstep * (float)s);
		itl.AddVertex(radius * cosf(rad), -halfHeight, radius * sinf(rad), 0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
	}
	for (int s = 0; s < slices; s++) {
		if (s == (slices - 1)) {
			itl.AddTriIndices(base, (slices - s) + base, (s + 1) + base);
		}
		else {
			itl.AddTriIndices(base, (s + 2) + base, (s + 1) + base);
		}
	}

	base = itl.GetFloatCount() / 8;
	float hstep = height / (float)rings;
	for (int r = 0; r < (rings + 1); r++) {
		hgt = halfHeight - (hstep * (float)r);
		for (int s = 0; s < slices; s++) {
			rad = DEG2RAD(thstep * (float)s);
			itl.AddVertex(radius * cosf(rad), hgt, radius * sinf(rad), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < slices; s++) {
			if (s == slices - 1) {
				itl.AddTriIndices(
					base + (slices * r) + s,
					base + (slices * (r + 1)) + ((s - slices) + 1),
					base + (slices * r) + ((s - slices) + 1));
				itl.AddTriIndices(
					base + (slices * r) + s,
					base + (slices * (r + 1)) + s,
					base + (slices * (r + 1)) + ((s - slices) + 1));
			}
			else {
				itl.AddTriIndices(
					base + (slices * r) + s,
					base + (slices * (r + 1)) + (s + 1),
					base + (slices * r) + (s + 1));
				itl.AddTriIndices(
					base + (slices * r) + s,
					base + (slices * (r + 1)) + s,
					base + (slices * (r + 1)) + (s + 1));
			}
		}
	}

	itl.CalculateVertexNormals();

	glGenBuffers(2, buffers);
	itl.SetGLBufferIds(buffers[0], buffers[1]);
	itl.BindArrays();
	return itl;

}

IndexedTriangleList IndexedTriangleList::CreateCone(float radius, float height, int slices)
{
	IndexedTriangleList itl;
	GLuint buffers[2];
	float rad = 0.0f;
	float halfHeight = height / 2.0f;

	float thstep = 360.0f / (float)slices;
	for (int s = 0; s < slices; s++) {
		rad = DEG2RAD(thstep * s);
		itl.AddVertex(radius * cosf(rad), -halfHeight, radius * sinf(rad), 0.0f, 0.0f, 0.0f, 0.0f, 0.0);
	}

	for (int s = 0; s < slices; s++) {
		itl.AddVertex(0.0f, halfHeight, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		if (s == (slices - 1)) {
			itl.AddTriIndices(s, (s - slices) + 1, slices + s);
		}
		else {
			itl.AddTriIndices(s, s + 1, slices + s);
		}
	}

	int base = itl.GetFloatCount() / 8;
	itl.AddVertex(0.0f, -halfHeight, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	for (int s = 0; s < slices; s++) {
		rad = DEG2RAD(thstep * s);
		itl.AddVertex(radius * cosf(rad), -halfHeight, radius * sinf(rad), 0.0f, 0.0f, 0.0f, 0.0f, 0.0);
	}
	for (int s = 0; s < slices; s++) {
		if (s == (slices - 1))
		{
			itl.AddTriIndices(base, base + (s - slices) + 2, base + s + 1);
		}
		else {
			itl.AddTriIndices(base, base + s + 2, base + s + 1);
		}
	}

	itl.CalculateVertexNormals();
	glGenBuffers(2, buffers);
	itl.SetGLBufferIds(buffers[0], buffers[1]);
	itl.BindArrays();
	return itl;
}

IndexedTriangleList IndexedTriangleList::CreateCubes(unsigned int numCubes, CUBE* cubeList)
{

	IndexedTriangleList trilist;

	GLuint buffers[2];

	// for testing this is only the x/y face
	// 4 * 8 = one face
	// 8 * 8 = two faces
	// 12 * 8 = three faces
	// 16 * 8 = four faces
	int CubeDataSize = 24 * 8; // 24 * 8;
	int VertDataSize = CubeDataSize * numCubes;
	//lpList->Vertices = (float*)malloc(VertDataSize * sizeof(float));
	//memset(lpList->Vertices, 0, VertDataSize * sizeof(float));

	// for testing this is only the x/y face
	// 6 = one face
	// 12 = two faces
	// 18 = three faces
	// 24 = four faces
	int CubeIndexSize = 36; // 36;
	int IndexDataSize = CubeIndexSize * numCubes;
	//lpList->Indices = (unsigned int*)malloc(IndexDataSize * sizeof(unsigned int));
	//memset(lpList->Indices, 0, IndexDataSize * sizeof(unsigned int));

	for (unsigned int c = 0; c < numCubes; c++) {
		//unsigned int* iptr = (unsigned int*)(lpList->Indices + (c * CubeIndexSize));
		//float* ptr = (float*)(lpList->Vertices + (c * CubeDataSize));
		CUBE* cb = cubeList + c;

		// face 1 vertices x/y neg z
		// pos x,y,z		                                                    tex u,v		                     norm i,j,k
		trilist.AddVertex(cb->ox, cb->oy, cb->oz, 0.0f, cb->dy / 2.0f, 0.0f, 0.0f, -1.0f);
		trilist.AddVertex(cb->ox, cb->oy + cb->dy, cb->oz, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy + cb->dy, cb->oz, cb->dx / 2.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy, cb->oz, cb->dx / 2.0f, cb->dy / 2.0f, 0.0f, 0.0f, -1.0f);

		// face 1 indexes
		trilist.AddTriIndices((c * 24) + 0, (c * 24) + 2, (c * 24) + 1);
		trilist.AddTriIndices((c * 24) + 0, (c * 24) + 3, (c * 24) + 2);

		// face 2 vertices x/y pos z
		// pos x,y,z
		trilist.AddVertex(cb->ox, cb->oy, cb->oz + cb->dz, 0.0f, cb->dy / 2.0f, 0.0f, 0.0f, 1.0f);
		trilist.AddVertex(cb->ox, cb->oy + cb->dy, cb->oz + cb->dz, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy + cb->dy, cb->oz + cb->dz, cb->dx / 2.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy, cb->oz + cb->dz, cb->dx / 2.0f, cb->dy / 2.0f, 0.0f, 0.0f, 1.0f);

		trilist.AddTriIndices((c * 24) + 4, (c * 24) + 5, (c * 24) + 6);
		trilist.AddTriIndices((c * 24) + 4, (c * 24) + 6, (c * 24) + 7);

		// face 3 vertices x/y pos z
		trilist.AddVertex(cb->ox, cb->oy, cb->oz, 0.0f, cb->dz / 2.0f, 0.0f, -1.0f, 0.0f);
		trilist.AddVertex(cb->ox, cb->oy, cb->oz + cb->dz, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy, cb->oz + cb->dz, cb->dx / 2.0f, 0.0f, 0.0f, -1.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy, cb->oz, cb->dx / 2.0f, cb->dz / 2.0f, 0.0f, -1.0f, 0.0f);

		// face 3 indexes
		trilist.AddTriIndices((c * 24) + 8, (c * 24) + 9, (c * 24) + 10);
		trilist.AddTriIndices((c * 24) + 8, (c * 24) + 10, (c * 24) + 11);

		// face 4 vertices x/y pos z
		trilist.AddVertex(cb->ox, cb->oy + cb->dy, cb->oz, 0.0f, cb->dz / 2.0f, 0.0f, 1.0f, 0.0f);
		trilist.AddVertex(cb->ox, cb->oy + cb->dy, cb->oz + cb->dz, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy + cb->dy, cb->oz + cb->dz, cb->dx / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy + cb->dy, cb->oz, cb->dx / 2.0f, cb->dz / 2.0f, 0.0f, 1.0f, 0.0f);

		// face 4 indexes
		trilist.AddTriIndices((c * 24) + 12, (c * 24) + 14, (c * 24) + 13);
		trilist.AddTriIndices((c * 24) + 12, (c * 24) + 15, (c * 24) + 14);

		// face 4 vertices x/y pos z
		trilist.AddVertex(cb->ox, cb->oy, cb->oz, 0.0f, cb->dz / 2.0f, -1.0f, 0.0f, 0.0f);
		trilist.AddVertex(cb->ox, cb->oy, cb->oz + cb->dz, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		trilist.AddVertex(cb->ox, cb->oy + cb->dy, cb->oz + cb->dz, cb->dy / 2.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		trilist.AddVertex(cb->ox, cb->oy + cb->dy, cb->oz, cb->dy / 2.0f, cb->dz / 2.0f, -1.0f, 0.0f, 0.0f);

		// face 4 indexes
		trilist.AddTriIndices((c * 24) + 16, (c * 24) + 18, (c * 24) + 17);
		trilist.AddTriIndices((c * 24) + 16, (c * 24) + 19, (c * 24) + 18);

		// face 4 vertices x/y pos z
		trilist.AddVertex(cb->ox + cb->dx, cb->oy, cb->oz, 0.0f, cb->dz / 2.0f, 1.0f, 0.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy, cb->oz + cb->dz, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy + cb->dy, cb->oz + cb->dz, cb->dy / 2.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		trilist.AddVertex(cb->ox + cb->dx, cb->oy + cb->dy, cb->oz, cb->dy / 2.0f, cb->dz / 2.0f, 1.0f, 0.0f, 0.0f);

		// face 4 indexes
		trilist.AddTriIndices((c * 24) + 20, (c * 24) + 21, (c * 24) + 22);
		trilist.AddTriIndices((c * 24) + 20, (c * 24) + 22, (c * 24) + 23);
	}

	glGenBuffers(2, buffers);
	//trilist.VertexArrayBuffer = buffers[0];
	//trilist.IndexArrayBuffer = buffers[1];
	trilist.SetGLBufferIds(buffers[0], buffers[1]);
	//lpList->NumIndices = IndexDataSize;

	//glBindBuffer(GL_ARRAY_BUFFER, lpList->VertexArrayBuffer);
	//glBufferData(GL_ARRAY_BUFFER, VertDataSize * sizeof(float), lpList->Vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lpList->IndexArrayBuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexDataSize * sizeof(unsigned int), lpList->Indices, GL_STATIC_DRAW);
	trilist.BindArrays();

	return trilist;
}

void IndexedTriangleList::SetModelMatrix(glm::mat4& m)
{
	this->modelMatrix = m;
}
