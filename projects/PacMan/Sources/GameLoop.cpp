#include "GameLoop.h"
#include "Core.h"

void	GameLoop::update()
{

}


GameLoop::GameLoop(CMSP linterface) :mMainInterface(linterface)
{
	mBoard = new Board("laby.json");


	for (int i = 0; i < 5; i++)
	{
		mGhostList.push_back(KigsCore::GetInstanceOf("gg", "Ghost"));
	}
	
}