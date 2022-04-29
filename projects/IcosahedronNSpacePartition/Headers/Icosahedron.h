#pragma once

#include "CoreModifiable.h"
#include "CoreModifiableAttribute.h"
#include "TecLibs/Tec3D.h"
#include "MeshUtils.h"

// manage icosahedron (faces, vertices...)
class Icosahedron : public GenericMesh
{
public:

	Icosahedron();

	std::vector<std::pair<v3f, v3f>>	getEdges() const;
	std::vector<std::pair<v3f, std::vector<v3f>>>	getVertices() const;
	std::vector<std::vector<v3f>>	getFaces() const;

	u32		getNormalFlag(const v3f& tst);

protected:

	// construct icosahedron
	void	construct();

	// barycentric coordinates for each faces
	Matrix3x3	mBarycentricMatrix[20];
};