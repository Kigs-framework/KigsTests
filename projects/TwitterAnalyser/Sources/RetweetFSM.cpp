#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveUserRetweeters)
public:
	unsigned int				mStateStep = 0;
	TwitterAnalyser::UserList	mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(TwitterAnalyser::UserList& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveUserRetweeted)
public:
	TwitterAnalyser::UserList	mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(TwitterAnalyser::UserList& touserlist);
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

	KigsCore::Connect(retrievetweetstransition.get(), "ExecuteTransition", this, "setRetrieveTweetsParams", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrievetweetsState = getFSMState(fsm, TwitterAnalyser, RetrieveTweets);
			retrievetweetsState->mUseHashtag = mUseHashTags;
			retrievetweetsState->mUserID = mPanelRetreivedUsers.getUserStructAtIndex(0).mID;
			retrievetweetsState->mUserName = mPanelRetreivedUsers.getUserStructAtIndex(0).mName.ToString();
			retrievetweetsState->mExcludeRetweets = true; // don't take retweets into account here
			retrievetweetsState->mExcludeReplies = false; // take replies into account here
		});

	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrievetweetstransition);

	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());

	// transition to RetrieveUserRetweeters (push)
	SP<CoreFSMTransition> managetweettransition = KigsCore::GetInstanceOf("managetweettransition", "CoreFSMInternalSetTransition");
	managetweettransition->setValue("TransitionBehavior", "Push");
	managetweettransition->setState("RetrieveUserRetweeters");
	managetweettransition->Init();

	KigsCore::Connect(managetweettransition.get(), "ExecuteTransition", this, "setRetrieveRetweetersState", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrieveRTState = getFSMState(fsm, TwitterAnalyser, RetrieveUserRetweeters);
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

	// transition to GetRetweeters (push)
	SP<CoreFSMTransition> getretweeterstransition = KigsCore::GetInstanceOf("getretweeterstransition", "CoreFSMInternalSetTransition");
	getretweeterstransition->setValue("TransitionBehavior", "Push");
	getretweeterstransition->setState("GetRetweeters");
	getretweeterstransition->Init();

	fsm->addState("RetrieveUserRetweeters", new CoreFSMStateClass(TwitterAnalyser, RetrieveUserRetweeters)());
	// can go to get likers or pop
	fsm->getState("RetrieveUserRetweeters")->addTransition(getretweeterstransition);
	fsm->getState("RetrieveUserRetweeters")->addTransition(mTransitionList["userlistdetailtransition"]);
	// when enough retweeters, pop
	fsm->getState("RetrieveUserRetweeters")->addTransition(mTransitionList["popwhendone"]);



	// create GetLikers state
	fsm->addState("GetRetweeters", new CoreFSMStateClass(TwitterAnalyser, GetRetweeters)());
	// after GetLikers, can go to get user data (or pop)
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["waittransition"]);

	// get likes can also go to NeedUserListDetail
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["userlistdetailtransition"]);


	return "RetrieveUserRetweeters";
}

std::string	TwitterAnalyser::searchRetweetedFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// no hashtag for retweeted
	fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());

	// go to RetrieveTweets
	SP<CoreFSMTransition> retrievetweetstransition = KigsCore::GetInstanceOf("retrievetweetstransition", "CoreFSMInternalSetTransition");
	retrievetweetstransition->setState("RetrieveTweets");
	retrievetweetstransition->Init();

	KigsCore::Connect(retrievetweetstransition.get(), "ExecuteTransition", this, "setRetrieveTweetsParams", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrievetweetsState = getFSMState(fsm, TwitterAnalyser, RetrieveTweets);
			retrievetweetsState->mUseHashtag = mUseHashTags;
			retrievetweetsState->mUserID = mPanelRetreivedUsers.getUserStructAtIndex(0).mID;
			retrievetweetsState->mUserName = mPanelRetreivedUsers.getUserStructAtIndex(0).mName.ToString();
			retrievetweetsState->mExcludeRetweets = false; // don't take retweets into account here
			retrievetweetsState->mExcludeReplies = true; // take replies into account here
		});

	// Init can go to Wait or RetrieveTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrievetweetstransition);

	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());

	CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)* retreiveTweets = (CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)*)fsm->getState("RetrieveTweets");

	// transition to RetrieveUserRetweeted (push)
	SP<CoreFSMTransition> managetweettransition = KigsCore::GetInstanceOf("managetweettransition", "CoreFSMInternalSetTransition");
	managetweettransition->setValue("TransitionBehavior", "Push");
	managetweettransition->setState("RetrieveUserRetweeted");
	managetweettransition->Init();

	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setValue("TransitionBehavior", "Push");
	gettweetstransition->setState("GetTweets");
	gettweetstransition->Init();

	// can go to get tweets or managetweet or done
	fsm->getState("RetrieveTweets")->addTransition(gettweetstransition);
	fsm->getState("RetrieveTweets")->addTransition(managetweettransition);
	fsm->getState("RetrieveTweets")->addTransition(mTransitionList["donetransition"]);
	fsm->getState("RetrieveTweets")->addTransition(mTransitionList["userlistdetailtransition"]);


	fsm->addState("RetrieveUserRetweeted", new CoreFSMStateClass(TwitterAnalyser, RetrieveUserRetweeted)());
	fsm->getState("RetrieveUserRetweeted")->addTransition(mTransitionList["popwhendone"]);
	fsm->getState("RetrieveUserRetweeted")->addTransition(mTransitionList["userlistdetailtransition"]);

	
	return "RetrieveUserRetweeted";
}

