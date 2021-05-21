#pragma once

#include "CoreModifiable.h"
#include "Board.h"

class GameLoop
{
protected:

	CMSP	mMainInterface;
	Board* mBoard = nullptr;

public:

	GameLoop(CMSP linterface) :mMainInterface(linterface)
	{
		mBoard = new Board("laby.json");
	}

	~GameLoop()
	{
		delete mBoard;
		mBoard = nullptr;
	}

	void	update();
};
