#include <MeshSimplifier.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>
#include "MeshSimplification.h"
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

		std::cout << "In vertice count: " << vcount << std::endl;
		std::cout << "In triangle count: " << icount/3 << std::endl;


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
SmartPointer<ModernMesh>	MeshSimplifier::getCube(u32 flag,u32 debugflag)
{
	// cube already in map
	if (mOctreeCubes.find(flag|(debugflag<<8)) != mOctreeCubes.end())
	{
		return mOctreeCubes[flag | (debugflag << 8)];
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

		if (debugflag)
		{
			color = color * 0.5f;
			color.a = 0.8f;
		}

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

	mOctreeCubes[flag | (debugflag << 8)] = Mesh;
	return Mesh;

}


SmartPointer<ModernMesh>	MeshSimplifier::buildMesh(const std::vector<u32>& indices,const std::vector<v3f>& vertices, const std::string& meshName, const v3f& color)
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

	/*for (size_t i = 0; i < indices.size(); i += 3)
	{
		Mesh->AddTriangle((void*)&vertices[indices[i]], (void*)&vertices[indices[i+1]], (void*)&vertices[indices[i+2]]);
	}*/
	auto group=Mesh->EndMeshGroup(vertices.size(),(v3f*)vertices.data(),nullptr,nullptr,nullptr,indices.size()/3,(v3u*)indices.data());

	SmartPointer<Material> GeneratedMaterial = KigsCore::GetInstanceOf(meshName + "_M", "Material");
	GeneratedMaterial->SetSpecularColor(0.8,0.8,0.8);
	GeneratedMaterial->SetAmbientColor(0.8,0.8,0.8);
	GeneratedMaterial->SetDiffuseColor(color.x,color.y,color.z);
	GeneratedMaterial->setValue("Shininess", 1.0);
	GeneratedMaterial->Init();

	group->addItem(GeneratedMaterial);

	Mesh->EndMeshBuilder();
	Mesh->Init();

	return Mesh;
}

void MeshSimplifier::addBboxToMesh(const BBox& boxtoadd)
{
	int startVerticeIndex = mMeshVertices.size();

	mMeshVertices.push_back({ boxtoadd.m_Min.x,boxtoadd.m_Min.y,boxtoadd.m_Min.z });
	mMeshVertices.push_back({ boxtoadd.m_Max.x,boxtoadd.m_Min.y,boxtoadd.m_Min.z });
	mMeshVertices.push_back({ boxtoadd.m_Max.x,boxtoadd.m_Max.y,boxtoadd.m_Min.z });
	mMeshVertices.push_back({ boxtoadd.m_Min.x,boxtoadd.m_Max.y,boxtoadd.m_Min.z });
	mMeshVertices.push_back({ boxtoadd.m_Min.x,boxtoadd.m_Min.y,boxtoadd.m_Max.z });
	mMeshVertices.push_back({ boxtoadd.m_Max.x,boxtoadd.m_Min.y,boxtoadd.m_Max.z });
	mMeshVertices.push_back({ boxtoadd.m_Max.x,boxtoadd.m_Max.y,boxtoadd.m_Max.z });
	mMeshVertices.push_back({ boxtoadd.m_Min.x,boxtoadd.m_Max.y,boxtoadd.m_Max.z });

	// -x
	mMeshVertexIndices.push_back(startVerticeIndex + 0);
	mMeshVertexIndices.push_back(startVerticeIndex + 7);
	mMeshVertexIndices.push_back(startVerticeIndex + 3);
	mMeshVertexIndices.push_back(startVerticeIndex + 0);
	mMeshVertexIndices.push_back(startVerticeIndex + 4);
	mMeshVertexIndices.push_back(startVerticeIndex + 7);

	// +x
	mMeshVertexIndices.push_back(startVerticeIndex + 1);
	mMeshVertexIndices.push_back(startVerticeIndex + 6);
	mMeshVertexIndices.push_back(startVerticeIndex + 5);
	mMeshVertexIndices.push_back(startVerticeIndex + 1);
	mMeshVertexIndices.push_back(startVerticeIndex + 2);
	mMeshVertexIndices.push_back(startVerticeIndex + 6);

	// -y
	mMeshVertexIndices.push_back(startVerticeIndex + 1);
	mMeshVertexIndices.push_back(startVerticeIndex + 4);
	mMeshVertexIndices.push_back(startVerticeIndex + 0);
	mMeshVertexIndices.push_back(startVerticeIndex + 1);
	mMeshVertexIndices.push_back(startVerticeIndex + 5);
	mMeshVertexIndices.push_back(startVerticeIndex + 4);

	// +y
	mMeshVertexIndices.push_back(startVerticeIndex + 7);
	mMeshVertexIndices.push_back(startVerticeIndex + 6);
	mMeshVertexIndices.push_back(startVerticeIndex + 3);
	mMeshVertexIndices.push_back(startVerticeIndex + 3);
	mMeshVertexIndices.push_back(startVerticeIndex + 6);
	mMeshVertexIndices.push_back(startVerticeIndex + 2);

	// -z
	mMeshVertexIndices.push_back(startVerticeIndex + 0);
	mMeshVertexIndices.push_back(startVerticeIndex + 2);
	mMeshVertexIndices.push_back(startVerticeIndex + 1);
	mMeshVertexIndices.push_back(startVerticeIndex + 0);
	mMeshVertexIndices.push_back(startVerticeIndex + 3);
	mMeshVertexIndices.push_back(startVerticeIndex + 2);

	// +z
	mMeshVertexIndices.push_back(startVerticeIndex + 4);
	mMeshVertexIndices.push_back(startVerticeIndex + 5);
	mMeshVertexIndices.push_back(startVerticeIndex + 6);
	mMeshVertexIndices.push_back(startVerticeIndex + 4);
	mMeshVertexIndices.push_back(startVerticeIndex + 6);
	mMeshVertexIndices.push_back(startVerticeIndex + 7);
}

