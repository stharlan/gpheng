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
