#include "TwitterAnalyser.h"
#include "CoreFSMState.h"



START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFollowing)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, TreatUser)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()


void TwitterAnalyser::createFollowersFSM()
{

}