void MeshSimplifier::createTestMesh()
{
	mMeshVertices.clear();
	mMeshVertexIndices.clear();

	BBox	FirstOne({-0.5f,-1.0f,-1.0f}, {0.5f,1.0f,1.0f});
	addBboxToMesh(FirstOne);

	BBox	SecondOne({-1.0f,-0.2f,-1.0f}, {1.0f,0.2f,1.0f});
	addBboxToMesh(SecondOne);

/*	BBox	ThirdOne({-0.6f,-0.6f,-0.6f}, {0.6f,0.6f,0.6f});
	addBboxToMesh(ThirdOne);*/


}

void	MeshSimplifier::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");
	DECLARE_FULL_CLASS_INFO(KigsCore::Instance(), MeshSimplificationOctree, MeshSimplificationOctree, ModuleName);


	//createTestMesh();
	//mPrecision = 0.2f;

	//importRaw3DFile("coude.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.01f;
	//importRaw3DFile("complex.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.1f;
	//importRaw3DFile("complexcao.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.01f;
	//mConstructEnveloppe = false;
	//mConstructMesh = false;
	//importRaw3DFile("simplebox.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.02f;
	//importRaw3DFile("almostbox.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.01f;
	//importRaw3DFile("crash.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.308378041f;
	//importRaw3DFile("crashAgain.raw3d", mMeshVertexIndices, mMeshVertices);
	//mPrecision = 0.200000003f;

	float precision;
	//LoadFromString(std::tie(precision, mMeshVertexIndices, mMeshVertices), "aknhcddmboaaaaaaaaaaaaaaaaaaaaaaabaaaaaaacaaaaaaaaaaaaaaacaaaaaaadaaaaaaaaaaaaaaadaaaaaaaeaaaaaaadaaaaaaafaaaaaaaeaaaaaaaeaaaaaaagaaaaaaaaaaaaaaagaaaaaaahaaaaaaaaaaaaaaagaaaaaaaiaaaaaaahaaaaaaagaaaaaaajaaaaaaaiaaaaaaaiaaaaaaakaaaaaaahaaaaaaaiaaaaaaalaaaaaaakaaaaaaamaaaaaaaaaaaaaapbbjpodpgjihieeaaaaaaaiakpljmgeagjihieeaaaaaaaiakpljmgeankekkmeaaaaaaaiallacnmlpnkekkmeaaaaaaaiaeggpbldpepncifeaaaaaaaiallacnmlpepncifeaaaaaaaiaeggpbldpafijhemaaaaaaaiapbbjpodpojkklamaaaaaaaiallacnmlpcglfilmaaaaaaaiallacnmlpafijhemaaaaaaaialhkmklmaojkklamaaaaaaaaalhkmklmacglfilmaaaaaaaia");
	//LoadFromString(std::tie(precision, mMeshVertexIndices, mMeshVertices), "aknhcddmaiabaaaaaaaaaaaaaaaaaaaaabaaaaaaacaaaaaaadaaaaaaaaaaaaaaacaaaaaaadaaaaaaaeaaaaaaaaaaaaaaadaaaaaaafaaaaaaaeaaaaaaaeaaaaaaafaaaaaaagaaaaaaaaaaaaaaaeaaaaaaahaaaaaaaeaaaaaaaiaaaaaaahaaaaaaaiaaaaaaajaaaaaaahaaaaaaaiaaaaaaaeaaaaaaakaaaaaaahaaaaaaalaaaaaaaaaaaaaaalaaaaaaamaaaaaaaaaaaaaaaaaaaaaaamaaaaaaanaaaaaaalaaaaaaahaaaaaaaoaaaaaaaaaaaaaaanaaaaaaapaaaaaaaoaaaaaaahaaaaaabaaaaaaabbaaaaaabcaaaaaabdaaaaaabeaaaaaabbaaaaaabdaaaaaabeaaaaaabfaaaaaabbaaaaaabfaaaaaabgaaaaaabbaaaaaabhaaaaaabeaaaaaabdaaaaaabhaaaaaabiaaaaaabeaaaaaabhaaaaaabjaaaaaabiaaaaaabeaaaaaabiaaaaaabkaaaaaabdaaaaaablaaaaaabhaaaaaabdaaaaaabmaaaaaablaaaaaabmaaaaaabnaaaaaablaaaaaabmaaaaaabdaaaaaaboaaaaaabdaaaaaabpaaaaaaboaaaaaaboaaaaaacaaaaaaabmaaaaaablaaaaaacbaaaaaabhaaaaaabhaaaaaacbaaaaaaccaaaaaacbaaaaaacdaaaaaaccaaaaaabhaaaaaaccaaaaaaceaaaaaablaaaaaacfaaaaaacbaaaaaablaaaaaacgaaaaaacfaaaaaacbaaaaaacfaaaaaachaaaaaacfaaaaaaciaaaaaachaaaaaacjaaaaaackaaaaaaclaaaaaacmaaaaaacjaaaaaaclaaaaaacnaaaaaacjaaaaaacmaaaaaacnaaaaaacmaaaaaacoaaaaaacnaaaaaacoaaaaaacpaaaaaacoaaaaaacmaaaaaadaaaaaaadbaaaaaacjaaaaaacnaaaaaacjaaaaaadbaaaaaadcaaaaaadcaaaaaadbaaaaaaddaaaaaacjaaaaaadcaaaaaadeaaaaaacjaaaaaadeaaaaaadfaaaaaadeaaaaaadcaaaaaadgaaaaaadbaaaaaacnaaaaaadhaaaaaadhaaaaaacnaaaaaadiaaaaaadhaaaaaadiaaaaaadjaaaaaadiaaaaaacnaaaaaadkaaaaaadbaaaaaadhaaaaaadlaaaaaadbaaaaaadlaaaaaadmaaaaaadbaaaaaadmaaaaaadnaaaaaadlaaaaaadhaaaaaadoaaaaaadoaaaaaadhaaaaaadpaaaaaadlaaaaaadoaaaaaaeaaaaaaaebaaaaaaecaaaaaaedaaaaaaeeaaaaaaebaaaaaaedaaaaaaefaaaaaaebaaaaaaeeaaaaaaefaaaaaaeeaaaaaaegaaaaaaefaaaaaaegaaaaaaehaaaaaaeeaaaaaaeiaaaaaaegaaaaaaefaaaaaaejaaaaaaebaaaaaaebaaaaaaejaaaaaaekaaaaaaekaaaaaaejaaaaaaelaaaaaaebaaaaaaekaaaaaaemaaaaaaebaaaaaaemaaaaaaenaaaaaaemaaaaaaekaaaaaaeoaaaaaaejaaaaaaefaaaaaaepaaaaaaepaaaaaaefaaaaaafaaaaaaaepaaaaaafaaaaaaafbaaaaaafaaaaaaaefaaaaaafcaaaaaaejaaaaaaepaaaaaafdaaaaaaejaaaaaafdaaaaaafeaaaaaaejaaaaaafeaaaaaaffaaaaaafdaaaaaaepaaaaaafgaaaaaafgaaaaaaepaaaaaafhaaaaaafdaaaaaafgaaaaaafiaaaaaafjaaaaaafkaaaaaaflaaaaaafjaaaaaafmaaaaaafkaaaaaafkaaaaaafmaaaaaafnaaaaaafjaaaaaafoaaaaaafmaaaaaafoaaaaaafpaaaaaafmaaaaaagaaaaaaafoaaaaaafjaaaaaagaaaaaaagbaaaaaafoaaaaaagcaaaaaaaaaaaaaamckflndopaggdplojoopjhlplgbgladofoiigmlojoopjhlpmdmdjndopcckijlojoopjhlphjkbihdohlgljhlojoopjhlpokccokdnjcehkalojoopjhlpiikofndojcehkalojoopjhlpppfpcjdopjeikdlojoopjhlpbkcocclnpaggdplojoopjhlpmamdljdmpcckijlojoopjhlpnpngfglmfoiigmlojoopjhlpbfpkigdnhlgljhlojoopjhlpbjgkpndnclfgejdojoopjhlppbakfedoclfgejdojoopjhlpjnfijedoiiccfedojoopjhlpaodlcidniiccfedojoopjhlpmckflndonciogjdojoopjhlpbkcocclnnciogjdojoopjhlpffkkecdnmkhfbclnphkadnlnfomjdidnfcinbglnphkadnlnlcendadnleapbnlnphkadnlncalpgbdnfcinbglnphkadnlncjnofhdnmkhfbclnphkadnlndpeeendnhobabblnphkadnlnacdkhgdnebagdklnphkadnlncolnhadngailcflnphkadnlnlhnehednfhgmcplnphkadnlnmmdkgkdnleapbnlnphkadnlnlcendadnmopmfglnphkadnlnmildcfdnclkaeelnphkadnlnfamlcjdnccibeolnphkadnlnmildcfdnfhgmcplnphkadnlnfamlcjdngailcflnphkadnlnhneocednebagdklnphkadnlncalpgbdndahpfnlnphkadnlncolnhadnccibeolnphkadnlnmmdkgkdnmopmfglnphkadnlnlhnehednclkaeelnphkadnlnffkkecdnlijggblnphkadnlnfomjdidndahpfnlnphkadnlncjnofhdnlijggblnphkadnlndpeeendnaepmgclnphkadnlncjpngcdncllpeplnncdamjdngikafmdnpekafelnncdamjdnkpdhffdnjllcfhlnncdamjdndpeeendnjdlofilnncdamjdnimkjdcdngkgcejlnncdamjdnbgoidndnpekafelnncdamjdnffildhdncllpeplnncdamjdnnafaefdnjllcfhlnncdamjdncjpngcdnfhencelnncdamjdnjjpagkdnnbbcdclnncdamjdnpdnoghdnbikkcklnncdamjdnjjpagkdnlbpjeblnncdamjdnpdnoghdngkgcejlnncdamjdnjbpmgldnebagdklnncdamjdnimkjdcdnbikkcklnncdamjdnonilcodnebagdklnncdamjdnogjhcpdnnbbcdclnncdamjdnogjhcpdnlbpjeblnncdamjdndpeeendnopenbllnncdamjdnkpdhffdnohfjbmlnncdamjdngikafmdnioglbplnncdamjdnbgoidndnioglbplnncdamjdnffildhdnfhencelnncdamjdnnafaefdnohfjbmlnncdamjdncjpngcdncllpeplnklpacfdogikafmdnpekafelnklpacfdokpdhffdnjllcfhlnklpacfdodpeeendnjdlofilnklpacfdoimkjdcdngkgcejlnklpacfdobgoidndnpekafelnklpacfdoffildhdncllpeplnklpacfdonafaefdnjllcfhlnklpacfdocjpngcdnfhencelnklpacfdojjpagkdnnbbcdclnklpacfdopdnoghdnbikkcklnklpacfdojjpagkdnlbpjeblnklpacfdopdnoghdngkgcejlnklpacfdojbpmgldnebagdklnklpacfdoimkjdcdnbikkcklnklpacfdoonilcodnebagdklnklpacfdoogjhcpdnnbbcdclnklpacfdoogjhcpdnlbpjeblnklpacfdodpeeendnopenbllnklpacfdokpdhffdnohfjbmlnklpacfdogikafmdnioglbplnklpacfdobgoidndnioglbplnklpacfdoffildhdnfhencelnklpacfdonafaefdnohfjbmlnklpacfdolfmfhbdnfbdhiblnmjgaacdolfmfhbdnakihfelncnibphdnlfmfhbdnhedjgnlnjijopmdnlfmfhbdnhiifbplncnibphdnlfmfhbdnebagdklnipmcpfdnlfmfhbdnlpdlodlmmjgaacdolfmfhbdnaondaglnjijopmdnlfmfhbdnfbdhiblnpkjecgdolfmfhbdnlpdlodlmpkjecgdo");
	//LoadFromString(std::tie(precision, mMeshVertexIndices, mMeshVertices), "aknhcddmiiacaaaaaaaaaaaaaaaaaaaaabaaaaaaacaaaaaaacaaaaaaadaaaaaaaaaaaaaaaeaaaaaaafaaaaaaagaaaaaaaeaaaaaaahaaaaaaafaaaaaaacaaaaaaaiaaaaaaadaaaaaaacaaaaaaajaaaaaaaiaaaaaaajaaaaaaacaaaaaaakaaaaaaajaaaaaaakaaaaaaalaaaaaaahaaaaaaaeaaaaaaamaaaaaaahaaaaaaamaaaaaaajaaaaaaajaaaaaaalaaaaaaanaaaaaaajaaaaaaanaaaaaaahaaaaaaaoaaaaaaapaaaaaabaaaaaaabbaaaaaaapaaaaaaaoaaaaaabaaaaaaabcaaaaaaaoaaaaaaaoaaaaaabdaaaaaabbaaaaaabeaaaaaabbaaaaaabdaaaaaabcaaaaaabaaaaaaabfaaaaaabdaaaaaabgaaaaaabeaaaaaabfaaaaaabhaaaaaabcaaaaaabeaaaaaabgaaaaaabiaaaaaabhaaaaaabfaaaaaabjaaaaaabiaaaaaabkaaaaaabeaaaaaabjaaaaaablaaaaaabhaaaaaablaaaaaabjaaaaaabmaaaaaabkaaaaaabiaaaaaabnaaaaaabmaaaaaaboaaaaaablaaaaaabnaaaaaabpaaaaaabkaaaaaacaaaaaaaboaaaaaabmaaaaaabpaaaaaabnaaaaaacbaaaaaabmaaaaaaccaaaaaacaaaaaaacdaaaaaacaaaaaaaccaaaaaacbaaaaaaceaaaaaabpaaaaaaceaaaaaacbaaaaaacfaaaaaaccaaaaaacgaaaaaacdaaaaaacfaaaaaachaaaaaaceaaaaaacdaaaaaacgaaaaaaciaaaaaacjaaaaaachaaaaaacfaaaaaaciaaaaaackaaaaaacdaaaaaacfaaaaaaclaaaaaacjaaaaaacmaaaaaacjaaaaaaclaaaaaackaaaaaaciaaaaaacnaaaaaaclaaaaaacoaaaaaacmaaaaaacnaaaaaacpaaaaaackaaaaaacmaaaaaacoaaaaaadaaaaaaacpaaaaaacnaaaaaadbaaaaaadaaaaaaadcaaaaaacmaaaaaadbaaaaaaddaaaaaacpaaaaaaddaaaaaadbaaaaaadeaaaaaadcaaaaaadaaaaaaadfaaaaaadeaaaaaadgaaaaaaddaaaaaadfaaaaaadhaaaaaadcaaaaaadiaaaaaadgaaaaaadeaaaaaadhaaaaaadfaaaaaadjaaaaaadeaaaaaadkaaaaaadiaaaaaadlaaaaaadiaaaaaadkaaaaaadjaaaaaadmaaaaaadhaaaaaadmaaaaaadjaaaaaadlaaaaaadkaaaaaadnaaaaaadlaaaaaadlaaaaaadnaaaaaadmaaaaaadoaaaaaadpaaaaaaeaaaaaaaebaaaaaadpaaaaaadoaaaaaaeaaaaaaaecaaaaaadoaaaaaadoaaaaaaedaaaaaaebaaaaaaeeaaaaaaebaaaaaaedaaaaaaecaaaaaaeaaaaaaaefaaaaaaedaaaaaaegaaaaaaeeaaaaaaefaaaaaaehaaaaaaecaaaaaaeeaaaaaaegaaaaaaeiaaaaaaehaaaaaaefaaaaaaejaaaaaaeiaaaaaaekaaaaaaeeaaaaaaejaaaaaaelaaaaaaehaaaaaaelaaaaaaejaaaaaaemaaaaaaekaaaaaaeiaaaaaaenaaaaaaemaaaaaaeoaaaaaaelaaaaaaenaaaaaaepaaaaaaekaaaaaafaaaaaaaeoaaaaaaemaaaaaaepaaaaaaenaaaaaafbaaaaaaemaaaaaafcaaaaaafaaaaaaafdaaaaaafaaaaaaafcaaaaaafbaaaaaafeaaaaaaepaaaaaafeaaaaaafbaaaaaaffaaaaaafcaaaaaafgaaaaaafdaaaaaaffaaaaaafhaaaaaafeaaaaaafdaaaaaafgaaaaaafiaaaaaafjaaaaaafhaaaaaaffaaaaaafiaaaaaafkaaaaaafdaaaaaaffaaaaaaflaaaaaafjaaaaaafmaaaaaafjaaaaaaflaaaaaafkaaaaaafiaaaaaafnaaaaaaflaaaaaafoaaaaaafmaaaaaafnaaaaaafpaaaaaafkaaaaaafmaaaaaafoaaaaaagaaaaaaafpaaaaaafnaaaaaagbaaaaaagaaaaaaagcaaaaaafmaaaaaagbaaaaaagdaaaaaafpaaaaaagdaaaaaagbaaaaaageaaaaaagcaaaaaagaaaaaaagfaaaaaageaaaaaaggaaaaaagdaaaaaagfaaaaaaghaaaaaagcaaaaaagiaaaaaaggaaaaaageaaaaaaghaaaaaagfaaaaaagjaaaaaageaaaaaagkaaaaaagiaaaaaaglaaaaaagiaaaaaagkaaaaaagjaaaaaagmaaaaaaghaaaaaagmaaaaaagjaaaaaaglaaaaaagkaaaaaagnaaaaaaglaaaaaaglaaaaaagnaaaaaagmaaaaaagoaaaaaagpaaaaaahaaaaaaahbaaaaaahaaaaaaagpaaaaaagoaaaaaahcaaaaaagpaaaaaahdaaaaaahbaaaaaagpaaaaaahcaaaaaagoaaaaaaheaaaaaahcaaaaaaheaaaaaahfaaaaaahdaaaaaahgaaaaaahbaaaaaahgaaaaaahdaaaaaahhaaaaaahcaaaaaahfaaaaaahiaaaaaahhaaaaaahdaaaaaahjaaaaaahfaaaaaahkaaaaaahiaaaaaahhaaaaaahjaaaaaahlaaaaaahmaaaaaahkaaaaaahfaaaaaahnaaaaaahhaaaaaahlaaaaaahkaaaaaahmaaaaaahoaaaaaahlaaaaaahpaaaaaahnaaaaaahoaaaaaahmaaaaaaiaaaaaaaiaaaaaaaibaaaaaahoaaaaaahpaaaaaaicaaaaaahnaaaaaaicaaaaaahpaaaaaaidaaaaaaieaaaaaaibaaaaaaiaaaaaaaifaaaaaaicaaaaaaidaaaaaaigaaaaaaibaaaaaaieaaaaaaihaaaaaaifaaaaaaidaaaaaaibaaaaaaigaaaaaaiiaaaaaaidaaaaaaijaaaaaaihaaaaaaikaaaaaaiiaaaaaaigaaaaaailaaaaaaihaaaaaaijaaaaaaikaaaaaaimaaaaaaiiaaaaaaimaaaaaaikaaaaaainaaaaaailaaaaaaijaaaaaaioaaaaaailaaaaaaioaaaaaaipaaaaaaimaaaaaainaaaaaajaaaaaaajbaaaaaajaaaaaaainaaaaaaioaaaaaajcaaaaaaipaaaaaajbaaaaaainaaaaaajdaaaaaaipaaaaaajcaaaaaajeaaaaaajdaaaaaajfaaaaaajbaaaaaajgaaaaaaipaaaaaajeaaaaaajfaaaaaajhaaaaaajbaaaaaajgaaaaaajeaaaaaajiaaaaaajfaaaaaajjaaaaaajhaaaaaajiaaaaaajjaaaaaajgaaaaaajiaaaaaajhaaaaaajjaaaaaajkaaaaaajlaaaaaajmaaaaaajnaaaaaajkaaaaaajmaaaaaajoaaaaaajlaaaaaajkaaaaaajkaaaaaajnaaaaaajpaaaaaajlaaaaaajoaaaaaakaaaaaaajpaaaaaajnaaaaaakbaaaaaakaaaaaaajoaaaaaakcaaaaaajpaaaaaakbaaaaaakdaaaaaakeaaaaaakaaaaaaakcaaaaaakfaaaaaakdaaaaaakbaaaaaakeaaaaaakcaaaaaakgaaaaaakfaaaaaakhaaaaaakdaaaaaakgaaaaaakiaaaaaakeaaaaaakhaaaaaakfaaaaaakjaaaaaakgaaaaaakkaaaaaakiaaaaaakhaaaaaakjaaaaaaklaaaaaakmaaaaaakiaaaaaakkaaaaaaknaaaaaaklaaaaaakjaaaaaakmaaaaaakkaaaaaakoaaaaaaknaaaaaakpaaaaaaklaaaaaakoaaaaaalaaaaaaakmaaaaaakpaaaaaaknaaaaaalbaaaaaakoaaaaaalcaaaaaalaaaaaaakpaaaaaalbaaaaaaldaaaaaaleaaaaaalaaaaaaalcaaaaaalfaaaaaaldaaaaaalbaaaaaaleaaaaaalcaaaaaalgaaaaaaldaaaaaalfaaaaaalhaaaaaalgaaaaaaliaaaaaaleaaaaaalhaaaaaalfaaaaaaljaaaaaalgaaaaaalkaaaaaaliaaaaaalhaaaaaaljaaaaaallaaaaaalmaaaaaaliaaaaaalkaaaaaalnaaaaaallaaaaaaljaaaaaalmaaaaaalkaaaaaaloaaaaaallaaaaaalnaaaaaalpaaaaaalmaaaaaaloaaaaaamaaaaaaalpaaaaaalnaaaaaambaaaaaaloaaaaaamcaaaaaamaaaaaaalpaaaaaambaaaaaamdaaaaaameaaaaaamaaaaaaamcaaaaaamfaaaaaamdaaaaaambaaaaaameaaaaaamcaaaaaamgaaaaaamhaaaaaameaaaaaamgaaaaaamdaaaaaamfaaaaaamiaaaaaamgaaaaaamjaaaaaamhaaaaaamiaaaaaamfaaaaaamkaaaaaamhaaaaaamjaaaaaamlaaaaaamiaaaaaamkaaaaaammaaaaaamnaaaaaamhaaaaaamlaaaaaamoaaaaaammaaaaaamkaaaaaamlaaaaaampaaaaaamnaaaaaamoaaaaaanaaaaaaammaaaaaampaaaaaanbaaaaaamnaaaaaanaaaaaaamoaaaaaancaaaaaanbaaaaaampaaaaaandaaaaaanaaaaaaancaaaaaaneaaaaaanfaaaaaanbaaaaaandaaaaaaneaaaaaancaaaaaangaaaaaanfaaaaaandaaaaaanhaaaaaangaaaaaaniaaaaaaneaaaaaanhaaaaaanjaaaaaanfaaaaaangaaaaaanjaaaaaaniaaaaaanjaaaaaanhaaaaaaniaaaaaankaaaaaaaaaaaaaabcidmalmdgbenelmjkbfjkdmbcidmalmpoiehklnelgechdnbcidmalmgmoiinlnogbooldmbcidmalmkodeoilmoaehdidmbcidmalmdgbenelmjkbfjklmbcidmalmpoiehklnelgechlnbcidmalmcedjfjlncedjfjlnbcidmalmgmoiinlnogboollmbcidmalmmfcmphlmoaehlidlbcidmalmgpbcadlnffhacncibcidmalmadmbjdlnogbogldmbcidmalmjkjjjjlnffhacncibcidmalmikbmonlmjkbfbklmbcidmalmogbijclnanoajglmohplcjlnofjedklmebbfcodncnlcbnlnofjedklmebbfcodncnlcbnlnaabaolkgfidjdedncnlcbnlnfidjlelmbnbebmdnohplcjlnaabaolkgfidjdednohplcjlnfidjlelmbnbebmdncnlcbnlnapoapolmapoapodmcnlcbnlnofjedkdmebbfcodnohplcjlnapoapolmapoapodmohplcjlnofjedkdmebbfcodnohplcjlnbnbebmlnfidjledmcnlcbnlnfidjledmbnbebmdncnlcbnlnbnbebmlnfidjledmohplcjlnfidjledmbnbebmdncnlcbnlnapoapodmapoapodmohplcjlnebbfcolnofjedkdmohplcjlnapoapodmapoapodmcnlcbnlnebbfcolnofjedkdmohplcjlnbnbebmdnfidjledmohplcjlnfidjdelnffhacncicnlcbnlnbnbebmdnfidjledmohplcjlnebbfcodnofjedkdmcnlcbnlnfidjdelnffhacnciohplcjlnebbfcolnofjedklmcnlcbnlnebbfcodnofjedkdmcnlcbnlnebbfcolnofjedklmcnlcbnlnfidjdednffhacncicnlcbnlnbnbebmlnfidjlelmohplcjlnfidjdednffhacnciohplcjlnbnbebmlnfidjlelmcnlcbnlnapoapolmapoapolmcnlcbnlnebbfcodnofjedklmohplcjlnapoapolmapoapolmohplcjlnebbfcodnofjedklmohplcjlnfidjlelmbnbebmlncnlcbnlnbnbebmdnfidjlelmcnlcbnlnfidjlelmbnbebmlnohplcjlnbnbebmdnfidjlelmcnlcbnlnapoapodmapoapolmohplcjlnofjedklmebbfcolnohplcjlnapoapodmapoapolmcnlcbnlnofjedklmebbfcolnohplcjlnfidjledmbnbebmlnohplcjlnaabaolkgfidjdelncnlcbnlnfidjledmbnbebmlnohplcjlnofjedkdmebbfcolncnlcbnlnaabaolkgfidjdelncnlcbnlnofjedkdmebbfcolnecgagfllhobilpdmndnammdlecgagfdlhobilpdmndnammdlecgagfdldjngmfdmffhacnciecgagfdlokfekldmdjngefdmecgagflldjngmfdmffhacnciecgagfllokfekldmdjngefdmecgagfdlejoeildmejoeildmecgagfdlhobilpdmndnammllecgagfllejoeildmejoeildmecgagfllhobilpdmndnammllecgagflldjngefdmokfekldmecgagfdlokfekldmdjngeflmecgagfdldjngefdmokfekldmecgagfllokfekldmdjngeflmecgagfdlejoeildmejoeillmecgagfllndnammdlhobilpdmecgagfllejoeildmejoeillmecgagfdlndnammdlhobilpdmecgagflldjngefdmokfekllmecgagfllaabaolkgdjngmfdmecgagfdldjngefdmokfekllmecgagfllndnammdlhobilplmecgagfdlaabaolkgdjngmfdmecgagfllndnammllhobilpdmecgagfdlndnammdlhobilplmecgagfdlndnammllhobilpdmecgagfdlaabaolkgdjngmflmecgagfdldjngeflmokfekldmecgagfllaabaolkgdjngmflmecgagflldjngeflmokfekldmecgagfdlejoeillmejoeildmecgagfdlndnammllhobilplmecgagfllejoeillmejoeildmecgagfllndnammllhobilplmecgagfllokfekllmdjngefdmecgagfdldjngeflmokfekllmecgagfdlokfekllmdjngefdmecgagflldjngeflmokfekllmecgagfdlejoeillmejoeillmecgagfllhobilplmndnammdlecgagfllejoeillmejoeillmecgagfdlhobilplmndnammdlecgagfllokfekllmdjngeflmecgagflldjngmflmffhacnciecgagfdlokfekllmdjngeflmecgagfllhobilplmndnammllecgagfdldjngmflmffhacnciecgagfdlhobilplmndnammllecgagflldaiklkdmambdilllbcidmalmgpbcaddnffhacnciecgagflldjngmfdmffhacnciecgagflldaiklkdmalbdildlbcidmalmikbmondmjkbfbklmbcidmalmikbmondmjkbfbkdmecgagfllcidokpdmambdallmecgagfllkhankadmdejcgilmecgagfllcidokpdmalbdaldmecgagfllkhankadmdejcgidmbcidmalmdgbenedmjkbfjklmbcidmalmdgbenedmjkbfjkdmbcidmalmppikjcdmpglcmjlmbcidmalmppikjcdmpglcmjdmecgagflldcdafndmeddijilmecgagflldcdafndmeddijidmbcidmalmipadccdmfdfapjlmbcidmalmipadccdmfdfapjdmecgagfllcmikpedlglchlmlmbcidmalmaabaolkgfdfapjlmecgagfllcmikpedlglchlmdmbcidmalmaabaolkgfdfapjdmecgagfllaabaolkgglchlmlmecgagfllaabaolkgglchlmdmecgagfllcmikpellglchlmlmecgagfllcmikpellglchlmdmbcidmalmipadcclmfdfapjlmbcidmalmipadcclmfdfapjdmecgagflldcdafnlmeddijilmecgagflldcdafnlmeddijidmbcidmalmppikjclmpglcmjlmecgagfllkhankalmdejcgilmbcidmalmppikjclmpglcmjdmecgagfllkhankalmdejcgidmbcidmalmdgbenelmjkbfjklmbcidmalmikbmonlmjkbfbklmbcidmalmdgbenelmjkbfjkdmecgagfllcidokplmalbdallmbcidmalmkodeoilmoaehdidmecgagflldaiklklmalbdilllecgagfllpapblclmdejcoidlbcidmalmgpbcadlnffhacncibcidmalmmfcmphlmoaehlidlecgagflldjngmflmffhacncicnlcbnlngmoiindnogbooldmbcidmalmpoiehkdnelgechdnbcidmalmgmoiindnogbooldmbcidmalmogbijcdnanoajgdmcnlcbnlnpoiehkdnelgechdncnlcbnlnadmbjddnogbogldmbcidmalmcedjfjdncedjfjdnbcidmalmjkjjjjdnffhacncicnlcbnlncedjfjdncedjfjdncnlcbnlnjkjjjjdnffhacncibcidmalmelgechdnpoiehkdnbcidmalmadmbjddnogbogllmcnlcbnlnelgechdnpoiehkdncnlcbnlnogbijcdnanoajglmbcidmalmogbooldmgmoiindnbcidmalmgmoiindnogboollmcnlcbnlnogbooldmgmoiindncnlcbnlngmoiindnogboollmbcidmalmogbogldmadmbjddnbcidmalmpoiehkdnelgechlncnlcbnlnogbogldmadmbjddncnlcbnlnpoiehkdnelgechlnbcidmalmaabaolkgjkjjjjdnbcidmalmcedjfjdncedjfjlncnlcbnlnaabaolkgjkjjjjdncnlcbnlncedjfjdncedjfjlnbcidmalmogbogllmadmbjddnbcidmalmelgechdnpoiehklncnlcbnlnogbogllmadmbjddncnlcbnlnelgechdnpoiehklnbcidmalmogboollmgmoiindnbcidmalmogbooldmgmoiinlncnlcbnlnogboollmgmoiindncnlcbnlnogbooldmgmoiinlnbcidmalmelgechlnpoiehkdnbcidmalmogbogldmadmbjdlncnlcbnlnelgechlnpoiehkdncnlcbnlnogbogldmadmbjdlnbcidmalmcedjfjlncedjfjdnbcidmalmaabaolkgjkjjjjlncnlcbnlncedjfjlncedjfjdncnlcbnlnaabaolkgjkjjjjlnbcidmalmpoiehklnelgechdnbcidmalmogbogllmadmbjdlncnlcbnlnpoiehklnelgechdnbcidmalmgmoiinlnogbooldmcnlcbnlnogbogllmadmbjdlncnlcbnlngmoiinlnogbooldmbcidmalmogboollmgmoiinlncnlcbnlnogbijclnanoajgdmcnlcbnlnogboollmgmoiinlnbcidmalmadmbjdlnogbogldmbcidmalmelgechlnpoiehklncnlcbnlnjkjjjjlnffhacncicnlcbnlnelgechlnpoiehklnbcidmalmjkjjjjlnffhacncibcidmalmcedjfjlncedjfjlncnlcbnlnadmbjdlnogbogllmcnlcbnlncedjfjlncedjfjlnbcidmalmogbijclnanoajglmbcidmalmpoiehklnelgechlncnlcbnlngmoiinlnogboollmcnlcbnlnpoiehklnelgechlnbcidmalmgmoiinlnogboollm");
	//mPrecision = precision;
	
	{
		auto crb = ModuleFileManager::LoadFileToBuffer("fastTest.bin");
		LoadFromString(std::tie(precision, mMeshVertexIndices, mMeshVertices), std::string{ (char*)crb->data(), crb->size() });
		mPrecision = precision;
		std::cout << "In vertice count: " << mMeshVertices.size() << std::endl;
		std::cout << "In triangle count: " << mMeshVertexIndices.size() / 3 << std::endl;
		//mConstructMesh = false;
		mConstructEnveloppe = false;
	}
	
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

	mPrecision.changeNotificationLevel(Owner);

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	MeshSimplifier::rebuildMesh()
{
	if (mMeshSimplification)
	{
		delete mMeshSimplification;
	}
	if (mMeshNode)
		mScene3D->removeItem(mMeshNode);

	auto timer=GetApplicationTimer();
	auto startTime = timer->GetTime();

	if (mTestGroup)
	{
		mMeshSimplification = new MeshSimplification(mMeshVertices, mPrecision, 10);

		std::size_t half_size = mMeshVertexIndices.size() / 3;
		half_size /= 2;
		half_size *= 3;

		std::vector<u32>	grp1(mMeshVertexIndices.begin(), mMeshVertexIndices.begin()+ half_size);
		std::vector<u32>	grp2(mMeshVertexIndices.begin() + half_size, mMeshVertexIndices.end() );

		mMeshSimplification->addTriangleGroup(grp1, 0);
		mMeshSimplification->addTriangleGroup(grp2, 1);

		mMeshSimplification->doSimplification();
	}
	else
	{
		mMeshSimplification = new MeshSimplification(mMeshVertexIndices, mMeshVertices, mPrecision, 10);
	}
	auto deltaTime = timer->GetTime() - startTime;

	std::cout << "simplification time :" << deltaTime << std::endl;

	std::cout << "out vertice count: " << mMeshSimplification->getVerticeCount() << std::endl;
	std::cout << "out triangle count: " << mMeshSimplification->getTriangleCount() << std::endl;
	if (mTestGroup)
	{
		std::cout << "grp 1 out triangle count: " << mMeshSimplification->getTriangleCount(0) << std::endl;
		std::cout << "grp 2 out triangle count: " << mMeshSimplification->getTriangleCount(1) << std::endl;
	}

	BBox tst = mMeshSimplification->getOctreeBoundingBox();
	mRecenterTranslate = -tst.Center(); 
	Matrix3x4 topos;
	topos.SetIdentity();
	topos.SetTranslation(mRecenterTranslate);
	mMeshNode = KigsCore::GetInstanceOf("MeshNode", "Node3D");
	if (mConstructMesh)
	{
		auto m = buildMesh(mMeshVertexIndices, mMeshSimplification->getOctreeCoordVertices(), "InOctreeCoordMesh");
		
		mMeshNode->addItem(m);
	
	}

	if (mTestGroup)
	{
		SmartPointer<ModernMesh> outmesh = buildMesh(mMeshSimplification->getFinalTriangles(0), mMeshSimplification->getFinalVertices(), "FinalMesh");
		mMeshNode->addItem(outmesh);
		outmesh = buildMesh(mMeshSimplification->getFinalTriangles(1), mMeshSimplification->getFinalVertices(), "FinalMesh", {0.5,1.0,0.5});
		mMeshNode->addItem(outmesh);
	}
	else
	{
		SmartPointer<ModernMesh> outmesh = buildMesh(mMeshSimplification->getFinalTriangles(), mMeshSimplification->getFinalVertices(), "FinalMesh");
		mMeshNode->addItem(outmesh);
	}

	mMeshNode->Init();
	mMeshNode->ChangeMatrix(topos);

	mScene3D->addItem(mMeshNode);

	if(mRootEnvNode)
		mScene3D->removeItem(mRootEnvNode);

	mRootEnvNode = KigsCore::GetInstanceOf("RootEnvNode", "Node3D");

	// enveloppe
	if (mConstructEnveloppe)
	{
		for (size_t i = 0; i < mMeshSimplification->getEnveloppeSize(); i++)
		{
			u32 flag = 0;
			v3i pos = mMeshSimplification->getEnveloppePos(i, flag);
			pos /= 2;

			SP<Node3D> EnvNode = KigsCore::GetInstanceOf("EnvNode_" + std::to_string(i), "Node3D");

			u32 debugflag = 0;
#ifdef _DEBUG
			debugflag = mMeshSimplification->getNodeDebugFlag(i);
#endif

			EnvNode->addItem(getCube(flag, debugflag));

			topos.SetTranslation(v3f(pos.x, pos.y, pos.z) + mRecenterTranslate);

			EnvNode->Init();
			EnvNode->ChangeMatrix(topos);

			mRootEnvNode->addItem(EnvNode);
		}
	}
	mScene3D->addItem(mRootEnvNode);

}


