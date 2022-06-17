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

	std::vector<std::pair<SIMDv4f, SIMDv4f>>				getEdges() const;
	std::vector<std::pair<SIMDv4f, std::vector<SIMDv4f>>>	getVertices() const;
	std::vector<std::vector<SIMDv4f>>						getFaces() const;

protected:

	// construct icosahedron
	void	construct();


};