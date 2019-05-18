
#include "stdafx.h"

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern std::ostream& operator<<(std::ostream& os, const glm::vec3& dt);

#define NEXT_TOKEN(f,t) f >> t;if(f.eof()) goto is_eof
#define EAT_TOKEN(f,t) f >> t
#define IF_TOKEN_ASSIGN(f,t,v,d) if(!t.compare(v)){std::getline(std::getline(f,t,'"'),d,'"');}
#define IF_TOKEN_QUIT(f,t,v) if(!t.compare(v)) goto is_eof;
#define PARSE_SECTION(s,f,t,fn,d) if (!t.compare(s)) { \
	EAT_TOKEN(f,t); \
	fn(f, d); }
#define PARSE_SECTION_ADD(s,f,t,fn,v) if (!t.compare(s)) { \
	EAT_TOKEN(f,t); \
	v.push_back(fn(f)); }

void eatHidden(std::ifstream &f)
{
	std::string t;
	int bracketCounter = 0;
	while (true) {
		NEXT_TOKEN(f, t);
		if (t.compare("{") == 0) {
			bracketCounter++;
		}
		else if (t.compare("}") == 0) {
			bracketCounter--;
			if (bracketCounter == 0) {
				goto is_eof;
			}
		}
	}
is_eof:
	return;
}

void parseVersionInfo(std::ifstream &f, VMF_FILE_DATA &d)
{
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"editorversion\"", d.versionInfo.editorversion);
		IF_TOKEN_ASSIGN(f, t, "\"editorbuild\"", d.versionInfo.editorbuild);
		IF_TOKEN_ASSIGN(f, t, "\"mapversion\"", d.versionInfo.mapversion);
		IF_TOKEN_ASSIGN(f, t, "\"formatversion\"", d.versionInfo.formatversion);
		IF_TOKEN_ASSIGN(f, t, "\"prefab\"", d.versionInfo.prefab);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return;
}

VISGROUP parseVisgroupCreate(std::ifstream &f)
{
	VISGROUP item;
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"name\"", item.name);
		IF_TOKEN_ASSIGN(f, t, "\"visgroupid\"", item.visgroupid);
		IF_TOKEN_ASSIGN(f, t, "\"color\"", item.color);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return item;
}

void parseVisGroups(std::ifstream &f, VMF_FILE_DATA &d)
{
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		PARSE_SECTION_ADD("visgroup", f, t, parseVisgroupCreate, d.visgroups);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return;
}

void parseViewSettings(std::ifstream &f, VMF_FILE_DATA &d)
{
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"bSnapToGrid\"", d.viewSettings.bSnapToGrid);
		IF_TOKEN_ASSIGN(f, t, "\"bShowGrid\"", d.viewSettings.bShowGrid);
		IF_TOKEN_ASSIGN(f, t, "\"bShowLogicalGrid\"", d.viewSettings.bShowLogicalGrid);
		IF_TOKEN_ASSIGN(f, t, "\"nGridSpacing\"", d.viewSettings.nGridSpacing);
		IF_TOKEN_ASSIGN(f, t, "\"bShow3DGrid\"", d.viewSettings.bShow3DGrid);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return;
}

SIDE parseSideCreate(std::ifstream &f)
{
	SIDE newSide;
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"id\"", newSide.id);
		IF_TOKEN_ASSIGN(f, t, "\"plane\"", newSide.plane);
		IF_TOKEN_ASSIGN(f, t, "\"material\"", newSide.material);
		IF_TOKEN_ASSIGN(f, t, "\"uaxis\"", newSide.uaxis);
		IF_TOKEN_ASSIGN(f, t, "\"vaxis\"", newSide.vaxis);
		IF_TOKEN_ASSIGN(f, t, "\"rotation\"", newSide.rotation);
		IF_TOKEN_ASSIGN(f, t, "\"lightmapscale\"", newSide.lightmapscale);
		IF_TOKEN_ASSIGN(f, t, "\"smoothing_groups\"", newSide.smoothing_groups);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return newSide;
}

void parseEditor(std::ifstream &f, EDITOR &d)
{
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"color\"", d.color);
		IF_TOKEN_ASSIGN(f, t, "\"groupid\"", d.groupid);
		IF_TOKEN_ASSIGN(f, t, "\"visgroupshown\"", d.visgroupshown);
		IF_TOKEN_ASSIGN(f, t, "\"visgroupautoshown\"", d.visgroupautoshown);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return;
}

