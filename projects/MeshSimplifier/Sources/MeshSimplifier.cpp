#include <MeshSimplifier.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>
#include "CollisionMeshSimplification.h"
#include "Scene3D.h"
#include "Material.h"
#include "GLSLDebugDraw.h"

bool	importRaw3DFile(const char* fname, std::vector<u32>& indices, std::vector<v3f>& vertices)
{
	auto handle = Platform_fopen(fname, "rb");

	if (handle->mStatus& FileHandle::Exist)
	{
		u32 icount;
		u32 vcount;

		Platform_fread(&icount, sizeof(u32), 1, handle.get());
		indices.resize(icount);
		Platform_fread(&vcount, sizeof(u32), 1, handle.get());
		vertices.resize(vcount);

		Platform_fread(indices.data(), sizeof(u32), icount, handle.get());
		Platform_fread(vertices.data(), sizeof(v3f), vcount, handle.get());


		Platform_fclose(handle.get()); 
		return true;
	}
	return false;
}


IMPLEMENT_CLASS_INFO(MeshSimplifier);

IMPLEMENT_CONSTRUCTOR(MeshSimplifier)
{

}

// build a cube with colored faces 
SmartPointer<ModernMesh>	MeshSimplifier::getCube(u32 flag)
{
	// cube already in map
	if (mOctreeCubes.find(flag) != mOctreeCubes.end())
	{
		return mOctreeCubes[flag];
	}

	SmartPointer<ModernMesh> Mesh = KigsCore::GetInstanceOf("cube_"+std::to_string(flag), "ModernMesh");

	Mesh->StartMeshBuilder();
	auto description = MakeCoreVector();

	auto vertices_desc = MakeCoreNamedVector("vertices");
	description->set("", vertices_desc);

	auto ndesc = MakeCoreNamedVector("generate_normals");
	description->set("", ndesc);

	auto cdesc = MakeCoreNamedVector("colors");
	description->set("", cdesc);

	Mesh->StartMeshGroup((CoreVector*)description.get());
	Mesh->setValue("Optimize", false);


	std::vector<v3f>	vertices;
	std::vector<u32>	indices;

	vertices.push_back({ 0,0,0 });
	vertices.push_back({ 1,0,0 });
	vertices.push_back({ 1,1,0 });
	vertices.push_back({ 0,1,0 });
	vertices.push_back({ 0,0,1 });
	vertices.push_back({ 1,0,1 });
	vertices.push_back({ 1,1,1 });
	vertices.push_back({ 0,1,1 });

	// -x
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(7);
	indices.push_back(0);
	indices.push_back(7);
	indices.push_back(4);

	// +x
	indices.push_back(1);
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(1);
	indices.push_back(6);
	indices.push_back(2);

	// -y
	indices.push_back(1);
	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(1);
	indices.push_back(4);
	indices.push_back(5);

	// +y
	indices.push_back(7);
	indices.push_back(3);
	indices.push_back(6);
	indices.push_back(3);
	indices.push_back(2);
	indices.push_back(6);

	// -z
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	// +z
	indices.push_back(4);
	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(4);
	indices.push_back(7);
	indices.push_back(6);

	// for each face

	v4f	colors[6] = { {1.0,0.0,0.0,0.8},{1.0,1.0,0.0,0.8},{0.0,1.0,0.0,0.8},{0.0,1.0,1.0,0.8},{0.0,0.0,1.0,0.8},{1.0,0.0,1.0,0.8} };

	for (int i = 0; i < 6; i++)
	{
		struct vPlusc
		{
			v3f			 v;
			v4f			 C;
		};

		v4f color = colors[i];

		if (flag & (1 << i))
		{
			vPlusc realvertices[6];
			realvertices[0].v = vertices[indices[i * 6 + 0]];
			realvertices[0].C = color;
			realvertices[1].v = vertices[indices[i * 6 + 1]];
			realvertices[1].C = color;
			realvertices[2].v = vertices[indices[i * 6 + 2]];
			realvertices[2].C = color;
			realvertices[3].v = vertices[indices[i * 6 + 3]];
			realvertices[3].C = color;
			realvertices[4].v = vertices[indices[i * 6 + 4]];
			realvertices[4].C = color;
			realvertices[5].v = vertices[indices[i * 6 + 5]];
			realvertices[5].C = color;

			Mesh->AddTriangle((void*)&realvertices[0], (void*)&realvertices[2], (void*)&realvertices[1]);
			Mesh->AddTriangle((void*)&realvertices[3], (void*)&realvertices[5], (void*)&realvertices[4]);
		}
	}

	auto group = Mesh->EndMeshGroup();
	group->addItem(mCubeMaterial);

	Mesh->EndMeshBuilder();
	Mesh->Init();

	mOctreeCubes[flag] = Mesh;
	return Mesh;

}


