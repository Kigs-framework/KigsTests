#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"


START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFollowing)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, TreatUser)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()


std::string TwitterAnalyser::searchFollowersFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// Init state, check if user was already started and launch next steps
	if (mUseHashTags)
	{
		fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitHashTag)());
	}
	else
	{
		fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());
	}

	return "";
	
}

std::string	TwitterAnalyser::searchFollowingFSM()
{
	return "";
}

void	TwitterAnalyser::analyseFollowersFSM(const std::string& lastState)
{

}
void	TwitterAnalyser::analyseFollowingFSM(const std::string& lastState)
{

}
