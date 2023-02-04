#include "Arbres.h"
#include "FilePathManager.h"
#include "JSonFileParser.h"
#include "UI/UIItem.h"
#include "AI.h"
#include "ModuleThread.h"
#include "Thread.h"

using namespace Kigs;
using namespace Kigs::File;
using namespace Kigs::Draw2D;
using namespace Kigs::Thread;

//#define REALLY_SIMPLE

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

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
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
	if (mThread)
	{
		SP<Thread::Thread> toKill = mThread;
		toKill->Kill();
		mThread = nullptr;

	}
	CoreDestroyModule(ModuleThread);
	DataDrivenBaseApplication::ProtectedClose();
}

void	Arbres::ProtectedInitSequence(const std::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mMainInterface = GetFirstInstanceByName("UIItem", "Interface");

		showHideControls(false);
	}
}
void	Arbres::ProtectedCloseSequence(const std::string& sequence)
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

	// create Case Array
#ifdef REALLY_SIMPLE
	mLabyrintheSize.Set(7, 7);
#else
	mLabyrintheSize.Set(35, 25);
#endif
	mLabyrinthe = new Case * [mLabyrintheSize.y];
	for (int i = 0; i < mLabyrintheSize.y; i++)
	{
		mLabyrinthe[i] = new Case[mLabyrintheSize.x]; 
		for (int j = 0; j < mLabyrintheSize.x; j++)
		{
			Case::CaseType	ctype = Case::CaseType::Wall;
			// only isolated cells to begin
			if ((i & 1) && (j & 1))
			{
				ctype = Case::CaseType::Slab;
			}
			mLabyrinthe[i][j].setType(ctype);
		}
	}

	// init display, as this is threaded, we can follow the construction of the
	// maze step by step
	initGraphicLabyrinthe();
	mLabyrintheCanBeShow = true;

	// lambda to check if a pos is inside labyrinthe size
	auto checkInside = [this](const v2i& pos)->bool {
		if ((pos.x > 0) && (pos.x < (mLabyrintheSize.x - 1)) && (pos.y > 0) && (pos.y < (mLabyrintheSize.y - 1)))
		{
			return true;
		}
		return false;
	};

	// this struct is used only here, so we can define it locally
	struct wallstruct
	{
		Case*	mCase;
		v2i		mPos;
		bool	mHorizontal;
	};

	// list of walls to be checked for opening (replaced by slab)
	std::vector<wallstruct>	wallList;

	// list of possible directions to get neighbors
	v2i	deltapos[2][2] = { { {0,-1} , {0,1} } , { {-1,0} , {1,0} } };

	// lambda to add walls around the given case to the list
	auto addwalls = [&](const v2i& pos) {
		for (size_t dx = 0; dx < 2; dx++)
		{
			for (size_t dy = 0; dy < 2; dy++)
			{
				v2i wallpos = pos + deltapos[dy][dx];

				// check wallpos is inside labyrinthe
				if (checkInside(wallpos))
				{
					if (mLabyrinthe[wallpos.y][wallpos.x].getType() == Case::CaseType::Wall)
					{
						if (!mLabyrinthe[wallpos.y][wallpos.x].isVisit()) // this wall was not already processed ?
						{
							wallList.push_back({ &mLabyrinthe[wallpos.y][wallpos.x],{wallpos.x,wallpos.y},dy == 0 });
							mLabyrinthe[wallpos.y][wallpos.x].setVisit(true);
							mLabyrinthe[wallpos.y][wallpos.x].setText((dy == 0) ? "__" : " | ");
						}
					}
				}
			}
		}

	};

	// choose random start case
	
	v2i startpos(1 + ((rand()*2) % (mLabyrintheSize.x - 1)), 1 + ((rand()*2) % (mLabyrintheSize.y - 1)));
	mLabyrinthe[startpos.y][startpos.x].setVisit(true);
	// add walls to the "wall to process" list
	addwalls(startpos);

	// while the list is not empty
	while (wallList.size())
	{
		waitMainThread(); // this is for the multithread management, wait for step by step construction
		int randWall = rand() % wallList.size();
		wallstruct currentwall = wallList[randWall];
		size_t countvisited = 0;

		for (size_t i = 0; i < 2; i++) // a wall is horizontal or vertical, so only to slab around it 
		{
			v2i tstpos = currentwall.mPos + deltapos[currentwall.mHorizontal ? 0 : 1][i];

			// check tstpos is inside labyrinthe
			if (checkInside(tstpos))
			{
				if (mLabyrinthe[tstpos.y][tstpos.x].getType() == Case::CaseType::Slab)
				{
					countvisited += mLabyrinthe[tstpos.y][tstpos.x].isVisit() ? 1 : 0;
				}
			}
			else
			{
				countvisited = 0;
				break;
			}

		}

		if (countvisited == 1) // ok, this wall can be opened (changed to slab)
		{
			// change this wall to a slab
			currentwall.mCase->setType(Case::CaseType::Slab);
			
			for (size_t i = 0; i < 2; i++)
			{
				v2i tstpos = currentwall.mPos + deltapos[currentwall.mHorizontal ? 0 : 1][i];

				// check tstpos is inside labyrinthe
				if (checkInside(tstpos))
				{
					if (mLabyrinthe[tstpos.y][tstpos.x].getType() == Case::CaseType::Slab)
					{
						if (!mLabyrinthe[tstpos.y][tstpos.x].isVisit())
						{
							mLabyrinthe[tstpos.y][tstpos.x].setVisit(true);
							// add walls
							addwalls(tstpos);
						}
					}
				}
			}
		}
		currentwall.mCase->setText("");
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

void	Arbres::removeRandomWalls(u32 tryOpeningCount)
{
	for (size_t i = 0; i < tryOpeningCount; i++)
	{
		v2i randpos(1 + (rand() % (mLabyrintheSize.x - 2)), 1 + (rand() % (mLabyrintheSize.y - 2)));
		Case& current = mLabyrinthe[randpos.y][randpos.x];
		if (current.getType() == Case::CaseType::Wall)
		{
			current.setType(Case::CaseType::Slab);
		}
	}
}

void	Arbres::openMoreWallsLabyrinthe(u32 tryOpeningCount)
{
	v2i	deltapos[4] = {  {0,-1} , {1,0}  ,  {0,1} , {-1,0}  };
	for (size_t i = 0; i < tryOpeningCount; i++)
	{
		v2i randpos(1 + (rand() % (mLabyrintheSize.x - 2)), 1 + (rand() % (mLabyrintheSize.y - 2)));

		Case& current = mLabyrinthe[randpos.y][randpos.x];
		if (current.getType() == Case::CaseType::Wall)
		{
			bool isSlab[4];
			int slabCount = 0;
			for (size_t p = 0; p < 4; p++)
			{
				v2i neighborpos = randpos + deltapos[p];
				isSlab[p] = (mLabyrinthe[neighborpos.y][neighborpos.x].getType() == Case::CaseType::Slab);
				if (isSlab[p])
					slabCount++;
			}

			if (slabCount == 2)
			{
				bool horizontal = isSlab[0] && isSlab[2];
				bool vertical = isSlab[1] && isSlab[3];

				if (horizontal ^ vertical)
				{
					current.setType(Case::CaseType::Slab);
				}
			}
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

void	Arbres::initGraphicLabyrinthe()
{
	size_t caseIndex = 0;

	mLabyBG = KigsCore::GetInstanceOf("bg", "UIItem");
	mLabyBG->setValue("Size", v2f( 1280,800 ));
	mLabyBG->setValue("Priority", 10);

	for (int i = 0; i < mLabyrintheSize.y; i++)
	{
		for (int j = 0; j < mLabyrintheSize.x; j++)
		{

			SP<UIItem>	uicase = KigsCore::GetInstanceOf("case", "UIPanel");
			uicase->setValue("Size", v2f(tileSize, tileSize));
			uicase->setValue("Dock", v2f(0.5 + (float)(j - mLabyrintheSize.x / 2) * (tileSize / 1280.0f), 0.5 + (float)(i - mLabyrintheSize.y / 2) * (tileSize / 800.0f)));
			uicase->setValue("Priority", 20);

			
			SP<UIItem>	uitext = KigsCore::GetInstanceOf("caset", "UIText");
			uitext->setValue("Size", v2f(-1,-1));
			uitext->setValue("Priority", 25);
			uitext->setValue("Text", "");
			uitext->setValue("Color", v3f(0.0, 0.0, 0.0));
			uitext->setValue("Font", "Calibri.ttf");
			uitext->setValue("FontSize", 24);
			uitext->setValue("MaxWidth", 64);
			uicase->addItem(uitext);
			uitext->Init();
			

			mLabyBG->addItem(uicase);
			uicase->Init();
			mGraphicCases.push_back(uicase);
			caseIndex++;
		}
	}

	mMainInterface->addItem(mLabyBG);

}

void	Arbres::initLabyrinthe()
{
	
	
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
	if (mLabyrinthe && mLabyrintheCanBeShow)
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
				if(mLabyrinthe[i][j].getType() == Case::Slab)
					mGraphicCases[caseIndex]->setValue("Color", mLabyrinthe[i][j].isVisit()?color:mLabyrinthe[i][j].getColor());
				else
					mGraphicCases[caseIndex]->setValue("Color", mLabyrinthe[i][j].getColor());

				CMSP caset = mGraphicCases[caseIndex]->GetFirstSonByName("UIText", "caset");
				if (mLabyrinthe[i][j].getText() != caset->getValue<std::string>("Text"))
				{
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
	mPathFound = false;
	mLabyrintheCanBeShow = false;
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

	if(mLabyBG)
		mMainInterface->removeItem(mLabyBG);
	mLabyBG = nullptr;

	mGraphicCases.clear();
}

void	Arbres::step()
{
	unlock();
}

void Arbres::speedup()
{
	mCurrentFrequency *= 2.0f;
	setValue("TickerFrequency", mCurrentFrequency);
	if (mCurrentFrequency > 64.0f)
	{
		mMainInterface["speedUp"]("IsHidden") = true;
		mMainInterface["speedUp"]("IsTouchable") = false;
	}

	mMainInterface["speedDown"]("IsHidden") = false;
	mMainInterface["speedDown"]("IsTouchable") = true;
}

void Arbres::speeddown()
{
	mCurrentFrequency *= 0.5f;
	setValue("TickerFrequency", mCurrentFrequency);
	if (mCurrentFrequency < 8.0f)
	{
		mMainInterface["speedDown"]("IsHidden") = true;
		mMainInterface["speedDown"]("IsTouchable") = false;
	}

	mMainInterface["speedUp"]("IsHidden") = false;
	mMainInterface["speedUp"]("IsTouchable") = true;
}

void	Arbres::play()
{
	if (mIsPlaying)
	{
		mMainInterface["step"]("IsHidden") = false;
		mMainInterface["step"]("IsTouchable") = true;
		mMainInterface["play"]("Text") = "Play";
		mMainInterface["speedDown"]("IsHidden") = true;
		mMainInterface["speedDown"]("IsTouchable") = false;
		mMainInterface["speedUp"]("IsHidden") = true;
		mMainInterface["speedUp"]("IsTouchable") = false;
		Downgrade("TickerUpgrador");
	}
	else
	{
		mMainInterface["step"]("IsHidden") = true;
		mMainInterface["step"]("IsTouchable") = false;
		mMainInterface["play"]("Text") = "Pause";
		mMainInterface["speedDown"]("IsHidden") = true;
		mMainInterface["speedDown"]("IsTouchable") = false;
		mMainInterface["speedUp"]("IsHidden") = false;
		mMainInterface["speedUp"]("IsTouchable") = true;
		Upgrade("TickerUpgrador");
		mCurrentFrequency = 4.0f;
		setValue("TickerFrequency", mCurrentFrequency);
		setValue("TickerFunction", "step");
	}
	mIsPlaying = !mIsPlaying;
}

void	Arbres::launchAI()
{
	srand(time(nullptr)); // srand each thread
	mPathFound=mAI->run();
	mThread = nullptr;
	delete mAI;
	mAI = nullptr;
	mIsPlaying = true;
	play();
	showHideControls(false);
	showHideAI(true);
}

void	Arbres::setupAI()
{
	mPathFound = false;
	clearVisited();
	mAI->init(&mLabyrinthe[mStartPos.y][mStartPos.x], mStartPos, &mLabyrinthe[mExitPos.y][mExitPos.x], mExitPos);
	clearVisited();
	SP<Thread::Thread> aithread = KigsCore::GetInstanceOf("aithread", "Thread");
	mThread = aithread;
	aithread->setMethod(this->SharedFromThis(), "launchAI");
	aithread->Init();
}

void	Arbres::setupLabyrinthe(u32 tryOpeningCount,u32 removerandomwalls)
{
	showHideControls(true);
	showHideAI(false);
	clearLabyrinthe(); 
	mTryOpeningCount = tryOpeningCount;
	mRemoveRandomWalls = removerandomwalls;
	SP<Thread::Thread> labythread = KigsCore::GetInstanceOf("labythread", "Thread");
	mThread = labythread;
	labythread->setMethod(this->SharedFromThis(), "launchLaby");
	labythread->Init();
}

void	Arbres::launchLaby()
{
	srand(time(nullptr)); // srand each thread
	generateLabyrinthe();
	openMoreWallsLabyrinthe(mTryOpeningCount);
	removeRandomWalls(mRemoveRandomWalls);
	initLabyrinthe();
	clearVisited();
	mIsPlaying = true; // reset play system
	play();
	setupAI();
}

void	Arbres::firstfound()
{
	mAI = new FirstFound();
#ifdef REALLY_SIMPLE
	setupLabyrinthe(2, 0);
#else
	setupLabyrinthe(60,0);	
#endif
}
void	Arbres::bestfound()
{
	mAI = new BestFound();
	setupLabyrinthe(0,0);
}

void	Arbres::dijkstra()
{
	mAI = new Dijkstra();
	setupLabyrinthe(0,0);
}

void	Arbres::astar()
{
	mAI = new AStar();
#ifdef REALLY_SIMPLE
	setupLabyrinthe(2, 3);
#else
	setupLabyrinthe(80,800);
#endif
}

void	Arbres::showHideControls(bool show)
{
	mMainInterface["step"]("IsHidden") = !show;
	mMainInterface["step"]("IsTouchable") = show;
	mMainInterface["play"]("IsHidden") = !show;
	mMainInterface["play"]("IsTouchable") = show;

	mMainInterface["speedUp"]("IsHidden") = true;
	mMainInterface["speedUp"]("IsTouchable") = false;
	mMainInterface["speedDown"]("IsHidden") = true;
	mMainInterface["speedDown"]("IsTouchable") = false;

}

void	Arbres::showHideAI(bool show)
{
	mMainInterface["FirstFound"]("IsHidden") = !show;
	mMainInterface["FirstFound"]("IsTouchable") = show;
	mMainInterface["BestFound"]("IsHidden") = !show;
	mMainInterface["BestFound"]("IsTouchable") = show;
	mMainInterface["Dijkstra"]("IsHidden") = !show;
	mMainInterface["Dijkstra"]("IsTouchable") = show;
	mMainInterface["AStar"]("IsHidden") = !show;
	mMainInterface["AStar"]("IsTouchable") = show;
}