#pragma once

#include "DataDrivenBaseApplication.h"
#include "Case.h"

class AI;
namespace Kigs
{

	namespace Draw2D
	{
		class UIItem;
	}
	using namespace DDriven;

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
			mIsLocked = false;
		}

		static void lock()
		{
			mIsLocked = true;
		}

		static void waitMainThread()
		{
			while (isLocked())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			lock();
		}

	protected:
		void	ProtectedInit() override;
		void	ProtectedUpdate() override;
		void	ProtectedClose() override;

		void	ProtectedInitSequence(const std::string& sequence) override;
		void	ProtectedCloseSequence(const std::string& sequence) override;

		// Labyrinthe managment

		void	generateLabyrinthe();
		void	initGraphicLabyrinthe();
		void	loadLabyrinthe();
		void	initLabyrinthe();
		void	openMoreWallsLabyrinthe(u32 tryOpeningCount);
		void	removeRandomWalls(u32 tryOpeningCount);
		void	clearLabyrinthe();
		void	clearVisited();
		bool	mLabyrintheCanBeShow = false;
		void	setupLabyrinthe(u32 tryOpeningCount, u32 removerandomwalls);

		u32		mTryOpeningCount = 0;
		u32		mRemoveRandomWalls = 0;

		void	drawLabyrinthe();

		v2i		mLabyrintheSize;
		v2i		mStartPos;
		v2i		mExitPos;
		Case** mLabyrinthe = nullptr;

		std::vector<SP<Draw2D::UIItem>>	mGraphicCases;
		CMSP							mMainInterface;
		SP<Draw2D::UIItem>				mLabyBG;

		AI* mAI = nullptr;
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

		void	setupAI();
		void	launchLaby();

		WRAP_METHODS(step, play, launchAI, firstfound, bestfound, dijkstra, astar, speedup, speeddown, launchLaby);

		CMSP	mThread;
		static volatile bool	mIsLocked;

	};
}
