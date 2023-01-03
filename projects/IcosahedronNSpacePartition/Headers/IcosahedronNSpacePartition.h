#pragma once

#include "DataDrivenBaseApplication.h"

namespace Kigs
{

	namespace Scene
	{
		class Scene3D;
	}
	using namespace Kigs::DDriven;
	class ClassificationIcosahedron;

	class IcosahedronNSpacePartition : public DataDrivenBaseApplication
	{
	public:
		DECLARE_CLASS_INFO(IcosahedronNSpacePartition, DataDrivenBaseApplication, Core);
		DECLARE_CONSTRUCTOR(IcosahedronNSpacePartition);

	protected:

		maInt	mSelectedVertice = BASE_ATTRIBUTE(SelectedVertice, -1);
		maInt	mSelectedFace = BASE_ATTRIBUTE(SelectedFace, -1);

		void	drawEdges() const;
		void	drawVertices() const;
		void	drawFaces() const;

		void	ProtectedInit() override;
		void	ProtectedUpdate() override;
		void	ProtectedClose() override;


		void	ProtectedInitSequence(const std::string& sequence) override;
		void	ProtectedCloseSequence(const std::string& sequence) override;


		ClassificationIcosahedron* mIcosahedron = nullptr;

		SmartPointer<Scene::Scene3D>		mScene3D = nullptr;
	};
}
