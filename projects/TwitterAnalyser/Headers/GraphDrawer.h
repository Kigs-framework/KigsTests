#pragma once

#include "CoreModifiable.h"
#include "CoreBaseApplication.h"
#include "CoreFSMState.h"


class TwitterAnalyser;

// manage drawing of current graph
class GraphDrawer : public CoreModifiable
{
protected:
	CMSP				mMainInterface;
		
	TwitterAnalyser* mTwitterAnalyser = nullptr;
	void	drawSpiral(std::vector<std::tuple<unsigned int, float,u64>>& toShow);
	void	drawForce();
	void	drawStats();

	void	drawGeneralStats();

	bool	mEverythingDone=false;

	std::unordered_map<u64, CMSP>									mShowedUser;

	enum Measure
	{
		Percent = 0,
		Similarity = 1,
		Normalized = 2,
		MEASURE_COUNT = 3
	};

	const std::string	mUnits[MEASURE_COUNT] = { "\%","sc","n" };

	u32		mCurrentUnit = 0;

public:
	DECLARE_CLASS_INFO(GraphDrawer, CoreModifiable, GraphDrawer);
	DECLARE_CONSTRUCTOR(GraphDrawer);

	void	setInterface(CMSP minterface)
	{
		mMainInterface = minterface;
	}

	void InitModifiable() override;

	maBool	mHasJaccard = BASE_ATTRIBUTE(HasJaccard, false);
	maBool  mGoNext = BASE_ATTRIBUTE(GoNext, false);
	void	setEverythingDone()
	{
		mEverythingDone = true;
	}

	void	nextDrawType();

};

// Percent drawing
START_DECLARE_COREFSMSTATE(GraphDrawer, Percent)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// Normalized drawing
START_DECLARE_COREFSMSTATE(GraphDrawer, Normalized)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// Jaccard Drawing
START_DECLARE_COREFSMSTATE(GraphDrawer, Jaccard)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// user stats drawing
START_DECLARE_COREFSMSTATE(GraphDrawer, UserStats)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// Force drawing
START_DECLARE_COREFSMSTATE(GraphDrawer, Force)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

