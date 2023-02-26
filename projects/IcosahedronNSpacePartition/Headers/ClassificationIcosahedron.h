#pragma once
#include "Icosahedron.h"

namespace Kigs
{

	class ClassificationIcosahedron : public Icosahedron
	{
	public:

		ClassificationIcosahedron();

		// return face index and barycentric coords (not normalised)
		u32		getNormalFaceIndex(const v4f& tst, v4f& barycentricCoords);
		// return only face index
		u32		getNormalFaceIndex(const v4f& tst);

		inline u32		signBitToOctan(const v4f& tst)
		{
			// combine sign bit of x,y,z coordinates to get octan
			u32 result = (((*(u32*)&tst.x) >> 31) & 1) | (((*(u32*)&tst.y) >> 30) & 2) | (((*(u32*)&tst.z) >> 29) & 4);
			return result;
		}

		v3f	getNormal(u32 faceindex, v4f barycoords);

		void	populate(std::vector<v3f> vertices, std::vector<u32> indices);

	protected:
		// barycentric coordinates matrix for each faces (TODO: use SIMD)
		Maths::Matrix3x3		mBarycentricMatrix[20];

		// input / output data

		struct nStruct
		{
			v4f		mNormal;		//	not normalized triangle normal
			v4f		mBaryCoords;	//  normal bary coordinates in icosahedron face 
			u32			mTIndices;		//	original triangle indice
			u32			mFlag;			//  flag giving icosahedron face index
		};

		std::vector<nStruct>	mNormals;
	};
}