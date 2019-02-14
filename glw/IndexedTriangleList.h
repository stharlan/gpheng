#pragma once

class IndexedTriangleList
{
public:
	IndexedTriangleList();
	IndexedTriangleList(const IndexedTriangleList& p);
	IndexedTriangleList& operator=(IndexedTriangleList& p);
	~IndexedTriangleList();
	void FreeResources();
	void AddVertex(float vx, float vy, float vz, float tx, float ty, float ni, float nj, float nk);
	void AddTriIdices(unsigned int idx1, unsigned int idx2, unsigned int idx3);
	void SetGLBufferIds(GLuint vertex, GLuint index);
	void BindArrays();
	void BindBuffers();
	void BindAttribs();
	void UnbindAttribs();
	void DrawElements();
	void SetRigidDynamic(physx::PxRigidDynamic* lpDyn, physx::PxShape* lpShp);
	void SetRigidStatic(physx::PxRigidStatic* lpStatic, physx::PxShape* lpShp);
	physx::PxRigidDynamic* get_RigidDynamic();
	physx::PxRigidStatic* get_RigidStatic();
private:
	GLuint VertexArrayBuffer;
	GLuint IndexArrayBuffer;
	std::vector<float> Vertices;
	std::vector<unsigned int> Indices;
	physx::PxRigidStatic* pxRigidStatic;
	physx::PxRigidDynamic* pxRigidDynamic;
	physx::PxShape* pxShape;
};