SmartPointer<ModernMesh>	MeshSimplifier::buildMesh(const std::vector<u32>& indices, const std::vector<v3f>& vertices, const std::string& meshName)
{
	
	SmartPointer<ModernMesh> Mesh = KigsCore::GetInstanceOf(meshName, "ModernMesh");

	Mesh->StartMeshBuilder();
	auto description = MakeCoreVector();

	auto vertices_desc = MakeCoreNamedVector("vertices");
	description->set("", vertices_desc);

	auto ndesc = MakeCoreNamedVector("generate_normals");
	description->set("", ndesc);
	
	Mesh->StartMeshGroup((CoreVector*)description.get());
	Mesh->setValue("Optimize", false);

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		Mesh->AddTriangle((void*)&vertices[indices[i]], (void*)&vertices[indices[i+1]], (void*)&vertices[indices[i+2]]);
	}
	auto group=Mesh->EndMeshGroup();

	SmartPointer<Material> GeneratedMaterial = KigsCore::GetInstanceOf(meshName + "_M", "Material");
	GeneratedMaterial->SetSpecularColor(0.8,0.8,0.8);
	GeneratedMaterial->SetAmbientColor(0.8,0.8,0.8);
	GeneratedMaterial->SetDiffuseColor(1.0,0.5,0.5);
	GeneratedMaterial->setValue("Shininess", 1.0);
	GeneratedMaterial->Init();

	group->addItem(GeneratedMaterial);

	Mesh->EndMeshBuilder();
	Mesh->Init();

	return Mesh;
}


void	MeshSimplifier::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");
	DECLARE_FULL_CLASS_INFO(KigsCore::Instance(), MeshSimplificationOctree, MeshSimplificationOctree, ModuleName);

	std::vector<v3f> vertices;
	importRaw3DFile("coude.raw3d", mMeshVertexIndices, vertices);

	mMeshSimplification = new CollisionMeshSimplification(mMeshVertexIndices, vertices, 0.02f, false);

	mCubeMaterial = KigsCore::GetInstanceOf("CubeMaterial", "Material");
	mCubeMaterial->SetSpecularColor(0.0, 0.0, 0.0);
	mCubeMaterial->SetAmbientColor(0.8, 0.8, 0.8);
	mCubeMaterial->SetDiffuseColor(1.0, 1.0, 1.0);
	mCubeMaterial->setValue("Shininess", 1.0);
	mCubeMaterial->setValue("BlendEnabled", true);
	mCubeMaterial->Init();

	mDisplayCubeCount.changeNotificationLevel(Owner);
	mDebugCubePos.changeNotificationLevel(Owner);

	mShowObject.changeNotificationLevel(Owner);
	mShowEnveloppe.changeNotificationLevel(Owner);

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	MeshSimplifier::ProtectedUpdate()
{
	if (mScene3D)
	{
		if (!mMeshNode)
		{
			
			auto m = buildMesh(mMeshVertexIndices, mMeshSimplification->getOctreeCoordVertices(), "InOctreeCoordMesh");

			mMeshNode = KigsCore::GetInstanceOf("MeshNode", "Node3D");
			mMeshNode->addItem(m);

			BBox tst = mMeshSimplification->getOctreeBoundingBox();
			mRecenterTranslate = -tst.Center();

			Matrix3x4 topos;
			topos.SetIdentity();
			topos.SetTranslation(mRecenterTranslate);

			mMeshNode->Init();
			mMeshNode->ChangeMatrix(topos);

			mScene3D->addItem(mMeshNode);
			

			// enveloppe
			for (size_t i = 0; i < mMeshSimplification->getEnveloppeSize(); i++)
			{
				u32 flag = 0;
				v3i pos = mMeshSimplification->getEnveloppePos(i, flag);
				pos /= 2;

				SP<Node3D> EnvNode = KigsCore::GetInstanceOf("EnvNode_" + std::to_string(i), "Node3D");

				EnvNode->addItem(getCube(flag));

				topos.SetTranslation(v3f(pos.x, pos.y, pos.z) + mRecenterTranslate);

				EnvNode->Init();
				EnvNode->ChangeMatrix(topos);

				mScene3D->addItem(EnvNode);
			}

			mDebugCubeNode = KigsCore::GetInstanceOf("DebugCube", "Node3D");
			mDebugCubeNode->addItem(getCube(0xFF));

			mScene3D->addItem(mDebugCubeNode);
			moveDebugCube();
			
		}

		if (mShowEdges)
		{
			drawEdges();
		}
		if(mShowVertices)
		{
			drawEnveloppeVertices();
		}


	}
	DataDrivenBaseApplication::ProtectedUpdate();
}

