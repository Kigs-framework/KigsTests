#pragma once

#include "CoreModifiable.h"
#include "CoreModifiableAttribute.h"
#include "TecLibs/Tec3D.h"
#include "MeshUtils.h"

// manage icosahedron (faces, vertices...)
class Icosahedron : public GenericMesh
{
public:

	Icosahedron(u32 subdivisionLevel = 0);

	std::vector<std::pair<v3f, v3f>>	getEdges() const;
	std::vector<std::pair<v3f, std::vector<v3f>>>	getVertices() const;
	std::vector<std::vector<v3f>>	getFaces() const;

	u32		getNormalFlag(const v3f& tst);

protected:

	void	constructLevel0();


	u32	mSubdivisionLevel;

	u32	mFaceFlag[20];

};