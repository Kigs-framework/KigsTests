#include "MeshUtils.h"

void GenericMesh::setUpFaces()
{

	// for each vertice, take each edge
	size_t verticeIndex = 0;
	for (const auto& v : mVertices)
	{
		for (auto ein : v.mEdges)
		{
			u32 ew;
			u32 ei = unpackEdgeInfos(ein, ew);
		
			auto* currentE = &mEdges[ei];

			if (currentE->mF[ew] == -1) // edge not already setup ?
			{
				// add new face 
				faceStruct toAdd;
				toAdd.mFlags = 0;
				size_t tindex = mFaces.size();

				// follow edges  
				currentE->mF[ew] = tindex; // set this new face at edge side 
				toAdd.mEdges.push_back(ei | (ew << 31));

				while (currentE->mV[1 - ew] != verticeIndex) // while the face was not closed (edge end vertice == first vertice), go to next edge
				{
					currentE = getNextEdge(currentE,ei,ew);
					currentE->mF[ew] = tindex; // set this new face at edge side 
					toAdd.mEdges.push_back(ei | (ew << 31));
				}

				mFaces.push_back(toAdd);
			}
		}
		verticeIndex++;
	}

}

GenericMesh::edgeStruct*  GenericMesh::getNextEdge(edgeStruct* currentE, u32& ei, u32& ew) const
{
	// get end vertice
	auto& ev = mVertices[currentE->mV[1 - ew]];

	u32 internEdgeIndex = ev.getInternIndexForEdge(ei);
	u32 ein = ev.getEdgeBefore(internEdgeIndex);
	ei = unpackEdgeInfos(ein, ew);

	return (GenericMesh::edgeStruct*) & mEdges[ei];
}


void GenericMesh::getTriangleVertices(u32 faceIndex, v3f* tvertices)
{
	
	const auto& f = mFaces[faceIndex];
	if (f.mEdges.size() != 3)
	{
		// TODO error management here ?
		return;
	}

	size_t i = 0;
	for (auto e : f.mEdges)
	{
		u32 ew;
		u32 ei = unpackEdgeInfos(e, ew);

		const auto& edge = mEdges[ei];

		tvertices[i] = mVertices[edge.mV[ew]].mVerticePos;

		i++;
	}
}