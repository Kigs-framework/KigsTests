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

	void	generateLabyrinthe();
	void	loadLabyrinthe();
	void	initLabyrinthe();
	void	openMoreWallsLabyrinthe(u32 tryOpeningCount);
	void	clearLabyrinthe();
	void	clearVisited();

	void	drawLabyrinthe();

	v2i		mLabyrintheSize;
	v2i		mStartPos;
	v2i		mExitPos;
	Case**	mLabyrinthe=nullptr;

	std::vector<SP<UIItem>>	mGraphicCases;
	CMSP					mMainInterface;

	AI*						mAI = nullptr;
	bool					mPathFound = false;
	bool					mIsPlaying = false;

	void	launchAI();
	void	step();
	void	play();
	void	firstfound();
	void	bestfound();
	void	dijkstra();
	void	astar();

	float	mCurrentFrequency = 4.0f;

	void	speedup();
	void	speeddown();

	void	showHideControls(bool show);
	void	showHideAI(bool show);

	template<typename ai>
	void	setupAI(u32 tryOpeningCount);

	WRAP_METHODS(step,play,launchAI, firstfound, bestfound, dijkstra, astar, speedup,speeddown);

	CMSP	mThread;
	static volatile bool	mIsLocked;

};