GROUP parseGroupCreate(std::ifstream &f)
{
	GROUP newItem;
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"id\"", newItem.id);
		PARSE_SECTION("editor", f, t, parseEditor, newItem.editor);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return newItem;
}

SOLID parseSolidCreate(std::ifstream &f)
{
	SOLID newSolid;
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"id\"", newSolid.id);
		PARSE_SECTION_ADD("side", f, t, parseSideCreate, newSolid.sides);
		PARSE_SECTION("editor", f, t, parseEditor, newSolid.editor);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return newSolid;
}

void parseWorld(std::ifstream &f, VMF_FILE_DATA &d)
{
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"id\"", d.world.id);
		IF_TOKEN_ASSIGN(f, t, "\"mapversion\"", d.world.mapversion);
		IF_TOKEN_ASSIGN(f, t, "\"classname\"", d.world.classname);
		IF_TOKEN_ASSIGN(f, t, "\"detailmaterial\"", d.world.detailmaterial);
		IF_TOKEN_ASSIGN(f, t, "\"detailvbsp\"", d.world.detailvbsp);
		IF_TOKEN_ASSIGN(f, t, "\"maxpropscreenwidth\"", d.world.maxpropscreenwidth);
		IF_TOKEN_ASSIGN(f, t, "\"skyname\"", d.world.skyname);
		PARSE_SECTION_ADD("solid", f, t, parseSolidCreate, d.world.solids);
		PARSE_SECTION_ADD("group", f, t, parseGroupCreate, d.world.groups);
		IF_TOKEN_QUIT(f, t, "}");
		if (t.compare("hidden") == 0) eatHidden(f);
	}
is_eof:
	return;
}

ENTITY parseEntityCreate(std::ifstream &f)
{
	ENTITY item;
	std::string t;
	while (true) {
		NEXT_TOKEN(f, t);
		IF_TOKEN_ASSIGN(f, t, "\"id\"", item.id);
		IF_TOKEN_ASSIGN(f, t, "\"classname\"", item.classname);
		IF_TOKEN_ASSIGN(f, t, "\"origin\"", item.origin);
		PARSE_SECTION_ADD("solid", f, t, parseSolidCreate, item.solidList);
		IF_TOKEN_QUIT(f, t, "}");
	}
is_eof:
	return item;
}

void ParseVMFFile(const char* VmfFileName, VMF_FILE_DATA &d)
{
	std::ifstream vmfFile(VmfFileName);
	std::string token;
	while (true) {
		NEXT_TOKEN(vmfFile, token);
		PARSE_SECTION("versioninfo", vmfFile, token, parseVersionInfo, d);
		PARSE_SECTION("visgroups", vmfFile, token, parseVisGroups, d);
		PARSE_SECTION("viewsettings", vmfFile, token, parseViewSettings, d);
		PARSE_SECTION("world", vmfFile, token, parseWorld, d);
		PARSE_SECTION_ADD("entity", vmfFile, token, parseEntityCreate, d.entityList);
		if (token.compare("hidden") == 0) eatHidden(vmfFile);
	}
is_eof:
	return;
}

