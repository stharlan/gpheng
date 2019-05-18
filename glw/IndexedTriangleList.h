#pragma once

class IndexedTriangleList
{
public:
	IndexedTriangleList();
	IndexedTriangleList(const IndexedTriangleList& p);
	IndexedTriangleList& operator=(IndexedTriangleList& p);
	~IndexedTriangleList();
	void FreeResources();
	void AddVertex(float vx, float vy, float vz, float tx, float ty, float ni, float nj, float nk, bool dbg = false);
	void AddTriIndices(unsigned int idx1, unsigned int idx2, unsigned int idx3, bool dbg = false);
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
	unsigned int GetFloatCount();
	unsigned int GetIntCount();
	void CalculateVertexNormals();
	void ReverseWinding();
	void SetModelMatrix(glm::mat4& m);
	inline float* GetModelMatrixPointer()
	{
		return &modelMatrix[0][0];
	}
	glm::mat3 GetNormalModelMatrix()
	{
		return glm::inverse(glm::transpose(glm::mat3(this->modelMatrix)));
	}

	static IndexedTriangleList CreateCone(float radius, float height, int slices);
	static IndexedTriangleList CreateCylinder(float radius, float height, int rings, int slices);
	static IndexedTriangleList CreateSphere(float radius, int rings, int slices);
	static IndexedTriangleList CreateSkyBox();
	static IndexedTriangleList CreateCubes(unsigned int numCubes, CUBE* cubeList, float UVDivisor);

	void GetVertex(unsigned int index, float *refFloats);
	unsigned int GetIndex(unsigned int index);

private:
	GLuint VertexArrayBuffer;
	GLuint IndexArrayBuffer;
	std::vector<float> Vertices;
	std::vector<unsigned int> Indices;
	physx::PxRigidStatic* pxRigidStatic;
	physx::PxRigidDynamic* pxRigidDynamic;
	physx::PxShape* pxShape;
	glm::mat4 modelMatrix;
};
