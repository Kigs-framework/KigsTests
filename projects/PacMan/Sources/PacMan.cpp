#include "PacMan.h"
#include "FilePathManager.h"
#include "NotificationCenter.h"
#include "GameLoop.h"
#include "Ghost.h"
#include "CoreFSM.h"

IMPLEMENT_CLASS_INFO(PacMan);

IMPLEMENT_CONSTRUCTOR(PacMan)
{

}

void	PacMan::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");

	initCoreFSM();

	DECLARE_FULL_CLASS_INFO(KigsCore::Instance(), Ghost, Ghost, PacMan);

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	PacMan::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();

	if (mGameLoop)
	{
		mGameLoop->update();
	}
}

void	PacMan::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	PacMan::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mMainInterface = GetFirstInstanceByName("UIItem", "Interface");
		mGameLoop = new GameLoop(mMainInterface);
	}
}
void	PacMan::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		delete mGameLoop;
		mGameLoop = nullptr;
		mMainInterface = nullptr;
	}
}

