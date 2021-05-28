#include "testXLSX.h"
#include "FilePathManager.h"
#include "NotificationCenter.h"
#include "ModuleFileManager.h"
#include "XLSXDocument.h"
#include <iostream>
#include <regex>

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

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");


	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();

	XLSXDocument	doc;
	doc.initEmpty();
	doc.addSheet("first");

	auto s1 = doc[0]; // get first sheet
	XLSXSheet* sheet = s1.sheet();
	sheet->setRange("A1:C3");
	*doc[0]["A1"]=12;
	*doc[0]["B3"] = "houla";
	*doc[0]["C2"] = "houla";

	SP<CoreRawBuffer> saved = doc.save();
	if (saved)
	{
		ModuleFileManager::SaveFile("export.xlsx", (u8*)saved->buffer(), saved->size());
	}


	
	// XLSX doc
	/*XLSXDocument	doc;

	// open it
	if (doc.open("tst.xlsx"))
	{
		// get value as string of the cell L6 of sheet 0 
		std::string test = *doc[0]["L6"];

		// get sheet 0, then line 15, then cell K (on line 15)
		test = *doc[0]["15"]["K"];

		// get cell i26 on sheet 1
		auto cell = doc[1]["I26"];
		
		// then get full row
		auto row = cell.row();

		// iterate row and get content of each cell
		for (auto c : row)
		{
			test = c;
		}

		// get col and ierate
		auto col = cell.col();
		for (auto c : col)
		{
			test = c;
		}

		// get all cells with a value containing "NEXT-BIM" 
		std::vector<XLSXElementRef> search = doc.find("NEXT-BIM");
		// get all cells with a value of 450
		search = doc.find(450);

		// get the archive
		CoreRawBuffer* saved = doc.save();
		if (saved)
		{
			ModuleFileManager::SaveFile("export.xlsx", (u8*)saved->buffer(), saved->size());
			saved->Destroy();
		}
	}
	*/

	/*	const std::string fnames[] = { "foo.txt", "bar.txt", "baz.dat", "zoidberg" };
		const std::regex txt_regex("[a-z]+\\.txt");

		for (const auto& fname : fnames) {
			std::cout << fname << ": " << std::regex_match(fname, txt_regex) << '\n';
		}*/
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

