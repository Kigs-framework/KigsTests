#include <testFileManager.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>


IMPLEMENT_CLASS_INFO(testFileManager);

IMPLEMENT_CONSTRUCTOR(testFileManager)
{

}

void	testFileManager::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager>& pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");
	pathManager->AddToPath("Skin2", "xml");

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	testFileManager::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();
}

void	testFileManager::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	testFileManager::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{

	}
}
void	testFileManager::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		
	}
}

