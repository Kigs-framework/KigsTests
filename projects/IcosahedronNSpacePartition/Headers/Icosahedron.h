#pragma once

#include "CoreModifiable.h"
#include "CoreModifiableAttribute.h"
#include "TecLibs/Tec3D.h"
#include "MeshUtils.h"

namespace Kigs
{

	// manage icosahedron (faces, vertices...)
	class Icosahedron : public GenericMesh
	{
	public:

		Icosahedron();

		std::vector<std::pair<v4f, v4f>>				getEdges() const;
		std::vector<std::pair<v4f, std::vector<v4f>>>	getVertices() const;
		std::vector<std::vector<v4f>>						getFaces() const;

	protected:

		// construct icosahedron
		void	construct();


	};
}