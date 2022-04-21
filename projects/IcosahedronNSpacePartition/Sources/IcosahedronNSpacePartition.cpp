#include <IcosahedronNSpacePartition.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>
#include "Scene3D.h"
#include "Icosahedron.h"
#include "GLSLDebugDraw.h"

IMPLEMENT_CLASS_INFO(IcosahedronNSpacePartition);

IMPLEMENT_CONSTRUCTOR(IcosahedronNSpacePartition)
{

}

void	IcosahedronNSpacePartition::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");


	mIcosahedron = new Icosahedron(0);


	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}


void	IcosahedronNSpacePartition::drawEdges()
{
#ifdef _DEBUG
	auto edgeList = mIcosahedron->getEdges();
	v3f color(0.0f, 1.0f, 0.0f);
	for (size_t i = 0; i < edgeList.size(); i++)
	{
		auto& e = edgeList[i];
		dd::line(e.first, e.second, color);
	}
	
#endif
}

void	IcosahedronNSpacePartition::drawVertices()
{
#ifdef _DEBUG
	auto edgeList = mIcosahedron->getVertices();

	v3f color(1.0, 0.0, 0.0);
	for (size_t i = 0; i < edgeList.size(); i++)
	{
		if (mSelectedVertice == i)
		{
			auto& e = edgeList[i];

			for (auto& v : e.second)
			{
				dd::line(e.first, v, color,0,false);
			}
		}
	}

#endif
}

void	IcosahedronNSpacePartition::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();

	if (mScene3D)
	{
		drawEdges();
		drawVertices();
	}
}

void	IcosahedronNSpacePartition::ProtectedClose()
{

	delete mIcosahedron;

	DataDrivenBaseApplication::ProtectedClose();
}

void	IcosahedronNSpacePartition::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mScene3D = GetFirstInstanceByName("Scene3D", "myScene");

		CMSP cam = mScene3D->GetFirstSonByName("Camera", "Camera", true);
		cam->Upgrade("OrbitCameraUp");
	}
}
void	IcosahedronNSpacePartition::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		
	}
}

