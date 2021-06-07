#include "Board.h"
#include "JSonFileParser.h"
#include "Ghost.h"
#include "Player.h"
#include "Core.h"
#include "CoreBaseApplication.h"

std::string ghostNames[4] = { "inky","clyde","pinky","blinky" };

Case				Board::mErrorCase;
#define tileSize 25.0f



Board::Board(const std::string& filename, SP<UIItem> minterface) : mParentInterface(minterface)
{
	JSonFileParser L_JsonParser;
	CoreItemSP initP = L_JsonParser.Get_JsonDictionary(filename);

	mBoardSize = (v2f)initP["Size"];

	if ((mBoardSize.y > 0) && (mBoardSize.x > 0))
	{
		CoreItemSP cases = initP["Cases"];
		size_t caseIndex = 0;
		mCases.resize(mBoardSize.y);
		for (int i = 0; i < mBoardSize.y; i++)
		{
			mCases[i].resize(mBoardSize.x);
			for (int j = 0; j < mBoardSize.x; j++)
			{
				mCases[i][j].setInitType((int)cases[caseIndex]);

				if ((int)cases[caseIndex] == 3)
				{
					mGhostAppearPos.push_back({ j,i });
				}

				caseIndex++;
			}
		}
	}

	mScreenSize = mParentInterface->getValue<v2f>("Size");
}

void	Board::InitGhosts()
{
	// Init Ghosts
	for (int i = 0; i < 4; i++)
	{
		mGhosts.push_back(KigsCore::GetInstanceOf("gg", "Ghost"));
		mGhosts.back()->setBoard(this);
		mGhosts.back()->setValue("Name", ghostNames[i]);
		mGhosts.back()->Init();
	}
}

void	Board::InitPlayer()
{
	mPlayer= KigsCore::GetInstanceOf("player", "Player");
	mPlayer->setBoard(this);
	mPlayer->Init();
}


void	Board::Update()
{
	const auto& t=KigsCore::GetCoreApplication()->GetApplicationTimer();
	for (auto g : mGhosts)
		g->CallUpdate(*t.get(),nullptr);

	mPlayer->CallUpdate(*t.get(), nullptr);
}

void	Board::initGraphicBoard()
{
	mLabyBG = KigsCore::GetInstanceOf("bg", "UIItem");
	mLabyBG->setValue("Size", mScreenSize);
	mLabyBG->setValue("Priority", 10);

	v2i labySize = getBoardSize();

	for (int i = 0; i < labySize.y; i++)
	{
		for (int j = 0; j < labySize.x; j++)
		{
			Case& currentC = getCase({ j,i });
			SP<UIItem>	uicase = nullptr;
			switch (currentC.getType())
			{
			case 1:
			case 3:
			{
				uicase = KigsCore::GetInstanceOf("case", "UIPanel");
				uicase->setValue("Anchor", v2f(0.5f, 0.5f));
				uicase->setValue("Size", v2f(tileSize * 0.9f, tileSize * 0.9f));
				uicase->setValue("Dock", v2f(0.5 + (float)(j - labySize.x / 2) * (tileSize / mScreenSize.x), 0.5 + (float)(i - labySize.y / 2) * (tileSize / mScreenSize.y)));
				uicase->setValue("Priority", 20);
				uicase->setValue("Color", v3f(0.0, 0.0, 1.0));
				uicase->setValue("Opacity", (currentC.getType() == 1) ? 1.0 : 0.5);
				mLabyBG->addItem(uicase);
				uicase->Init();
			}
			break;
			case 2:
			{
				uicase = KigsCore::GetInstanceOf("pastille", "UIImage");
				uicase->setValue("TextureName", "Pacman.json:dot");
				uicase->setValue("Anchor", v2f(0.5f, 0.5f));
				uicase->setValue("ForceNearest", true);
				uicase->setValue("Size", v2f(tileSize * 2.0f, tileSize * 2.0f));
				uicase->setValue("Dock", v2f(0.5 + (float)(j - labySize.x / 2) * (tileSize / mScreenSize.x), 0.5 + (float)(i - labySize.y / 2) * (tileSize / mScreenSize.y)));
				uicase->setValue("Priority", 3);
				mLabyBG->addItem(uicase);
				uicase->Init();
			}
			break;
			case 4:
			{
				uicase = KigsCore::GetInstanceOf("pomme", "UIImage");
				uicase->setValue("TextureName", "Pacman.json:apple");
				uicase->setValue("Anchor", v2f(0.5f, 0.5f));
				uicase->setValue("ForceNearest", true);
				uicase->setValue("Size", v2f(tileSize, tileSize));
				uicase->setValue("Dock", v2f(0.5 + (float)(j - labySize.x / 2) * (tileSize / mScreenSize.x), 0.5 + (float)(i - labySize.y / 2) * (tileSize / mScreenSize.y)));
				uicase->setValue("Priority", 3);
				mLabyBG->addItem(uicase);
				uicase->Init();
			}
			break;
			}
			if (uicase)
			{
				currentC.setGraphicRepresentation(uicase);
			}
		}
	}
	mParentInterface->addItem(mLabyBG);
	mLabyBG->Init();
}


