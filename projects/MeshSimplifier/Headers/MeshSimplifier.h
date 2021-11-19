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

	void NotifyUpdate(const unsigned int /* labelid */) override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	CollisionMeshSimplification*	mMeshSimplification;
	std::vector<u32>				mMeshVertexIndices;

	SmartPointer<Scene3D>		mScene3D;
	SmartPointer<Node3D>		mMeshNode;
	SmartPointer<Node3D>		mDebugCubeNode;
	SmartPointer<Material>		mCubeMaterial;
	
	v3f							mRecenterTranslate;

	SmartPointer<ModernMesh>	buildMesh(const std::vector<u32>& indices, const std::vector<v3f>& vertices, const std::string& meshName);

	SmartPointer<ModernMesh>	getCube(u32 flag);

	std::map<u32, SmartPointer<ModernMesh>>	mOctreeCubes;

	maInt						mDisplayCubeCount = BASE_ATTRIBUTE(DisplayCubeCount, 100000);
	maVect3DF					mDebugCubePos = BASE_ATTRIBUTE(DebugCubePos, 1,1,1);
	maBool						mShowObject = BASE_ATTRIBUTE(ShowObject, true);
	maBool						mShowEnveloppe = BASE_ATTRIBUTE(ShowEnveloppe, true);
	maBool						mShowEdges = BASE_ATTRIBUTE(ShowEdges, true);

	void						showEnveloppe(bool show);
	void						moveDebugCube();

	void						drawEdges();
	void						drawEnveloppeVertices();
};
