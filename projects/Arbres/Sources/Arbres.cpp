#include "Arbres.h"
#include "FilePathManager.h"
#include "JSonFileParser.h"
#include "UI/UIItem.h"
#include "AI.h"
#include "ModuleThread.h"
#include "Thread.h"

volatile bool	Arbres::mIsLocked=true;

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
	drawLabyrinthe();
	
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

		showHideControls(false);
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

void	Arbres::generateLabyrinthe()
{

	mLabyrintheSize.Set(29, 19);
	mLabyrinthe = new Case * [mLabyrintheSize.y];
	for (int i = 0; i < mLabyrintheSize.y; i++)
	{
		mLabyrinthe[i] = new Case[mLabyrintheSize.x]; 
		for (int j = 0; j < mLabyrintheSize.x; j++)
		{
			// only isolated cells to begin
			if ((i & 1) && (j & 1))
			{
				mLabyrinthe[i][j].setType(Case::CaseType::Slab);
			}
			else
			{
				mLabyrinthe[i][j].setType(Case::CaseType::Wall);
			}
		}
	}

	struct wallstruct
	{
		Case*	mCase;
		v2i		mPos;
		bool	mHorizontal;
	};

	std::vector<wallstruct>	wallList;
	v2i startpos(1 + ((rand()*2) % (mLabyrintheSize.x - 1)), 1 + ((rand()*2) % (mLabyrintheSize.y - 1)));
	mLabyrinthe[startpos.y][startpos.x].setVisit(true);
	wallList.push_back({ &mLabyrinthe[startpos.y - 1][startpos.x],{startpos.x,startpos.y - 1},true });
	mLabyrinthe[startpos.y - 1][startpos.x].setVisit(true);
	wallList.push_back({ &mLabyrinthe[startpos.y][startpos.x + 1],{startpos.x + 1,startpos.y},false });
	mLabyrinthe[startpos.y][startpos.x + 1].setVisit(true);
	wallList.push_back({ &mLabyrinthe[startpos.y + 1][startpos.x],{startpos.x,startpos.y + 1},true });
	mLabyrinthe[startpos.y + 1][startpos.x].setVisit(true);
	wallList.push_back({ &mLabyrinthe[startpos.y][startpos.x - 1],{startpos.x - 1,startpos.y},false });
	mLabyrinthe[startpos.y][startpos.x - 1].setVisit(true);

	v2i	deltapos[2][2] = { { {0,-1} , {0,1} } , { {-1,0} , {1,0} } };

	while (wallList.size())
	{
		int randWall = rand() % wallList.size();
		wallstruct currentwall = wallList[randWall];
		size_t countvisited = 0;
		for (size_t i = 0; i < 2; i++)
		{
			v2i tstpos = currentwall.mPos + deltapos[currentwall.mHorizontal ? 0 : 1][i];

			// check tstpos is inside labyrinthe
			if ((tstpos.x > 0) && (tstpos.x < (mLabyrintheSize.x - 1)) && (tstpos.y > 0) && (tstpos.y < (mLabyrintheSize.y - 1)))
			{
				if (mLabyrinthe[tstpos.y][tstpos.x].getType() == Case::CaseType::Slab)
				{
					countvisited += mLabyrinthe[tstpos.y][tstpos.x].isVisit() ? 1 : 0;
				}
			}

		}

		auto addwall = [&](v2i pos) {
			for (size_t dx = 0; dx < 2; dx++)
			{
				for (size_t dy = 0; dy < 2; dy++)
				{
					v2i wallpos = pos + deltapos[dy][dx];

					// check wallpos is inside labyrinthe
					if ((wallpos.x > 0) && (wallpos.x < (mLabyrintheSize.x - 1)) && (wallpos.y > 0) && (wallpos.y < (mLabyrintheSize.y - 1)))
					{
						if (mLabyrinthe[wallpos.y][wallpos.x].getType() == Case::CaseType::Wall)
						{
							if (!mLabyrinthe[wallpos.y][wallpos.x].isVisit())
							{
								wallList.push_back({ &mLabyrinthe[wallpos.y][wallpos.x],{wallpos.x,wallpos.y},dy==0 });
								mLabyrinthe[wallpos.y][wallpos.x].setVisit(true);
							}
						}
					}
				}
			}
		
		};

		if (countvisited == 1)
		{
			currentwall.mCase->setType(Case::CaseType::Slab);
			for (size_t i = 0; i < 2; i++)
			{
				v2i tstpos = currentwall.mPos + deltapos[currentwall.mHorizontal ? 0 : 1][i];

				// check tstpos is inside labyrinthe
				if ((tstpos.x > 0) && (tstpos.x < (mLabyrintheSize.x - 1)) && (tstpos.y > 0) && (tstpos.y < (mLabyrintheSize.y - 1)))
				{
					if (mLabyrinthe[tstpos.y][tstpos.x].getType() == Case::CaseType::Slab)
					{
						if (!mLabyrinthe[tstpos.y][tstpos.x].isVisit())
						{
							mLabyrinthe[tstpos.y][tstpos.x].setVisit(true);
							// add walls
							addwall(tstpos);
						}
					}
				}
			}
		}

		wallList.erase(wallList.begin() + randWall);
	}

	// set start and end pos
	bool setpos = false;
	while (!setpos)
	{
		int posy = rand() % mLabyrintheSize.y;
		if (mLabyrinthe[posy][1].getType() == Case::CaseType::Slab)
		{
			mLabyrinthe[posy][0].setType(Case::CaseType::Start);
			mStartPos.Set(0, posy);
			setpos = true;
		}
	}
	setpos = false;
	while (!setpos)
	{
		int posy = rand() % mLabyrintheSize.y;
		if (mLabyrinthe[posy][mLabyrintheSize.x-2].getType() == Case::CaseType::Slab)
		{
			mLabyrinthe[posy][mLabyrintheSize.x - 1].setType(Case::CaseType::Exit);
			mExitPos.Set(mLabyrintheSize.x - 1, posy);
			setpos = true;
		}
	}
}
void	Arbres::loadLabyrinthe()
{
	JSonFileParser L_JsonParser;
	CoreItemSP initP = L_JsonParser.Get_JsonDictionary("laby.json");

	mLabyrintheSize = (v2f)initP["Size"];

	if ((mLabyrintheSize.y > 0) && (mLabyrintheSize.x > 0))
	{
		CoreItemSP cases = initP["Cases"];
		size_t caseIndex = 0;
		mLabyrinthe = new Case * [mLabyrintheSize.y];
		for (int i = 0; i < mLabyrintheSize.y; i++)
		{
			mLabyrinthe[i] = new Case[mLabyrintheSize.x];
			for (int j = 0; j < mLabyrintheSize.x; j++)
			{
				mLabyrinthe[i][j].setType((Case::CaseType)(int)cases[caseIndex]);
				caseIndex++;
			}
		}
	}
}
void	Arbres::initLabyrinthe()
{
	
	size_t caseIndex = 0;
	for (int i = 0; i < mLabyrintheSize.y; i++)
	{
		for (int j = 0; j < mLabyrintheSize.x; j++)
		{
				
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
			mGraphicCases.push_back(uicase);
			caseIndex++;
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

	for (auto g : mGraphicCases)
	{
		mMainInterface->removeItem(g);
	}

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
		mMainInterface["step"]("IsTouchable") = true;
		mMainInterface["play"]("Text") = "Play";
		Downgrade("TickerUpgrador");
	}
	else
	{
		mMainInterface["step"]("IsHidden") = true;
		mMainInterface["step"]("IsTouchable") = false;
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
	delete mAI;
	mAI = nullptr;
	mIsPlaying = true;
	play();
	showHideControls(false);
	showHideAI(true);
}

template<typename ai>
void	Arbres::setupAI()
{
	showHideControls(true);
	showHideAI(false);
	clearLabyrinthe(); // just in case
	generateLabyrinthe();
	//loadLabyrinthe();
	initLabyrinthe();
	clearVisited();
	mPathFound = false;
	mAI = new ai(&mLabyrinthe[mStartPos.y][mStartPos.x]);

	clearVisited();
	SP<Thread> aithread = KigsCore::GetInstanceOf("aithread", "Thread");
	mThread = aithread;
	aithread->setMethod(this, "launchAI");
	aithread->Init();
}

void	Arbres::firstfound()
{
	setupAI<FirstFound>();
}
void	Arbres::bestfound()
{
	setupAI<BestFound>();
}

void	Arbres::dijkstra()
{
	setupAI< Dijkstra>();
}

void	Arbres::showHideControls(bool show)
{
	mMainInterface["step"]("IsHidden") = !show;
	mMainInterface["step"]("IsTouchable") = show;
	mMainInterface["play"]("IsHidden") = !show;
	mMainInterface["play"]("IsTouchable") = show;
}

void	Arbres::showHideAI(bool show)
{
	mMainInterface["FirstFound"]("IsHidden") = !show;
	mMainInterface["FirstFound"]("IsTouchable") = show;
	mMainInterface["BestFound"]("IsHidden") = !show;
	mMainInterface["BestFound"]("IsTouchable") = show;
	mMainInterface["Dijkstra"]("IsHidden") = !show;
	mMainInterface["Dijkstra"]("IsTouchable") = show;
}