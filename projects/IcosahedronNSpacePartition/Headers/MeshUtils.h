#pragma once
#include <vector>
#include "TecLibs/Tec3D.h"

class GenericMesh
{
public:
	struct faceStruct
	{
		std::vector<u32>	mEdges;
	};

	struct edgeStruct
	{
		// two vectors
		u32							mV[2];
		// two faces
		u32							mF[2];
	};

	struct verticeStruct
	{
		v3f							mVerticePos;
		// list of edges (bit 31 gives edge direction)
		std::vector<u32>			mEdges;

		// return index in mEdges list of the given edge (from its index in global list)
		u32 getInternIndexForEdge(u32 ei) const
		{
			size_t iei = 0;
			for (auto& e : mEdges)
			{
				if ((e & 0x7FFFFFFF) == ei)
				{
					return iei;
				}
				iei++;
			}
			return (u32) -1;
		}

		// return packed edge index (in global list) of the edge before the given one in mEdges list
		u32	getEdgeBefore(u32 iei) const
		{
			iei += mEdges.size() - 1;
			iei %= mEdges.size();
			return mEdges[iei];
		}

		// return packed edge index (in global list) of the edge after the given one in mEdges list
		u32	getEdgeAfter(u32 iei) const
		{
			iei++;
			iei %= mEdges.size();
			return mEdges[iei];
		}


	};

	void	addVertice(const v3f& v) // don't check if vertice already exist
	{
		verticeStruct toAdd;
		toAdd.mVerticePos = v;
		mVertices.push_back(toAdd);
	}

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
		toAdd.mV[1 - swap] = vend;
		toAdd.mF[0] = toAdd.mF[1] = -1;

		mEdges.push_back(toAdd);

		return mEdges.size() - 1;
	}

	// connect vertice must be done in counterclock order
	void	connectVertices(u32 vstart, u32 vend)
	{
		mVertices[vstart].mEdges.push_back(addEdge(vstart, vend));
	}

	// after edges and vertices where setup
	void setUpFaces();

	// given packed edge info e, return edge index and modify ew
	u32		unpackEdgeInfos(u32 e, u32& ew) const
	{
		ew = e >> 31;
		return e & 0x7FFFFFFF;
	}

	edgeStruct* getNextEdge(edgeStruct* currentE, u32& ei, u32& ew) const;

protected:


	std::vector<verticeStruct>		mVertices;
	std::vector<edgeStruct>			mEdges;
	std::vector<faceStruct>			mFaces;
};