void GetIntersection(std::vector<PLANE_VERTICES> &PlaneDataList, 
	unsigned int pi1, unsigned int pi2, unsigned int pi3,
	std::vector<glm::vec3> &vertices)
{

	const PLANE_VERTICES& p1 = PlaneDataList.at(pi1);
	const PLANE_VERTICES& p2 = PlaneDataList.at(pi2);
	const PLANE_VERTICES& p3 = PlaneDataList.at(pi3);

	// need to define a tolerance for d
	//float D_TOLERANCE = 0.005f;

	//glm::vec3 n1 = glm::cross(p1.v1 - p1.v2, p1.v3 - p1.v2);
	//glm::vec3 n2 = glm::cross(p2.v1 - p2.v2, p2.v3 - p2.v2);
	//glm::vec3 n3 = glm::cross(p3.v1 - p3.v2, p3.v3 - p3.v2);

	//std::cout << "====> GetIntersection" << std::endl;
	//std::cout << "p1 (" << p1.planeId << ") normal is " << p1.planeNormal << "; d = " << p1.d << std::endl;
	//std::cout << "p2 (" << p2.planeId << ") normal is " << p2.planeNormal << "; d = " << p2.d << std::endl;
	//std::cout << "p3 (" << p3.planeId << ") normal is " << p3.planeNormal << "; d = " << p3.d << std::endl;

	float den = glm::dot(p1.planeNormal, glm::cross(p2.planeNormal, p3.planeNormal));

	//std::cout << "den = " << den << std::endl;

	if (den == 0) {
	//if(fabs(den) < D_TOLERANCE) {
		//std::cout << "no intersection" << std::endl;
		return;
	}

	float d1 = p1.planeNormal[0] * p1.v1[0] + p1.planeNormal[1] * p1.v1[1] + p1.planeNormal[2] * p1.v1[2];
	float d2 = p2.planeNormal[0] * p2.v1[0] + p2.planeNormal[1] * p2.v1[1] + p2.planeNormal[2] * p2.v1[2];
	float d3 = p3.planeNormal[0] * p3.v1[0] + p3.planeNormal[1] * p3.v1[1] + p3.planeNormal[2] * p3.v1[2];

	glm::vec3 c23 = glm::cross(p2.planeNormal, p3.planeNormal);
	glm::vec3 c31 = glm::cross(p3.planeNormal, p1.planeNormal);
	glm::vec3 c12 = glm::cross(p1.planeNormal, p2.planeNormal);
	glm::vec3 d1n2n3 = (-1.0f * p1.d) * c23;
	glm::vec3 d2n3n1 = (-1.0f * p2.d) * c31;
	glm::vec3 d3n1n2 = (-1.0f * p3.d) * c12;
	glm::vec3 num = d1n2n3 + d2n3n1 + d3n1n2;

	//std::cout << "num = " << num << std::endl;

	glm::vec3 newVertex = num / den;
	//std::cout << "New vertex is: " << newVertex << std::endl;

	bool AlreadyAdded = false;
	std::vector<glm::vec3>::iterator iter = vertices.begin();
	for (; iter != vertices.end(); ++iter) {
		if (*iter == newVertex) {
			AlreadyAdded = true;
			break;
		}
	}

	// check if it's outside any plane
	std::vector<PLANE_VERTICES>::iterator vi = PlaneDataList.begin();
	bool IllegalAdd = false;
	for (; vi != PlaneDataList.end(); vi++) {
		float val = glm::dot(vi->planeNormal, newVertex) + vi->d;
		//std::cout << "** DOT WITH NORMAL (" << vi->planeId << ") = " << val << std::endl;
		if (val < -0.1f) {
			IllegalAdd = true;
		}
	}

	if (false == AlreadyAdded && false == IllegalAdd) {
		//std::cout << "adding vertex" << std::endl;
		vertices.push_back(newVertex);
	}
	else {
		//std::cout << "Vertex on plane with id '" << p1.planeId << " and material '" << p1.material << "' cannot be rendered ";
		if (true == AlreadyAdded) {
			//std::cout << "because it's already been added" << std::endl;
		}
		else if (true == IllegalAdd) {
			//std::cout << "because it's illegal" << std::endl;
		}
		else {
			//std::cout << "for unknown reasons" << std::endl;
		}
	}
}

//If you have a plane, you have a normal vector and an origin.I wouldn't do any "rotations" at all. You're just a few vector operations away from your answer.
//•Let's call your plane's normal vector the new z axis.
//•You can generate the new y axis by crossing the old x axis with the new z axis(your plane's normal).
//	•Generate the new x axis by crossing the new z with the new y.
//	•Make all your new axis vectors into unit vectors(length 1).
//	•For every point you have, create a vector that's from your new origin to the point (vector subtraction of point - plane_origin). Just dot with the new x and new y unit vectors and you get a pair (x,y) you can plot!
//
//	If you have cross and dot product functions already, this is just a few lines of code.I know it works because most of the 3D videogames I wrote worked this way.
//
//	Tricks:
//•Pay attention to which directions your vectors are pointing.If they point the wrong way, negate the resultant vector or change the order of the cross product.
//•You have trouble if your plane's normal is exactly the same as your original x axis.

