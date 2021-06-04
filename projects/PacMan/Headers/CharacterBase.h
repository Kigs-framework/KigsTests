#pragma once

#include "CoreModifiable.h"
#include "UI/UIItem.h"

class Board;

const v2i	movesVector[4] = { {1,0},{0,1},{-1,0},{0,-1} };

class CharacterBase : public CoreModifiable
{
public:
	DECLARE_ABSTRACT_CLASS_INFO(CharacterBase, CoreModifiable, PacMan);
	DECLARE_CONSTRUCTOR(CharacterBase);

	void	setCurrentPos(const v2f& pos);

	const v2f& getCurrentPos()
	{
		return mCurrentPos;
	}

	void	setBoard(Board* b)
	{
		mBoard = b;
	}

	Board* getBoard()
	{
		return mBoard;
	}

	void	setGraphicRepresentation(CMSP rep)
	{
		mGraphicRepresentation = rep;
	}

	CMSP getGraphicRepresentation()
	{
		return mGraphicRepresentation;
	}

	bool	isDead()
	{
		return mIsDead;
	}

	v2i getRoundPos()
	{
		return v2i(round(mCurrentPos.x), round(mCurrentPos.y));
	}

protected:

	v2f			mCurrentPos;
	Board*		mBoard = nullptr;
	SP<UIItem>	mGraphicRepresentation;

	bool		mIsDead = false;
	// -1 for no move direction, 
	int			mDirection=-1;
	v2i			mDestPos;

	float		mSpeed = 3.0f;
};
