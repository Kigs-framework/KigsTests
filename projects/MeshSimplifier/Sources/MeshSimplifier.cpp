#include <MeshSimplifier.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>


IMPLEMENT_CLASS_INFO(MeshSimplifier);

IMPLEMENT_CONSTRUCTOR(MeshSimplifier)
{

}

void	MeshSimplifier::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");


	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	MeshSimplifier::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();
}

void	MeshSimplifier::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	MeshSimplifier::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{

	}
}
void	MeshSimplifier::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		
	}
}