void CalcConvexHullAndCreatCallList(std::vector<glm::vec3> &VerticesForPlane, IndexedTriangleList &itl)
{

	if (VerticesForPlane.size() < 3) {
		std::cout << "ERROR! Vertices for plane has less than 3 pts" << std::endl;
		return;
	}

	// filter out duplicates
	//std::cout << "CalcConvexHullAndCreatCallList: cull vertices: before " << VerticesForPlane.size() << std::endl;
	std::vector<glm::vec3> CulledList;
	std::vector<glm::vec3>::iterator v3iter = VerticesForPlane.begin();
	for (; v3iter != VerticesForPlane.end(); ++v3iter)
	{
		bool CanAdd = true;
		std::vector<glm::vec3>::iterator clIter = CulledList.begin();
		for (; clIter != CulledList.end(); ++clIter) {
			if (clIter->x == v3iter->x && clIter->y == v3iter->y && clIter->z == v3iter->z) {
				CanAdd = false;
			}
		}
		if (true == CanAdd) {
			CulledList.push_back(*v3iter);
		}
	}
	//std::cout << "CalcConvexHullAndCreatCallList: cull vertices: after " << CulledList.size() << std::endl;

	std::vector<quickhull::Vector3<GLdouble>> pointCloud;
	//for (int i = 0; i < VerticesForPlane.size(); i++) {
		//std::cout << "qhull: adding " << VerticesForPlane[i] << std::endl;
		//quickhull::Vector3<GLdouble> v3(VerticesForPlane[i].x, VerticesForPlane[i].y, VerticesForPlane[i].z);
		//pointCloud.push_back(v3);
	//}

	std::vector<glm::vec3>::iterator clIter = CulledList.begin();
	for (; clIter != CulledList.end(); ++clIter) {
		quickhull::Vector3<GLdouble> v3(clIter->x, clIter->y, clIter->z);
		pointCloud.push_back(v3);
	}

	//std::cout << "point cloud has " << pointCloud.size() << " points" << std::endl;

	quickhull::QuickHull<GLdouble> qh;
	quickhull::ConvexHull<GLdouble> hull = qh.getConvexHull(pointCloud, true, false);
	std::vector<quickhull::IndexType> indices = hull.getIndexBuffer();
	quickhull::VertexDataSource<GLdouble> vertices = hull.getVertexBuffer();

	/*
	// need to remove duplicates again,
	// but, this time, if a vertex is indexed
	// need to re-assign to new index
	std::vector<glm::vec3> NewVertexList;
	for (int vi = 0; vi < vertices.size(); vi++) {
		std::vector<glm::vec3>::iterator nvlIter = NewVertexList.begin();
		bool AlreadyAdded = false;
		for (; nvlIter != NewVertexList.end(); ++nvlIter)
		{
			if(nvlIter->x == )
		}
	}
	*/

	//std::cout << "cvx hull" << std::endl;
	for (int v = 0; v < vertices.size(); v++) {
		itl.AddVertex(
			vertices[v].z / 12.0f, vertices[v].y / 12.0f, vertices[v].x / 12.0f,
			vertices[v].z / 100.0f, vertices[v].y / 100.0f,
			0.0f, 0.0f, 0.0f);
		//std::cout << vertices[v].z / 12.0f << ", " << vertices[v].y / 12.0f << ", " << vertices[v].x / 12.0f << std::endl;
	}

	for (int i = 0; i < indices.size(); i+=3) {
		itl.AddTriIndices(indices[i], indices[i + 1], indices[i + 2]);
		//std::cout << indices[i] << ", " << indices[i + 1] << ", " << indices[i + 2] << std::endl;
		//std::cout << "quick hull " << vertices[indices[i]].x << ", " << vertices[indices[i]].y << ", " << vertices[indices[i]].z << std::endl;

		// calculate text coords
		//glm::vec3 vertex((float)vertices[indices[i]].x, (float)vertices[indices[i]].y, (float)vertices[indices[i]].z);
		//float tu = ((glm::dot(vertex, PlaneData.uNormal) / 512.0f) / PlaneData.uScale) + (PlaneData.uTrans / 512.0f);
		//float tv = ((glm::dot(vertex, PlaneData.vNormal) / 512.0f) / PlaneData.vScale) + (PlaneData.vTrans / 512.0f);

		//std::cout << tu << ", " << tv << std::endl;

		//itl.AddVertex(
			//(float)vertices[indices[i]].z / 12.0f, (float)vertices[indices[i]].y / 12.0f, (float)vertices[indices[i]].x / 12.0f,
			//(float)vertices[indices[i]].z / 100.0f, (float)vertices[indices[i]].y / 100.0f,
			//0.0f, 0.0f,
			//0.0f, 0.0f, 0.0f);
			//PlaneNormal[0], PlaneNormal[1], PlaneNormal[2]);
		//if ((i+1) % 3 == 0) {
			//unsigned int base = (itl.GetFloatCount() / 8) - 3;
			//itl.AddTriIndices(base, base + 1, base + 2);
		//}
	}
}

