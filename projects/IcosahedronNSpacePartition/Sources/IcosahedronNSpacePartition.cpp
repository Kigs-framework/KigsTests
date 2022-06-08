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


	mIcosahedron = new Icosahedron();

	/*for (size_t i = 0; i < 100; i++)
	{
		v3f tstN((rand() & 255)-127, (rand() & 255) - 127, (rand() & 255) - 127);
		tstN.Normalize();
		mIcosahedron->getNormalFlag(tstN);
	}*/

	/*for (size_t i = 0; i < 8; i++)
	{
		v3f tstN((i&1)?-1.0f:1.0f, (i & 2) ? -1.0f: 1.0f, (i & 4) ? -1.0f: 1.0f);
		tstN.Normalize();
		mIcosahedron->getNormalFlag(tstN);
	}*/

	v3f tstN(1.0f, 1.0f, 0.2f);
	tstN.Normalize();
	mIcosahedron->getNormalFlag(tstN);


	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}


void	IcosahedronNSpacePartition::drawEdges() const
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

void	IcosahedronNSpacePartition::drawFaces() const
{
#ifdef _DEBUG
	auto faceList = mIcosahedron->getFaces();
	for (size_t i = 0; i < faceList.size(); i++)
	{
		if (mSelectedFace == i)
		{
			auto& vlist = faceList[i];

			float ecolor = 0.0f;
			for (size_t j = 0; j < vlist.size(); j++)
			{
				float R(0.0f), G(0.0f), B(0.0f);

				float currentcolor = ecolor;
				if (currentcolor > 2.0f)
				{
					B = currentcolor - 2.0f;
					currentcolor -= B;
				}

				if (currentcolor > 1.0f)
				{
					G = currentcolor - 1.0f;
					currentcolor -= G;
				}

				R = currentcolor;

				v3f color(R, G, B);
				dd::line(vlist[j], vlist[(j+1)%vlist.size()], color, 0, false);
				ecolor += 0.5f;
			}
		}
	}

#endif
}

void	IcosahedronNSpacePartition::drawVertices() const
{
#ifdef _DEBUG
	auto edgeList = mIcosahedron->getVertices();

	
	for (size_t i = 0; i < edgeList.size(); i++)
	{
		if (mSelectedVertice == i)
		{
			auto& e = edgeList[i];

			float ecolor = 0.0f;
			for (auto& v : e.second)
			{
				float R(0.0f), G(0.0f), B(0.0f);

				float currentcolor = ecolor;
				if (currentcolor > 2.0f)
				{
					B = currentcolor - 2.0f;
					currentcolor -= B;
				}

				if (currentcolor > 1.0f)
				{
					G = currentcolor - 1.0f;
					currentcolor -= G;
				}
				
				R = currentcolor;

				v3f color(R, G, B);
				dd::line(e.first, v, color,0,false);
				ecolor+=0.5f;
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
		drawFaces();
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

