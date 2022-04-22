#include "Icosahedron.h"

Icosahedron::Icosahedron(u32 subdivisionLevel) : mSubdivisionLevel(subdivisionLevel)
{
	memset(mFaceFlag, 0, 20 * sizeof(u32));
	constructLevel0();
}

u32		Icosahedron::getNormalFlag(const v3f& tst)
{
	u32 flag = 0;
	for (size_t i = 0; i < 3; i++) // the 0 coord index
	{
		if (tst[i] < 0.0f)
		{
			flag |= 1 << i;
		}
	}
	return flag;
}

void	Icosahedron::constructLevel0()
{
	float phi = 0.5f * (1.0f + sqrtf(5.0f));

	// for a unitary sphere icosahedron
	float OneOnNorm = 1.0f/sqrtf(phi * phi + 2.0f);
	phi *= OneOnNorm;

	// vertices
	v3f	toAdd;
	size_t verticeIndex = 0;
	for (size_t i = 0; i < 3; i++) // the 0 coord index
	{
		toAdd[i] = 0.0f;
		for (size_t j = 0; j < 2; j++) // the -phi / +phi
		{
			toAdd[(i + 1) % 3] = (j == 0) ? (-phi) : phi;
			for (size_t k = 0; k < 2; k++) // the -phi / +phi
			{
				toAdd[(i + 2) % 3] = ((k^j) == 0) ? (-OneOnNorm) : OneOnNorm;
				addVertice(toAdd);
				verticeIndex++;
			}
		}
	}

	// edges and triangles
	// for each starting vertice, connect to 5 others (counter clock)
	std::vector<std::vector<u32>>	connections = {
		{4,8,1,11,5}, //0
		{0,8,7,6,11}, //1
		{3,10,6,7,9}, //2
		{5,10,2,9,4}, //3
		{0,5,3,9,8},  //4
		{4,0,11,10,3},//5
		{11,1,7,2,10},//6
		{9,2,6,1,8},  //7
		{0,4,9,7,1},  //8
		{4,3,2,7,8},  //9
		{5,11,6,2,3}, //10
		{5,0,1,6,10}, //11
	};

	for (size_t i = 0; i < connections.size(); i++)
	{
		for (auto to : connections[i])
		{
			connectVertices(i, to);
		}
	}

	// flag edges according to octan cell

	// 2 bit flag per coord
	//  <0   0   >0  
	//  10   0   01
	u32	edgeFlags[30];
	memset(edgeFlags, 0, 30 * sizeof(u32));

	size_t edgeIndex = 0;
	for (auto& e : mEdges)
	{
		// take edge center
		v3f center(mVertices[e.mV[0]].mVerticePos+ mVertices[e.mV[1]].mVerticePos);
		
		for (size_t i = 0; i < 3; i++)
		{
			if (center[i] < 0.0f)
			{
				edgeFlags[edgeIndex] |= 2 << (i << 1);
			}
			else if (center[i] > 0.0f)
			{
				edgeFlags[edgeIndex] |= 1 << (i << 1);
			}
		}
		edgeIndex++;
	}

	setUpFaces();

	// flag faces using "or" of each vertice flag
	size_t faceindex = 0;
	for (auto& f : mFaces)
	{
		for (auto e : f.mEdges)
		{
			u32 ew;
			u32 ei = unpackEdgeInfos(e, ew);

			mFaceFlag[faceindex] |= edgeFlags[ei];
		}
		faceindex++;
	}
}

std::vector<std::vector<v3f>>	Icosahedron::getFaces() const
{
	std::vector<std::vector<v3f>> result;

	for (auto& f : mFaces)
	{
		std::vector<v3f> vlist;
		for (auto e : f.mEdges)
		{
			u32 ew;
			u32 ei = unpackEdgeInfos(e, ew);

			vlist.push_back(mVertices[mEdges[ei].mV[ew]].mVerticePos);
		}
		result.push_back(vlist);
	}

	return result;
}


std::vector<std::pair<v3f, v3f>>	Icosahedron::getEdges() const
{
	std::vector<std::pair<v3f, v3f>> result;

	for (auto& e : mEdges)
	{
		result.push_back({ mVertices[e.mV[0]].mVerticePos,mVertices[e.mV[1]].mVerticePos });
	}

	return result;
}

std::vector<std::pair<v3f, std::vector<v3f>>>	Icosahedron::getVertices() const
{
	std::vector<std::pair<v3f, std::vector<v3f>>> result;

	for (auto& v : mVertices)
	{
		std::pair<v3f, std::vector<v3f>> toAdd;
		toAdd.first = v.mVerticePos;

		for (auto ein : v.mEdges)
		{
			u32 ew;
			u32 ei = unpackEdgeInfos(ein, ew);

			toAdd.second.push_back(mVertices[mEdges[ei].mV[1 - ew]].mVerticePos);
		}

		result.push_back(toAdd);
	}

	return result;
}