#pragma once

#include <DataDrivenBaseApplication.h>
#include "ModernMesh.h"


class CollisionMeshSimplification;
class MeshSimplifier : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(MeshSimplifier, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(MeshSimplifier);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	CollisionMeshSimplification*	mMeshSimplification;
	std::vector<u32>				mMeshVertexIndices;

	SmartPointer<Scene3D>		mScene3D;
	SmartPointer<Node3D>		mMeshNode;
	SmartPointer<Material>		mCubeMaterial;
	
	Matrix3x4					mRecenterMatrix;

	SmartPointer<ModernMesh>	buildMesh(const std::vector<u32>& indices, const std::vector<v3f>& vertices, const std::string& meshName);

	SmartPointer<ModernMesh>	getCube(u32 flag);

	std::map<u32, SmartPointer<ModernMesh>>	mOctreeCubes;
};