bool GetPolygonFromPlanes(std::vector<PLANE_VERTICES> &PlaneDataList, std::vector<IndexedTriangleList> &TriLists)
{
	size_t PlaneDataListSize = PlaneDataList.size();

	IndexedTriangleList itl;

	std::vector<glm::vec3> VertexList;

	for (unsigned int p = 0; p < PlaneDataListSize; p++) {

		//std::cout << "checking plane " << PlaneDataList[p].planeId << std::endl;

		if (PlaneDataList[p].GeneratePolygon == 1) {

			// get the vertices for the plane
			for (unsigned int p1 = 0; p1 < PlaneDataListSize; p1++) {
				for (unsigned int p2 = 0; p2 < PlaneDataListSize; p2++) {
					if (p != p1 && p1 != p2 && p != p2)
					{
						//std::cout << "Checking planes " << p << " and " << p1 << " and " << p2 << std::endl;
						GetIntersection(PlaneDataList, p, p1, p2, PlaneDataList[p].VerticesForPlane);
					}
				}
			}

			// accumulate vertices and call convex hull only when all plances are complete
			VertexList.insert(VertexList.end(), PlaneDataList[p].VerticesForPlane.begin(), PlaneDataList[p].VerticesForPlane.end());

		}
	}

	if (VertexList.size() > 0) {
		CalcConvexHullAndCreatCallList(VertexList, itl);
	}
	//else {
	//	std::cout << "The plane with id '" << PlaneDataList[p].planeId
	//		<< "' and material '" << PlaneDataList[p].material
	//		<< "' has no valid vertices." << std::endl;
	//}

	if (itl.GetFloatCount() > 0)
	{
		itl.CalculateVertexNormals();
		GLuint buffers[2];
		glGenBuffers(2, buffers);
		itl.SetGLBufferIds(buffers[0], buffers[1]);
		itl.BindArrays();
		TriLists.push_back(itl);
		return true;
	}
	else {
		return false;
	}
}

