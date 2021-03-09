#include <testXLSX.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>
#include "XLSXDocument.h"

using namespace std;
IMPLEMENT_CLASS_INFO(testXLSX);

IMPLEMENT_CONSTRUCTOR(testXLSX)
{

}

void	testXLSX::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager>& pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");


	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();

	XLSXDocument	doc;

	if (doc.open("tst.xlsx"))
	{

	}

}

void	testXLSX::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();
}

void	testXLSX::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	testXLSX::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{

	}
}
void	testXLSX::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		
	}
}

