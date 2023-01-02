#include "ClassificationIcosahedron.h"

using namespace Kigs;

ClassificationIcosahedron::ClassificationIcosahedron() : Icosahedron()
{
	// compute barycentric coordinate conversion matrix for each face (using {0,0,0} as last tetraedron point)
	for (size_t faceIndex = 0; faceIndex < mFaces.size(); faceIndex++)
	{
		SIMDv4f p[3];
		getTriangleVertices(faceIndex, p);

		auto& m = mBarycentricMatrix[faceIndex];

		computeTriangleBarycentricCoordinatesMatrix(m, p);
	}
}


u32		ClassificationIcosahedron::getNormalFaceIndex(const SIMDv4f& tst)
{
	u32 face = signBitToOctan(tst);

	auto& f = mFaces[face];
	auto& m = mBarycentricMatrix[face];

	SIMDv4f barycentriccoords(tst);
	m.TransformVector((Vector3D*)&barycentriccoords.xyz);

	u32 signflag = signBitToOctan(barycentriccoords);

	if (signflag)
	{
		const u32 oppositeEdge[8] = { 0, 1, 2, 0, 0, 0, 0, 0 };
		u32 e = mFaces[face].mEdges[oppositeEdge[signflag]]; // take opposite edge
		u32 ew;
		u32 ei = unpackEdgeInfos(e, ew);
		face = mEdges[ei].mF[1 - ew];
	}

	return face;
}

u32		ClassificationIcosahedron::getNormalFaceIndex(const SIMDv4f& tst, SIMDv4f& barycentricCoords)
{
	u32 face = signBitToOctan(tst);

	auto& f = mFaces[face];
	auto& m = mBarycentricMatrix[face];

	SIMDv4f barycentriccoords(tst);
	m.TransformVector((Vector3D*)&barycentriccoords.xyz);

	u32 signflag = signBitToOctan(barycentriccoords);

	if (signflag)
	{
		const u32 oppositeEdge[8] = { 0, 1, 2, 0, 0, 0, 0, 0 };
		u32 e = mFaces[face].mEdges[oppositeEdge[signflag]]; // take opposite edge
		u32 ew;
		u32 ei = unpackEdgeInfos(e, ew);

		face = mEdges[ei].mF[1 - ew];

		barycentriccoords = tst;
		mBarycentricMatrix[face].TransformVector((Vector3D*)&barycentriccoords);

	}

	barycentricCoords = barycentriccoords;

	return face;
}

void	ClassificationIcosahedron::populate(std::vector<v3f> vertices, std::vector<u32> indices)
{

	u32 tcount = indices.size() / 3;
	
	mNormals.resize(tcount);

	size_t nindex=0;
	for (size_t t = 0; t < indices.size(); t+=3)
	{
		v3f& p1 = vertices[indices[t]];
		v3f& p2 = vertices[indices[t+1]];
		v3f& p3 = vertices[indices[t+2]];

		SIMDv4f	v1(p2 - p1);
		SIMDv4f	v2(p3 - p1);

		mNormals[nindex].mNormal.CrossProduct(v1, v2);
		mNormals[nindex].mFlag =getNormalFaceIndex(mNormals[nindex].mNormal, mNormals[nindex].mBaryCoords);

		mNormals[nindex].mBaryCoords.NormalizeBarycentricCoords();

		nindex++;
	}


}

// test func
v3f	ClassificationIcosahedron::getNormal(u32 faceindex, SIMDv4f barycoords)
{
	auto& f = mFaces[faceindex];
	SIMDv4f n(0.0f,0.0f,0.0f);

	u32 index=0;
	for (auto ein : f.mEdges)
	{
		u32 ew;
		u32 ei = unpackEdgeInfos(ein, ew);

		n += mVertices[mEdges[ei].mV[ew]].mVerticePos * barycoords[index++];
	}

	return n.xyz;
}