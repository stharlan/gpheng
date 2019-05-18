// bspreadr.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

using namespace std;

#define HEADER_LUMPS 64

struct lump_t
{
	int	fileofs;	// offset into file (bytes)
	int	filelen;	// length of lump (bytes)
	int	version;	// lump format version
	char	fourCC[4];	// lump ident code
};

struct dheader_t
{
	int	ident;                // BSP file identifier
	int	version;              // BSP file version
	lump_t	lumps[HEADER_LUMPS];  // lump directory array
	int	mapRevision;          // the map's revision (iteration, version) number
};

struct vertex_t
{
	float x;
	float y;
	float z;
};

struct dedge_t
{
	unsigned short	v[2];	// vertex indices
};

struct dface_t
{
	unsigned short	planenum;		// the plane number
	unsigned char		side;			// faces opposite to the node's plane direction
	unsigned char		onNode;			// 1 of on node, 0 if in leaf
	int		firstedge;		// index into surfedges
	short		numedges;		// number of surfedges
	short		texinfo;		// texture info
	short		dispinfo;		// displacement info
	short		surfaceFogVolumeID;	// ?
	unsigned char		styles[4];		// switchable lighting info
	int		lightofs;		// offset into lightmap lump
	float		area;			// face area in units^2
	int		LightmapTextureMinsInLuxels[2];	// texture lighting info
	int		LightmapTextureSizeInLuxels[2];	// texture lighting info
	int		origFace;		// original face this was split from
	unsigned short	numPrims;		// primitives
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;	// lightmap smoothing group
};

int main()
{
	FILE* f = nullptr;
	fopen_s(&f, "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\sdk_content\\maps\\de_subway.bsp", "rb");

	dheader_t hdr;
	fread(&hdr, sizeof(dheader_t), 1, f);
	cout << hex << "0x" << hdr.ident << dec << endl;
	cout << "version = " << hdr.version << endl;
	cout << "map revision = " << hdr.mapRevision << endl;

	//for (int i = 0; i < HEADER_LUMPS; i++) {
	//	cout << "===========================" << endl;
	//	cout << "version " << hdr.lumps[i].version << endl;
	//	cout << "offset " << hex << "0x" << hdr.lumps[i].fileofs << dec << endl;
	//	cout << "length " << hex << "0x" << hdr.lumps[i].filelen << dec << endl;
	//	cout << hdr.lumps[i].fourCC[0] <<
	//		hdr.lumps[i].fourCC[1] <<
	//		hdr.lumps[i].fourCC[2] <<
	//		hdr.lumps[i].fourCC[3] << endl;
	//}

	// 3 is vertexes
	cout << "vertex offset = " << hdr.lumps[3].fileofs << endl;
	cout << "vertex length = " << hdr.lumps[3].filelen << endl;
	cout << "num vertices = " << hdr.lumps[3].filelen / 12 << endl;
	vertex_t* vertices = (vertex_t*)malloc(hdr.lumps[3].filelen);
	fseek(f, hdr.lumps[3].fileofs, SEEK_SET);
	fread(vertices, hdr.lumps[3].filelen, 1, f);

	// edge
	cout << "edge offset = " << hdr.lumps[12].fileofs << endl;
	cout << "edge len = " << hdr.lumps[12].filelen << endl;

	dedge_t* edge = (dedge_t*)malloc(hdr.lumps[12].filelen);
	fseek(f, hdr.lumps[12].fileofs, SEEK_SET);
	fread(edge, hdr.lumps[12].filelen, 1, f);

	// surfedge
	cout << "surfedge offset = " << hdr.lumps[13].fileofs << endl;
	cout << "surfedge len = " << hdr.lumps[13].filelen << endl;

	int* surfedge = (int*)malloc(hdr.lumps[13].filelen);
	fseek(f, hdr.lumps[13].fileofs, SEEK_SET);
	fread(surfedge, hdr.lumps[13].filelen, 1, f);

	// orig face
	cout << "orig face offset = " << hdr.lumps[27].fileofs << endl;
	cout << "orig face len = " << hdr.lumps[27].filelen << endl;

	dface_t *origFaces = (dface_t*)malloc(hdr.lumps[27].filelen);
	fseek(f, hdr.lumps[27].fileofs, SEEK_SET);
	fread(origFaces, hdr.lumps[27].filelen, 1, f);
	unsigned int numOrigFaces = hdr.lumps[27].filelen / sizeof(dface_t);
	cout << "num orig faces = " << numOrigFaces << endl;
	for (unsigned int of = 0; of < numOrigFaces; of++) {
		cout << "se index = " << origFaces[of].firstedge << "; num = " << origFaces[of].numedges << endl;
		for (unsigned int se = 0; se < origFaces[of].numedges; se++) {
			cout << "\t" << surfedge[origFaces[of].firstedge + se] << " :: ";
			int abse = abs(surfedge[origFaces[of].firstedge + se]);
			cout << edge[abse].v[0] << " " << edge[abse].v[1] << " :: from [";
			cout << vertices[edge[abse].v[0]].x << ", " << vertices[edge[abse].v[0]].y << ", " << vertices[edge[abse].v[0]].z << "] to [" <<
				vertices[edge[abse].v[1]].x << ", " << vertices[edge[abse].v[1]].y << ", " << vertices[edge[abse].v[1]].z << "]" << endl;
		}
	}

	fclose(f);
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
