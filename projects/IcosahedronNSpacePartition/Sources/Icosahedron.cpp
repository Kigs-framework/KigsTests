#include "Icosahedron.h"

Icosahedron::Icosahedron(u32 subdivisionLevel) : mSubdivisionLevel(subdivisionLevel)
{
	constructLevel0();
}

void	Icosahedron::constructLevel0()
{
	float phi = 0.5f * (1.0f + sqrtf(5.0f));

	// vertices
	verticeStruct	toAdd;
	for (size_t i = 0; i < 3; i++) // the 0 coord index
	{
		toAdd.mVerticePos[i] = 0.0f;

		for (size_t j = 0; j < 2; j++) // the -phi / +phi
		{
			toAdd.mVerticePos[(i + 1) % 3] = (j == 0) ? (-phi) : phi;
			for (size_t k = 0; k < 2; k++) // the -phi / +phi
			{
				toAdd.mVerticePos[(i + 2) % 3] = ((k^j) == 0) ? (-1.0f) : 1.0f;
				mVertices.push_back(toAdd);
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

	setUpFaces();
}

void Icosahedron::setUpFaces()
{

	// for each vertice, take each edge
	for (const auto& v : mVertices)
	{
		for (auto ein : v.mEdges)
		{
			u32 ei = ein & 0x7FFFFFFF;
			u32 ew = ein >> 31;
			
			auto& currentE = mEdges[ei];
			
			if (currentE.mT[ew] == -1)
			{
				// add new face 
				triangleStruct toAdd;

				size_t tindex = mTriangles.size();

				// follow 3 edges  

				currentE.mT[ew] = tindex;


				mTriangles.push_back(toAdd);
			}
		}
	}

}

std::vector<std::pair<v3f, v3f>>	Icosahedron::getEdges()
{
	std::vector<std::pair<v3f, v3f>> result;

	for (auto& e : mEdges)
	{
		result.push_back({ mVertices[e.mV[0]].mVerticePos,mVertices[e.mV[1]].mVerticePos });
	}

	return result;
}

std::vector<std::pair<v3f, std::vector<v3f>>>	Icosahedron::getVertices()
{
	std::vector<std::pair<v3f, std::vector<v3f>>> result;

	for (auto& v : mVertices)
	{
		std::pair<v3f, std::vector<v3f>> toAdd;
		toAdd.first = v.mVerticePos;

		for (auto ein : v.mEdges)
		{
			u32 ei = ein & 0x7FFFFFFF;
			u32 ew = ein >> 31;

			toAdd.second.push_back(mVertices[mEdges[ei].mV[1 - ew]].mVerticePos);
		}

		result.push_back(toAdd);
	}

	return result;
}