void	MeshSimplifier::ProtectedClose()
{

	delete mMeshSimplification;


	CoreDestroyModule(ModuleThread);
	DataDrivenBaseApplication::ProtectedClose();
}

void	MeshSimplifier::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mScene3D = GetFirstInstanceByName("Scene3D", "myScene");

		CMSP cam=mScene3D->GetFirstSonByName("Camera", "Camera",true);
		cam->Upgrade("OrbitCameraUp");
	}
}
void	MeshSimplifier::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mScene3D=nullptr;
		mMeshNode=nullptr;

		mOctreeCubes.clear();
	}
}

void	MeshSimplifier::drawEnveloppeVertices()
{
	auto vertices = mMeshSimplification->getEnveloppeVertices();
	for (size_t i = 0; i < vertices.size(); i++)
	{
		auto& v = vertices[i];
		if (i < mDisplayVerticeCount)
		{
			float r = 0.05f;
			if (mSelectedVerticeIndex == i)
			{
				r *= 3.0f;
				dd::sphere(v + mRecenterTranslate, v3f(1.0f,0.0f,0.0), r);
			}
			dd::cross(v + mRecenterTranslate,r);
		}
	}
}

void	MeshSimplifier::drawEdges()
{
	auto edgeList=mMeshSimplification->getEdges();

	for (size_t i = 0; i < edgeList.size(); i++)
	{
		auto& e = edgeList[i];
	
		if (i < mDisplayEdgeCount)
		{
			v3f color(0.0, 1.0, 0.0);
			if (mSelectedEdgeIndex == i)
			{
				color.Set(1.0,1.0,0.0);
			}
			dd::line(e.first + mRecenterTranslate, e.second + mRecenterTranslate, color);
		}
	}
}

void MeshSimplifier::showEnveloppe(bool show)
{
	for (size_t i = 0; i < mMeshSimplification->getEnveloppeSize(); i++)
	{
		SP<Node3D> EnvNode = mScene3D->GetFirstSonByName("Node3D", "EnvNode_" + std::to_string(i));

		if (i <= mDisplayCubeCount)
		{
			EnvNode->setValue("Show", show);
		}
		else
		{
			EnvNode->setValue("Show", false);
		}
	}
}
void MeshSimplifier::NotifyUpdate(const unsigned int  labelid )
{
	if (!mScene3D)
	{
		return;
	}
	if ( (labelid == mDisplayCubeCount.getID()) || (labelid == mShowEnveloppe.getID()))
	{
		showEnveloppe(mShowEnveloppe);
	}
	else if (labelid == mDebugCubePos.getID())
	{
		moveDebugCube();
	}
	else if (labelid == mShowObject.getID())
	{
		mMeshNode->setValue("Show", mShowObject);
	}
	

}

void	MeshSimplifier::moveDebugCube()
{
	Matrix3x4 topos;
	v3i pos = (v3i)mDebugCubePos;
	pos /= 2;
	v3f posf(pos.x, pos.y, pos.z);
	topos.SetIdentity();
	topos.SetTranslation(posf + mRecenterTranslate);

	mDebugCubeNode->ChangeMatrix(topos);
}
