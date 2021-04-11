#pragma once

#include "DataDrivenBaseApplication.h"
#include "Case.h"
class UIItem;
class AI;

class Arbres : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(Arbres, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(Arbres);

	static bool	isLocked()
	{
		return mIsLocked;
	}

	static void	unlock()
	{
		mIsLocked=false;
	}

	static void lock()
	{
		mIsLocked = true;
	}

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	// Labyrinthe managment

	void	initLabyrinthe();
	void	clearLabyrinthe();
	void	clearVisited();

	void	drawLabyrinthe();

	v2i		mLabyrintheSize;
	v2i		mStartPos;
	v2i		mExitPos;
	Case**	mLabyrinthe=nullptr;

	std::vector<UIItem*>	mGraphicCases;
	CMSP					mMainInterface;

	AI*						mAI = nullptr;
	bool					mPathFound = false;
	bool					mIsPlaying = false;

	void	launchAI();
	void	step();
	void	play();
	WRAP_METHODS(step,play,launchAI);

	CMSP	mThread;
	static bool	mIsLocked;

};