void	TwitterAnalyser::analyseRetweetersFSM(const std::string& lastState)
{

}
void	TwitterAnalyser::analyseRetweetedFSM(const std::string& lastState)
{

}


void	CoreFSMStartMethod(TwitterAnalyser, RetrieveUserRetweeters)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveUserRetweeters)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveUserRetweeters))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}

	SP<CoreFSM> fsm = mFsm;
	auto tweetsState = fsm->getStackedState("RetrieveTweets")->as<CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)>();

	if (tweetsState->mCurrentTreatedTweetIndex < tweetsState->mTweets.size())
	{
		auto& currentTweet = tweetsState->mTweets[tweetsState->mCurrentTreatedTweetIndex];
		if (currentTweet.mRetweetCount)
		{
			auto getRTState = getFSMState(fsm, TwitterAnalyser, GetRetweeters);

			if (GetUpgrador()->mStateStep == 0)
			{
				getRTState->mTweetID = currentTweet.mTweetID;
				GetUpgrador()->mStateStep = 1;
				GetUpgrador()->activateTransition("getretweeterstransition");
			}
			else if (GetUpgrador()->mStateStep == 1)
			{
				GetUpgrador()->mStateStep = 2;
				GetUpgrador()->mUserlist = std::move(getRTState->mUserlist);
				tweetsState->mTweetRetrievedUserCount[currentTweet.mTweetID] = { currentTweet.mRetweetCount, GetUpgrador()->mUserlist.size() };
				GetUpgrador()->activateTransition("getuserdatatransition");
			}
			else
			{
				GetUpgrador()->mStateStep = 0;
				tweetsState->mCurrentTreatedTweetIndex++;
			}
		}
		else
		{
			GetUpgrador()->mStateStep = 0;
			tweetsState->mCurrentTreatedTweetIndex++;
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

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveUserRetweeters)::copyUserList(TwitterAnalyser::UserList& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}



void	CoreFSMStartMethod(TwitterAnalyser, RetrieveUserRetweeted)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveUserRetweeted)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveUserRetweeted))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}

	SP<CoreFSM> fsm = mFsm;
	auto tweetsState = fsm->getStackedState("RetrieveTweets")->as<CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)>();

	GetUpgrador()->mUserlist.clear();

	u64 currentUser = mPanelRetreivedUsers.getUserStructAtIndex(0).mID;

	for (auto& twt : tweetsState->mTweets)
	{
		if (twt.mAuthorID != currentUser)
		{
			GetUpgrador()->mUserlist.addUser(twt.mAuthorID);
		}
	}

	if (tweetsState->mCurrentTreatedTweetIndex < tweetsState->mTweets.size())
	{
		tweetsState->mCurrentTreatedTweetIndex = tweetsState->mTweets.size();
		GetUpgrador()->activateTransition("getuserdatatransition");
	}
	else
	{
		// need more tweets
		GetUpgrador()->popState();
	}

	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveUserRetweeted)::copyUserList(TwitterAnalyser::UserList& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}