v2i		Board::getAppearPosForGhost()
{
	v2i foundpos(0, 0);
	bool found = false;

	while (!found)
	{
		foundpos=mGhostAppearPos[rand() % (mGhostAppearPos.size())];
		if (!checkForGhostOnCase(foundpos))
		{
			found = true;
		}
	}

	return foundpos;
}

v2f		Board::convertBoardPosToDock(const v2f& p)
{
	v2i labySize = getBoardSize();

	v2f result(0.5f + (p.x - (float)(labySize.x / 2)) * (tileSize / mScreenSize.x), 0.5f + (p.y - (float)(labySize.y / 2)) * (tileSize / mScreenSize.y));

	return result;
}

bool	Board::checkForGhostOnCase(const v2i& pos,const Ghost* me)
{
	for (auto g : mGhosts)
	{
		if (!g->isDead())
		{
			if (g.get() != me)
			{
				v2f gpos = g->getCurrentPos();
				v2i igpos(round(gpos.x), round(gpos.y));
				if (pos == igpos)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool	Board::checkForWallOnCase(const v2i& pos)
{
	if (mCases[pos.y][pos.x].getType() == 1)
		return true;

	return false;
}

bool	Board::checkColumnVisibility(int x, int y1, int y2)
{
	if (y1 > y2)
	{
		int swp = y1;
		y1 = y2;
		y2 = swp;
	}
	for (int y = y1; y <= y2; y++)
	{
		if (mCases[y][x].getType() == 1)
		{
			return false;
		}
	}

	return true;
}

bool	Board::checkRowVisibility(int y, int x1, int x2)
{
	if (x1 > x2)
	{
		int swp = x1;
		x1 = x2;
		x2 = swp;
	}

	for (int x = x1; x <= x2; x++)
	{
		if (mCases[y][x].getType() == 1)
		{
			return false;
		}
	}

	return true;
}

v2i	Board::ghostSeePacman(const v2i& pos)
{
	auto poses = mPlayer->getPoses();

	for (const auto& p : poses)// first check if on the same line or column
	{
		if (p.x == pos.x)
		{
			if (checkColumnVisibility(p.x, p.y, pos.y))
			{
				return p;
			}
		}
		if (p.y == pos.y)
		{
			if (checkRowVisibility(p.y, p.x, pos.x))
			{
				return p;
			}
		}
	}

	return {-1,-1};
}

void	Board::checkEat(const v2i& pos)
{
	if (mCases[pos.y][pos.x].getType() == 2)
	{
		mLabyBG->removeItem(mCases[pos.y][pos.x].getGraphicRepresentation());
		mCases[pos.y][pos.x].setType(0);
		mCases[pos.y][pos.x].setGraphicRepresentation(nullptr);
	}
}


std::vector<bool> Board::getAvailableDirection(const v2i& pos)
{
	std::vector<bool> result;
	result.reserve(4);
	
	for (int direction = 0; direction < 4; direction++)
	{
		v2i destPos = pos + movesVector[direction];
		bool isOk = false;
		if ((destPos.x >= 0) && (destPos.x < mBoardSize.x))
		{
			if ((destPos.y >= 0) && (destPos.y < mBoardSize.y))
			{
				if (!checkForWallOnCase(destPos))
				{
					isOk = true;
				}
			}
		}
		result.push_back(isOk);
	}
	return result;
}