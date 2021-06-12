#pragma once

#include "CoreModifiable.h"
#include "Board.h"
#include "Ghost.h"


class GameLoop
{
protected:

	CMSP			mMainInterface;

	Board* mBoard = nullptr;

public:

	GameLoop(CMSP linterface);

	~GameLoop()
	{
		delete mBoard;
		mBoard = nullptr;
	}

	void	update();


};
