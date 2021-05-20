#include <testPixelSprites.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>


IMPLEMENT_CLASS_INFO(testPixelSprites);

IMPLEMENT_CONSTRUCTOR(testPixelSprites)
{

}

void	testPixelSprites::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");


	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	testPixelSprites::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();
}

void	testPixelSprites::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	testPixelSprites::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{

	}
}
void	testPixelSprites::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		
	}
}

