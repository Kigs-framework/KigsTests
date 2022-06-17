#pragma once

#include <DataDrivenBaseApplication.h>


class ClassificationIcosahedron;
class Scene3D;

class IcosahedronNSpacePartition : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(IcosahedronNSpacePartition, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(IcosahedronNSpacePartition);

protected:

	maInt	mSelectedVertice = BASE_ATTRIBUTE(SelectedVertice,-1);
	maInt	mSelectedFace = BASE_ATTRIBUTE(SelectedFace, -1);

	void	drawEdges() const;
	void	drawVertices() const;
	void	drawFaces() const;

	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;


	ClassificationIcosahedron* mIcosahedron=nullptr;

	SmartPointer<Scene3D>		mScene3D=nullptr;
};
