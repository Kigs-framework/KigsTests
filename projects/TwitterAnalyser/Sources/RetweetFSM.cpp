#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveRetweeters)
public:
	unsigned int			mStateStep = 0;
	std::vector<u64>		mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()


std::string	TwitterAnalyser::searchRetweetersFSM()
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

	// go to RetrieveTweets
	SP<CoreFSMTransition> retrievetweetstransition = KigsCore::GetInstanceOf("retrievetweetstransition", "CoreFSMInternalSetTransition");
	retrievetweetstransition->setState("RetrieveTweets");
	retrievetweetstransition->Init();
	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrievetweetstransition);

	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());

	// transition to RetrieveLikers (push)
	SP<CoreFSMTransition> managetweettransition = KigsCore::GetInstanceOf("managetweettransition", "CoreFSMInternalSetTransition");
	managetweettransition->setValue("TransitionBehavior", "Push");
	managetweettransition->setState("RetrieveRetweeters");
	managetweettransition->Init();

	KigsCore::Connect(managetweettransition.get(), "ExecuteTransition", this, "setRetrieveRetweetersState", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrieveRTState = getFSMState(fsm, TwitterAnalyser, RetrieveRetweeters);
			retrieveRTState->mStateStep = 0;
		});

	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setValue("TransitionBehavior", "Push");
	gettweetstransition->setState("GetTweets");
	gettweetstransition->Init();

	// can go to get tweets or managetweet or done
	fsm->getState("RetrieveTweets")->addTransition(gettweetstransition);
	fsm->getState("RetrieveTweets")->addTransition(managetweettransition);
	fsm->getState("RetrieveTweets")->addTransition(mTransitionList["donetransition"]);
	fsm->getState("RetrieveTweets")->addTransition(mTransitionList["userlistdetailtransition"]);

	// create GetTweets state
	fsm->addState("GetTweets", new CoreFSMStateClass(TwitterAnalyser, GetTweets)());

	// GetTweets can go to Wait or pop
	fsm->getState("GetTweets")->addTransition(mTransitionList["waittransition"]);
	// get tweets can also go to NeedUserListDetail
	fsm->getState("GetTweets")->addTransition(mTransitionList["userlistdetailtransition"]);


	// transition to GetRetweeters (push)
	SP<CoreFSMTransition> getretweeterstransition = KigsCore::GetInstanceOf("getretweeterstransition", "CoreFSMInternalSetTransition");
	getretweeterstransition->setValue("TransitionBehavior", "Push");
	getretweeterstransition->setState("GetRetweeters");
	getretweeterstransition->Init();

	fsm->addState("RetrieveRetweeters", new CoreFSMStateClass(TwitterAnalyser, RetrieveRetweeters)());
	// can go to get likers or pop
	fsm->getState("RetrieveRetweeters")->addTransition(getretweeterstransition);
	fsm->getState("RetrieveRetweeters")->addTransition(mTransitionList["userlistdetailtransition"]);

	// create GetLikers state
	fsm->addState("GetRetweeters", new CoreFSMStateClass(TwitterAnalyser, GetRetweeters)());
	// after GetLikers, can go to get user data (or pop)
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["waittransition"]);

	// get likes can also go to NeedUserListDetail
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["userlistdetailtransition"]);

	// when enough likers, pop
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["popwhendone"]);

	return "RetrieveRetweeters";
}

std::string	TwitterAnalyser::searchRetweetedFSM()
{
	return "";
}

void	TwitterAnalyser::analyseRetweetersFSM(const std::string& lastState)
{

}
void	TwitterAnalyser::analyseRetweetedFSM(const std::string& lastState)
{

}



void	CoreFSMStartMethod(TwitterAnalyser, RetrieveRetweeters)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveRetweeters)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveRetweeters))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}
	if (mCurrentTreatedTweetIndex < mTweets.size())
	{
		if (mTweets[mCurrentTreatedTweetIndex].mRetweetCount)
		{
			SP<CoreFSM> fsm = mFsm;
			auto getRTState = getFSMState(fsm, TwitterAnalyser, GetRetweeters);

			if (GetUpgrador()->mStateStep == 0)
			{
				getRTState->mTweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;
				GetUpgrador()->mStateStep = 1;
				GetUpgrador()->activateTransition("getretweeterstransition");
			}
			else if (GetUpgrador()->mStateStep == 1)
			{
				mCurrentTreatedUserIndex = 0;
				mValidTreatedLikersForThisTweet = 0;
				GetUpgrador()->mStateStep = 2;
				GetUpgrador()->mUserlist = std::move(getRTState->mUserlist);
				mTweetRetrievedLikerCount[mTweets[mCurrentTreatedTweetIndex].mTweetID] = GetUpgrador()->mUserlist.size();
				GetUpgrador()->activateTransition("getuserdatatransition");
			}
			else
			{
				GetUpgrador()->mStateStep = 0;
				mCurrentTreatedTweetIndex++;
			}
		}
		else
		{
			GetUpgrador()->mStateStep = 0;
			mCurrentTreatedTweetIndex++;
		}
	}
	else
	{
		GetUpgrador()->mStateStep = 0;
		// need more tweets
		GetUpgrador()->popState();
	}

	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveRetweeters)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}
