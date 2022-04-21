#pragma once

#include "CoreModifiable.h"
#include "CoreModifiableAttribute.h"
#include "TecLibs/Tec3D.h"

// manage icosahedron (faces, vertices...)
class Icosahedron
{
public:

	Icosahedron(u32 subdivisionLevel = 0);

	std::vector<std::pair<v3f, v3f>>	getEdges();

	std::vector<std::pair<v3f, std::vector<v3f>>>	getVertices();

protected:

	void	constructLevel0();

	u32	addEdge(u32 vstart, u32 vend)
	{
		// edges are stored with start indice < end indice

		u32	swap = 0;
		if (vstart > vend)
			swap = 1;

		size_t foundindex = 0;
		for (const auto& e : mEdges)
		{
			if ((e.mV[swap] == vstart) && (e.mV[1 - swap] == vend))
			{
				return (foundindex | (swap << 31));
			}
			foundindex++;
		}

		edgeStruct toAdd;
		toAdd.mV[swap] = vstart;
		toAdd.mV[1-swap] = vend;
		toAdd.mT[0] = toAdd.mT[1] = -1;

		mEdges.push_back(toAdd);

		return mEdges.size() - 1;
	}

	void	connectVertices(u32 vstart, u32 vend)
	{
		mVertices[vstart].mEdges.push_back(addEdge(vstart, vend));
	}

	void setUpFaces();

	struct triangleStruct
	{
		// 3 edges (bit 31 gives edge direction)
		u32	mE[3];
	};

	struct edgeStruct
	{
		// two vectors
		u32							mV[2];
		// two triangles
		u32							mT[2];
	};

	struct verticeStruct
	{
		v3f							mVerticePos;
		// list of edges (bit 31 gives edge direction)
		std::vector<u32>			mEdges;
	};


	std::vector<verticeStruct>		mVertices;
	std::vector<edgeStruct>			mEdges;
	std::vector<triangleStruct>		mTriangles;

	u32	mSubdivisionLevel;

};