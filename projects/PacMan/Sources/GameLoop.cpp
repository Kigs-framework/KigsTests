#include "GameLoop.h"
#include "Core.h"



void	GameLoop::update()
{
	mBoard->Update();
}


GameLoop::GameLoop(CMSP linterface) :mMainInterface(linterface)
{
	srand(time(NULL));
	// Init Board
	mBoard = new Board("laby.json", mMainInterface);
	mBoard->initGraphicBoard();
	mBoard->InitGhosts();
	mBoard->InitPlayer();
}
