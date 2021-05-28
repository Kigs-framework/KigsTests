#include "testFileManager.h"
#include "FilePathManager.h"
#include "ModuleFileManager.h"
#include "NotificationCenter.h"
#include "CorePackage.h"
#include <iostream>

IMPLEMENT_CLASS_INFO(testFileManager);

IMPLEMENT_CONSTRUCTOR(testFileManager)
{

}

// simple function to load a file and cout its content
void	PrintFileContent(const std::string& filename)
{
	std::cout << std::endl;
	u64 flen;
	SP<CoreRawBuffer> txtfile = ModuleFileManager::LoadFileAsCharString(filename.c_str(), flen, sizeof(char));
	if (txtfile)
	{
		std::string content = txtfile->buffer();
		std::cout << "Content of file :" << filename << std::endl;
		std::cout << content << std::endl;
	}
	else
	{
		std::cout << "file :" << filename << " not found" << std::endl;
	}
	std::cout << std::endl;
}

void	testFileManager::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	// AppInit.xml ask application to setup a bundle file with this line :
	// 	<Attr Type="string" N="BundleFileName" V="files.bundle" Dyn="true"/>

	// get FilePathManager singleton
	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	// search xml in root directory
	pathManager->AddToPath(".", "xml");
	// and in Skin2 directory
	// as Screen_Main.xml is present in both Skin1 & Skin2, adding Skin2 to the path for xml files => the Skin2 version of Screen_Main.xml will be loaded 
	pathManager->AddToPath("Skin2", "xml");
	// ask pathManager to search files preferably in Level1 folder 
	pathManager->AddToPath("Level2", "*");


	// check that a file can be found
	auto f=pathManager->FindFullName("FileNotFound.txt");
	// FileNotFound.txt is in Skin3 folder but Skin3 folder was not included in "files.bundle"
	// and not AddToPath so it will not be found
	if (f->mStatus & FileHandle::Exist)
	{
		PrintFileContent("FileNotFound.txt");
	}

	// load a package ( the package is not loaded in memory )
	if (pathManager->LoadPackage("someAssets.kpkg"))
	{
		// some fun with the loaded package
		CorePackage* package = pathManager->GetLoadedPackage("someAssets.kpkg");

		// print all files name (with path) in the package 
		std::cout << "Package someAssets.kpkg contains : " << std::endl;

		// use a lambda just to print file name
		package->IterateFiles([](const std::string& filepathname) {
			
			std::cout << filepathname << std::endl;

			});

		// two LevelFile.txt files are available in kpkg in Level1 and Level2 folders
		// as Level2 was added to path for all extensions, LevelFile.txt will be found in Level2 folder
		PrintFileContent("LevelFile.txt");
		// remove Level2 from path
		pathManager->RemoveFromPath("Level2");
		// and add Level1 
		pathManager->AddToPath("Level1"); // default parameter is "*" => all extensions
		// this time, LevelFile will be loaded from Level1 folder
		PrintFileContent("LevelFile.txt");

		// now unload package
		pathManager->UnloadPackage("someAssets.kpkg");

		// file LevelFile.txt will not be found anymore
		PrintFileContent("LevelFile.txt");

	}

	// "classic" fopen/fread/fclose... like functions are available
	// giving full path, this time we will find FileNotFound.txt
	auto file = Platform_fopen("Skin3/FileNotFound.txt", "rb");
	if (file->mFile)
	{
		char readbuffer[100];
		int readsize=Platform_fread(readbuffer, 1, 100, file.get());
		readbuffer[readsize] = 0;

		std::cout << "content read from Skin3/FileNotFound.txt : " << readbuffer << std::endl;

		Platform_fclose(file.get());
	}

	// access to some special folder : 
	file = Platform_fopen(FilePathManager::DevicePath("KigsTestFileManagerSave.txt", FilePathManager::DOCUMENT_FOLDER).c_str(), "wb");
	if (file->mFile)
	{
		std::string towrite = "write in the document directory";
		int readsize = Platform_fwrite(towrite.c_str(), 1, towrite.length(), file.get());
	
		Platform_fclose(file.get());
	}

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