void	MeshSimplifier::ProtectedUpdate()
{
	if (mScene3D)
	{
		if (!mMeshNode)
		{
			rebuildMesh();

			mDebugCubeNode = KigsCore::GetInstanceOf("DebugCube", "Node3D");
			mDebugCubeNode->addItem(getCube(0xFF,0));

			mScene3D->addItem(mDebugCubeNode);
			moveDebugCube();
			setValue("ShowEdges", true);
			setValue("ShowObject", true);
			setValue("ShowEnveloppe", false);
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
	if(mMeshSimplification)
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
#ifdef _DEBUG
	auto vertices = mMeshSimplification->getEnveloppeVertices();
	for (size_t i = 0; i < vertices.size(); i++)
	{
		auto& v = vertices[i].V;
		if (i < mDisplayVerticeCount)
		{
			float r = 0.05f;
			if (mSelectedVerticeIndex == i)
			{
				dd::sphere(v + mRecenterTranslate, v3f(1.0f,0.0f,0.0), r*3.0f);

				v3f ncolor(1.0f, 1.0f, 0.0f);
				ncolor /= vertices[i].N.size();
				v3f currentncolor = ncolor;

				for (auto n : vertices[i].N)
				{
					dd::sphere(n + mRecenterTranslate, currentncolor, r/2.0f);
					currentncolor += ncolor;
				}
			}
			dd::cross(v + mRecenterTranslate,r);
		}
	}
#endif
}

void	MeshSimplifier::drawEdges()
{
#ifdef _DEBUG
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
			dd::line(e.first.first + mRecenterTranslate, e.first.second + mRecenterTranslate, color);
		}
	}

	if((mSelectedEdgeIndex>=0) && (mSelectedEdgeIndex< edgeList.size()))
	{
		auto& e = edgeList[mSelectedEdgeIndex];
		v3f color(1.0, 0.3, 0.0);
		dd::line(e.first.first + mRecenterTranslate, e.first.second + mRecenterTranslate, color,0,false);
	}
#endif
}

void MeshSimplifier::showEnveloppe(bool show)
{
	if (mConstructEnveloppe)
	{
		for (size_t i = 0; i < mMeshSimplification->getEnveloppeSize(); i++)
		{
			SP<Node3D> EnvNode = mRootEnvNode->GetFirstSonByName("Node3D", "EnvNode_" + std::to_string(i));

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
	else if (labelid == mPrecision.getID())
	{
		rebuildMesh();
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
