#pragma once

#include "DataDrivenBaseApplication.h"


class GameLoop;

class PacMan : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(PacMan, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(PacMan);

	WRAP_METHODS(GameOver, LevelWon);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	void	GameOver();
	void	LevelWon();
	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	CMSP		mMainInterface=nullptr;
	GameLoop*	mGameLoop = nullptr;

	maInt	mScore = BASE_ATTRIBUTE(Score,0);
	maInt	mLives = BASE_ATTRIBUTE(Lives,3);
	maFloat mSpeedCoef = BASE_ATTRIBUTE(SpeedCoef, 1.0f);
};