void ProcessSide(std::vector<SIDE>::iterator sideIter, std::vector<PLANE_VERTICES> &PlaneDataList)
{
	std::string skip;
	std::string p1;
	std::string p2;
	std::string p3;
	float d1, d2, d3, d4, d5, d6, d7, d8, d9;

	std::stringstream planeStrm(sideIter->plane);
	std::getline(std::getline(planeStrm, skip, '('), p1, ')');
	std::getline(std::getline(planeStrm, skip, '('), p2, ')');
	std::getline(std::getline(planeStrm, skip, '('), p3, ')');

	// NOTE: This code flips the y and z
	std::stringstream ps1(p1);
	ps1 >> d1;
	ps1 >> d3;
	ps1 >> d2;

	std::stringstream ps2(p2);
	ps2 >> d4;
	ps2 >> d6;
	ps2 >> d5;

	std::stringstream ps3(p3);
	ps3 >> d7;
	ps3 >> d9;
	ps3 >> d8;

	PLANE_VERTICES pv;
	pv.v1 = glm::vec3(d1, d2, d3);
	pv.v2 = glm::vec3(d4, d5, d6);
	pv.v3 = glm::vec3(d7, d8, d9);
	pv.planeNormal = glm::normalize(glm::cross(pv.v1 - pv.v2, pv.v3 - pv.v2));
	pv.d = -1.0f * glm::dot(pv.v1, pv.planeNormal);
	pv.planeId = sideIter->id;
	pv.GeneratePolygon = 1;
	pv.material = sideIter->material;
	if (sideIter->material.compare("TOOLS/TOOLSHINT") == 0) pv.GeneratePolygon = 0;
	if (sideIter->material.compare("TOOLS/TOOLSSKIP") == 0) pv.GeneratePolygon = 0;
	if (sideIter->material.compare("TOOLS/TOOLSSKYBOX") == 0) pv.GeneratePolygon = 0;
	if (sideIter->material.compare("TOOLS/TOOLSNODRAW") == 0) pv.GeneratePolygon = 0;
	if (sideIter->material.compare("TOOLS/TOOLSPLAYERCLIP") == 0) pv.GeneratePolygon = 0;

	// calculate u and v here
	// [nx ny nz tr] sc
	std::stringstream uaxisStream(sideIter->uaxis);
	std::stringstream vaxisStream(sideIter->vaxis);
	std::string up1;
	std::string up2;
	std::string vp1;
	std::string vp2;
	std::getline(std::getline(uaxisStream, skip, '['), up1, ']');
	up2 = sideIter->uaxis.substr(sideIter->uaxis.find_first_of(']') + 1);
	std::getline(std::getline(vaxisStream, skip, '['), vp1, ']');
	vp2 = sideIter->vaxis.substr(sideIter->vaxis.find_first_of(']') + 1);

	float u1, u2, u3, u4, u5;
	float v1, v2, v3, v4, v5;
	std::stringstream tss(up1);
	tss >> u1;
	tss >> u2;
	tss >> u3;
	tss >> u4;
	tss.str(""); tss.clear(); tss.str(up2);
	tss >> u5;
	tss.str(""); tss.clear(); tss.str(vp1);
	tss >> v1;
	tss >> v2;
	tss >> v3;
	tss >> v4;
	tss.str(""); tss.clear(); tss.str(vp2);
	tss >> v5;

	pv.uNormal = glm::vec3(u1, u3, u2);
	pv.uTrans = u4;
	pv.uScale = u5;
	pv.vNormal = glm::vec3(v1, v3, v2);
	pv.vTrans = v4;
	pv.vScale = v5;

	PlaneDataList.push_back(pv);

	//std::cout << "=====> plane id = " << sideIter->id << std::endl;
	//std::cout << "v1 = " << pv.v1 << std::endl;
	//std::cout << "v2 = " << pv.v2 << std::endl;
	//std::cout << "v3 = " << pv.v3 << std::endl;
	//std::cout << "normal = " << pv.planeNormal << std::endl;
	//std::cout << "d = " << pv.d << std::endl;
}

void CreateIndexedTriangleListsFromVMF(VMF_FILE_DATA &d, std::vector<IndexedTriangleList> &TriList)
{

	//std::cout << "Processing VMF" << std::endl;

	std::vector<SOLID>::iterator solidIter = d.world.solids.begin();
	for (; solidIter != d.world.solids.end(); ++solidIter)
	{
		//if (solidIter->id.compare("194259") == 0) std::cout << "!!got it!!" << std::endl;
		//std::cout << "checking solid " << solidIter->id << std::endl;
		std::vector<PLANE_VERTICES> PlaneDataList;
		std::vector<SIDE>::iterator sideIter = solidIter->sides.begin();
		for (; sideIter != solidIter->sides.end(); ++sideIter)
		{
			//std::cout << "checking side " << sideIter->id << std::endl;
			ProcessSide(sideIter, PlaneDataList);
		}
		if (false == GetPolygonFromPlanes(PlaneDataList, TriList))
		{
			std::cout << "solid " << solidIter->id << " didn't generate any shapes" << std::endl;
		}
	}

	std::vector<ENTITY>::iterator entityIter = d.entityList.begin();
	for (; entityIter != d.entityList.end(); ++entityIter)
	{
		if (entityIter->solidList.size() > 0) {
			std::vector<SOLID>::iterator solidIter = entityIter->solidList.begin();
			for (; solidIter != entityIter->solidList.end(); ++solidIter)
			{
				std::vector<PLANE_VERTICES> PlaneDataList;
				std::vector<SIDE>::iterator sideIter = solidIter->sides.begin();
				for (; sideIter != solidIter->sides.end(); ++sideIter)
				{
					ProcessSide(sideIter, PlaneDataList);
				}
				if (false == GetPolygonFromPlanes(PlaneDataList, TriList)) {
					std::cout << "solid " << solidIter->id << " didn't generate any shapes" << std::endl;
				}
			}
		}
	}
}