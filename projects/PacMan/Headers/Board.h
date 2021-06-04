#pragma once
#include "Case.h"
#include <vector>
#include <string>
#include "CoreModifiable.h"
#include "UI/UIItem.h"

class Ghost;
class Player;

class Board
{
protected:

	std::vector<std::vector<Case>>	mCases;
	v2i								mBoardSize;
	static Case						mErrorCase;
	SP<UIItem>						mLabyBG;
	SP<UIItem>						mParentInterface;
	std::vector<v2i>				mGhostAppearPos;

	v2f								mScreenSize;

	std::vector<SP<Ghost>>			mGhosts;
	SP<Player>						mPlayer;

public:

	Board(const std::string& filename,SP<UIItem> minterface);

	const v2i& getBoardSize()
	{
		return mBoardSize;
	}

	Case& getCase(const v2i& pos)
	{
		if ((pos.x >= 0) && (pos.x < mBoardSize.x))
		{
			if ((pos.y >= 0) && (pos.y < mBoardSize.y))
			{
				return mCases[pos.y][pos.x];
			}
		}

		return mErrorCase;
	}

	void	initGraphicBoard();

	v2i		getAppearPosForGhost();


	SP<UIItem>	getGraphicInterface()
	{
		return mParentInterface;
	}

	v2f		convertBoardPosToDock(const v2f& p);

	void							InitGhosts();
	void							InitPlayer();

	bool	checkForGhostOnCase(const v2i& pos,const Ghost* me=nullptr);
	bool	checkForWallOnCase(const v2i& pos);

	std::vector<bool>	getAvailableDirection(const v2i& pos);

	void	Update();
};
