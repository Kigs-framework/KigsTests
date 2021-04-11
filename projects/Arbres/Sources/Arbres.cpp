#include "Arbres.h"
#include "FilePathManager.h"
#include "JSonFileParser.h"
#include "UI/UIItem.h"
#include "AI.h"
#include "ModuleThread.h"
#include "Thread.h"

bool	Arbres::mIsLocked=true;

IMPLEMENT_CLASS_INFO(Arbres);

IMPLEMENT_CONSTRUCTOR(Arbres)
{

}

#define tileSize 25.0f

void	Arbres::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager>& pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");

	CoreCreateModule(ModuleThread, nullptr);

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	Arbres::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();

	if (mAI)
	{
		drawLabyrinthe();
	}
}

void	Arbres::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	Arbres::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mMainInterface = GetFirstInstanceByName("UIItem", "Interface");
		clearLabyrinthe(); // just in case
		initLabyrinthe();
		mAI = new recursiveBestFound(&mLabyrinthe[mStartPos.y][mStartPos.x], mStartPos);
		clearVisited();

		SP<Thread> aithread = KigsCore::GetInstanceOf("aithread", "Thread");
		mThread = aithread;
		aithread->setMethod(this, "launchAI");
		aithread->Init();
	}
}
void	Arbres::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		Downgrade("TickerUpgrador");
		clearLabyrinthe();
		mMainInterface = nullptr;
	}
}

void	Arbres::initLabyrinthe()
{
	JSonFileParser L_JsonParser;
	CoreItemSP initP = L_JsonParser.Get_JsonDictionary("laby.json");

	mLabyrintheSize= (v2f)initP["Size"];

	if ((mLabyrintheSize.y>0) && (mLabyrintheSize.x>0))
	{
		CoreItemSP cases= initP["Cases"];
		size_t caseIndex = 0;
		mLabyrinthe = new Case * [mLabyrintheSize.y];
		for (int i = 0; i < mLabyrintheSize.y; i++)
		{
			mLabyrinthe[i] = new Case[mLabyrintheSize.x];
			for (int j = 0; j < mLabyrintheSize.x; j++)
			{
				mLabyrinthe[i][j].setType((Case::CaseType)(int)cases[caseIndex]);

				SP<UIItem>	uicase = KigsCore::GetInstanceOf("case", "UIPanel");
				uicase->setValue("SizeX", tileSize);
				uicase->setValue("SizeY", tileSize);
				uicase->setValue("Dock", v2f(0.5+(float)(j- mLabyrintheSize.x /2)*(tileSize /1280.0f),0.5 + (float)(i- mLabyrintheSize.y/2) * (tileSize /800.0f)));
				uicase->setValue("Priority", 20);

				// add text if needed

				if (mLabyrinthe[i][j].getType() != Case::CaseType::Wall)
				{
					SP<UIItem>	uitext = KigsCore::GetInstanceOf("caset", "UIText");
					uitext->setValue("SizeX", -1.0f);
					uitext->setValue("SizeY", -1.0f);
					uitext->setValue("Priority", 25);
					uitext->setValue("Text", "");
					uitext->setValue("Color", v3f( 0.0, 0.0, 0.0 ));
					uitext->setValue("Font", "Calibri.ttf");
					uitext->setValue("FontSize", 24);
					uitext->setValue("MaxWidth", 64);
					uicase->addItem(uitext);
					uitext->Init();
				}

				mMainInterface->addItem(uicase);
				uicase->Init();
				mGraphicCases.push_back(uicase.get());
				caseIndex++;
			}
		}
	}

	// init neighbors

	v2i	neighbors[4] = { {0,-1},{1,0},{0,1},{-1,0} };

	for (int i = 0; i < mLabyrintheSize.y; i++)
	{
		for (int j = 0; j < mLabyrintheSize.x; j++)
		{
			if (mLabyrinthe[i][j].getType() != Case::Wall)
			{
				if (mLabyrinthe[i][j].getType() == Case::Start)
				{
					mStartPos.Set(j, i);
				}
				if (mLabyrinthe[i][j].getType() == Case::Exit)
				{
					mExitPos.Set(j, i);
				}
				for (int dir = 0; dir < 4; dir++)
				{
					v2i pos = neighbors[dir];
					pos.x += j;
					if ((pos.x >= 0) && (pos.x < mLabyrintheSize.x))
					{
						pos.y += i;
						if ((pos.y >= 0) && (pos.y < mLabyrintheSize.y))
						{
							if (mLabyrinthe[pos.y][pos.x].getType() != Case::Wall)
							{
								mLabyrinthe[i][j].addNeighbour((Case::NeighbourPos)dir, &(mLabyrinthe[pos.y][pos.x]));
							}
						}
					}
				}
			}
		}
	}
}

void	Arbres::drawLabyrinthe() // refresh visit colors
{
	if (mLabyrinthe)
	{
		v3f	color={0.4,0.04,0.04};

		if (mPathFound)
		{
			color.Set(0.5f + 0.3f * sinf(mApplicationTimer->GetTime() * 2.0f), 0.04, 0.04);
		}

		size_t caseIndex = 0;
		for (size_t i = 0; i < mLabyrintheSize.y; i++)
		{
			for (size_t j = 0; j < mLabyrintheSize.x; j++)
			{
				mGraphicCases[caseIndex]->setValue("Color", mLabyrinthe[i][j].isVisit()?color:mLabyrinthe[i][j].getColor());

				if (mLabyrinthe[i][j].getText() != "")
				{
					CMSP caset = mGraphicCases[caseIndex]->GetFirstSonByName("UIText", "caset");
					caset->setValue("Text", mLabyrinthe[i][j].getText());
				}

				caseIndex++;
			}
		}
	}
}

void	Arbres::clearVisited()
{
	for (size_t i = 0; i < mLabyrintheSize.y; i++)
	{
		for (size_t j = 0; j < mLabyrintheSize.x; j++)
		{
			mLabyrinthe[i][j].setVisit(false);
		}
	}
}

void	Arbres::clearLabyrinthe()
{
	if (mLabyrinthe)
	{
		for (size_t i = 0; i < mLabyrintheSize.y; i++)
		{
			delete[] mLabyrinthe[i];
		}
		delete[] mLabyrinthe;
	}
	mLabyrinthe = nullptr;
	mLabyrintheSize.Set(0.0, 0.0);

	mGraphicCases.clear();
}

void	Arbres::step()
{
	unlock();
}

void	Arbres::play()
{
	if (mIsPlaying)
	{
		mMainInterface["step"]("IsHidden") = false;
		mMainInterface["step"]("IsTouchable") = false;
		mMainInterface["play"]("Text") = "Play";
		Downgrade("TickerUpgrador");
	}
	else
	{
		mMainInterface["step"]("IsHidden") = true;
		mMainInterface["step"]("IsTouchable") = true;
		mMainInterface["play"]("Text") = "Pause";
		Upgrade("TickerUpgrador");
		setValue("TickerFrequency", 4.0f);
		setValue("TickerFunction", "step");
	}
	mIsPlaying = !mIsPlaying;
}

void	Arbres::launchAI()
{
	mPathFound=mAI->run();
	mThread = nullptr;
}