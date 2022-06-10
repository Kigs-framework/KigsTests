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

	std::vector<std::pair<SIMDv4f, SIMDv4f>>	getEdges() const;
	std::vector<std::pair<SIMDv4f, std::vector<SIMDv4f>>>	getVertices() const;
	std::vector<std::vector<SIMDv4f>>	getFaces() const;

	// return face index and barycentric coords (not normalised)
	u32		getNormalFaceIndex(const SIMDv4f& tst, SIMDv4f& barycentricCoords);
	// return only face index
	u32		getNormalFaceIndex(const SIMDv4f& tst);

	inline u32		signBitToOctan(const SIMDv4f& tst)
	{
		u32 result = (((*(u32*)&tst.x) >> 31) & 1) | (((*(u32*)&tst.y) >> 30) & 2) | (((*(u32*)&tst.z) >> 29) & 4);
		return result;
	}

protected:

	// construct icosahedron
	void	construct();

	// barycentric coordinates for each faces
	Matrix3x3		mBarycentricMatrix[20];
	float32x4x4_t	mBarycentrixMatrixSIMD[20];
};