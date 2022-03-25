#pragma once

#include <DataDrivenBaseApplication.h>
#include "ModernMesh.h"


class MeshSimplification;
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

	void	rebuildMesh();

	MeshSimplification*	mMeshSimplification=nullptr;
	std::vector<u32>				mMeshVertexIndices;
	std::vector<v3f>				mMeshVertices;

	SmartPointer<Scene3D>		mScene3D;
	SmartPointer<Node3D>		mMeshNode;
	SP<Node3D>					mRootEnvNode;
	SmartPointer<Node3D>		mDebugCubeNode;
	SmartPointer<Material>		mCubeMaterial;
	
	v3f							mRecenterTranslate;

	SmartPointer<ModernMesh>	buildMesh(const std::vector<u32>& indices, const std::vector<v3f>& vertices, const std::string& meshName);

	SmartPointer<ModernMesh>	getCube(u32 flag,u32 debugflag);

	std::map<u32, SmartPointer<ModernMesh>>	mOctreeCubes;

	maInt						mDisplayCubeCount = BASE_ATTRIBUTE(DisplayCubeCount, 100000);
	maInt						mDisplayVerticeCount = BASE_ATTRIBUTE(DisplayVerticeCount, 100000);
	maInt						mDisplayEdgeCount = BASE_ATTRIBUTE(DisplayEdgeCount, 100000);
	maVect3DF					mDebugCubePos = BASE_ATTRIBUTE(DebugCubePos, 1,1,1);
	maInt						mSelectedVerticeIndex = BASE_ATTRIBUTE(SelectedVerticeIndex, 0);
	maInt						mSelectedEdgeIndex = BASE_ATTRIBUTE(SelectedEdgeIndex, 0);
	maBool						mShowObject = BASE_ATTRIBUTE(ShowObject, true);
	maBool						mShowEnveloppe = BASE_ATTRIBUTE(ShowEnveloppe, true);
	maBool						mShowEdges = BASE_ATTRIBUTE(ShowEdges, false);
	maBool						mShowVertices = BASE_ATTRIBUTE(ShowVertices, false);
	maFloat						mPrecision = BASE_ATTRIBUTE(Precision, 0.02f);
	void						showEnveloppe(bool show);
	void						moveDebugCube();

	void						drawEdges();
	void						drawEnveloppeVertices();

	bool						mConstructMesh = true;
	bool						mConstructEnveloppe = true;

	void						addBboxToMesh(const BBox& boxtoadd);
	void						createTestMesh();
};
