
typedef struct STRUCT_VERSION_INFO {
	std::string editorversion;
	std::string editorbuild;
	std::string mapversion;
	std::string formatversion;
	std::string prefab;
} VERSION_INFO;

typedef struct STRUCT_VIEW_SETTINGS
{
	std::string bSnapToGrid;
	std::string bShowGrid;
	std::string bShowLogicalGrid;
	std::string nGridSpacing;
	std::string bShow3DGrid;
} VIEW_SETTINGS;

typedef struct STRUCT_SIDE
{
	std::string id;
	std::string plane;
	std::string material;
	std::string uaxis;
	std::string vaxis;
	std::string rotation;
	std::string lightmapscale;
	std::string smoothing_groups;
} SIDE;

typedef struct STRUCT_EDITOR
{
	std::string color;
	std::string groupid;
	std::string visgroupshown;
	std::string visgroupautoshown;
} EDITOR;

typedef struct STRUCT_SOLID
{
	std::string id;
	std::vector<SIDE> sides;
	EDITOR editor;
} SOLID;

typedef struct STRUCT_GROUP
{
	std::string id;
	EDITOR editor;
} GROUP;

typedef struct STRUCT_WORLD
{
	std::string id;
	std::string mapversion;
	std::string classname;
	std::string detailmaterial;
	std::string detailvbsp;
	std::string maxpropscreenwidth;
	std::string skyname;
	std::vector<SOLID> solids;
	std::vector<GROUP> groups;
} WORLD;

typedef struct STRUCT_VISGROUP
{
	std::string name;
	std::string visgroupid;
	std::string color;
} VISGROUP;

typedef struct STRUCT_VISGROUPS
{
	std::vector<VISGROUP> visgroups;
} VISGROUPS;

typedef struct STRUCT_ENTITY
{
	std::string id;
	std::string classname;
	std::vector<SOLID> solidList;
	EDITOR editor;
	std::string origin;
} ENTITY;

typedef struct STRUCT_VMF_FILE_DATA {
	VERSION_INFO versionInfo;
	VIEW_SETTINGS viewSettings;
	WORLD world;
	std::vector<VISGROUP> visgroups;
	std::vector<ENTITY> entityList;
} VMF_FILE_DATA;

typedef struct STRUCT_PLANE {
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 p3;
	glm::vec3 nv;
	float dval;
} PLANE;

typedef struct S_PLANE_VERTICES
{
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 v3;
	glm::vec3 planeNormal;
	std::vector<glm::vec3> VerticesForPlane;
	std::string planeId;
	float d;
	int GeneratePolygon;
	std::string material;
	glm::vec3 uNormal;
	glm::vec3 vNormal;
	float uTrans;
	float vTrans;
	float uScale;
	float vScale;
} PLANE_VERTICES;

typedef struct STRUCT_TRIANGLE
{
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
} TRIANGLE;

void ParseVMFFile(const char* VmfFileName, VMF_FILE_DATA &d);
void CreateIndexedTriangleListsFromVMF(VMF_FILE_DATA &d, std::vector<IndexedTriangleList> &TriList);