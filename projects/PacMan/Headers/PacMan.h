#pragma once

#include <DataDrivenBaseApplication.h>

class GameLoop;

class PacMan : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(PacMan, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(PacMan);

protected:
	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	
	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	CMSP		mMainInterface=nullptr;
	GameLoop*	mGameLoop = nullptr;
};
