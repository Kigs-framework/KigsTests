#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"


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

	// go to RetrieveTweetActors
	SP<CoreFSMTransition> retrievetweetactorstransition = KigsCore::GetInstanceOf("retrievetweetactorstransition", "CoreFSMInternalSetTransition");
	retrievetweetactorstransition->setState("RetrieveTweetActors");
	retrievetweetactorstransition->Init();

	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrievetweetactorstransition);

	auto retrievetweetactorstate = new CoreFSMStateClass(TwitterAnalyser, RetrieveTweetActors)();
	// init state
	retrievetweetactorstate->mActorType = "Retweeters";
	retrievetweetactorstate->mMaxActorPerTweet = mMaxUsersPerTweet;

	fsm->addState("RetrieveTweetActors", retrievetweetactorstate);


	// transition to GetRetweeters (push)
	SP<CoreFSMTransition> getactorstransition = KigsCore::GetInstanceOf("getactorstransition", "CoreFSMInternalSetTransition");
	getactorstransition->setValue("TransitionBehavior", "Push");
	getactorstransition->setState("GetRetweeters");
	getactorstransition->Init();

	// transition to GetTweets (push)
	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setValue("TransitionBehavior", "Push");
	gettweetstransition->setState("GetTweets");
	gettweetstransition->Init();

	SP<CoreFSMTransition> retrievetweetstransition = KigsCore::GetInstanceOf("retrievetweetstransition", "CoreFSMInternalSetTransition");
	retrievetweetstransition->setValue("TransitionBehavior", "Push");
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
	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());
	fsm->getState("RetrieveTweets")->addTransition(gettweetstransition);

	// can go to retrieve tweets, get actors or done (or manage actors in analyse block)
	fsm->getState("RetrieveTweetActors")->addTransition(getactorstransition);
	fsm->getState("RetrieveTweetActors")->addTransition(retrievetweetstransition);
	fsm->getState("RetrieveTweetActors")->addTransition(mTransitionList["donetransition"]);
	fsm->getState("RetrieveTweetActors")->addTransition(mTransitionList["userlistdetailtransition"]);

	// create GetRetweeters state

	auto getactorsstate = new CoreFSMStateClass(TwitterAnalyser, GetRetweeters)();
	getactorsstate->mNeededUserCount = 0; // get them all

	fsm->addState("GetRetweeters", getactorsstate);
	// after GetRetweeters, can wait or pop
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["waittransition"]);

	auto gettweetsstate = new CoreFSMStateClass(TwitterAnalyser, GetTweets)();
	// create GetTweets state
	fsm->addState("GetTweets", gettweetsstate);
	// after GetTweets, can wait or pop
	fsm->getState("GetTweets")->addTransition(mTransitionList["waittransition"]);

	return "RetrieveTweetActors";
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
			retrievetweetsState->mExcludeRetweets = false; // take retweets into account here
			retrievetweetsState->mExcludeReplies = true; // don't take replies into account here
		});

	// Init can go to Wait or RetrieveTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrievetweetstransition);

	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());

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
	SP<CoreFSM> fsm = mFsm;

	// create new fsm block
	fsm->setCurrentBlock(fsm->addBlock());

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "PushBlock");
	getuserdatatransition->setState("GetUserDetail", 1);
	getuserdatatransition->Init();

	// retreive last state in block 0
	fsm->getState(lastState, 0)->addTransition(getuserdatatransition);

	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setuserdetailBlock1", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mPanelUserList);

			auto userDetailState = fsm->getState("GetUserDetail", 1)->as<CoreFSMStateClass(TwitterAnalyser, GetUserDetail)>();

			u64 user = mPanelUserList.getList()[mCurrentTreatedPanelUserIndex].first;

			mPanelRetreivedUsers.addUser(user);
			userDetailState->mUserID = user;
			userDetailState->nextTransition = "tweetactorstransition";

		});

	// create UpdateStats transition (Push)
	SP<CoreFSMTransition> tweetactorstransition = KigsCore::GetInstanceOf("tweetactorstransition", "CoreFSMInternalSetTransition");
	tweetactorstransition->setValue("TransitionBehavior", "Push");
	tweetactorstransition->setState("RetrieveTweetActors", 1);
	tweetactorstransition->Init();

	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());
	// GetUserDetail can go to RetrieveTweetActors, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserDetail")->addTransition(tweetactorstransition);

	auto retrievetweetactorstate = new CoreFSMStateClass(TwitterAnalyser, RetrieveTweetActors)();
	// init state
	retrievetweetactorstate->mActorType = "Retweeters";
	retrievetweetactorstate->mMaxActorPerTweet = mMaxUsersPerTweet;
	retrievetweetactorstate->mTreatAllActorsTogether = true;
	retrievetweetactorstate->mWantedActorCount = 200;

	fsm->addState("RetrieveTweetActors", retrievetweetactorstate);

	// transition to GetRetweeters (push)
	SP<CoreFSMTransition> getactorstransition = KigsCore::GetInstanceOf("getactorstransition", "CoreFSMInternalSetTransition");
	getactorstransition->setValue("TransitionBehavior", "Push");
	getactorstransition->setState("GetRetweeters", 1);
	getactorstransition->Init();

	// transition to GetTweets (push)
	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setValue("TransitionBehavior", "Push");
	gettweetstransition->setState("GetTweets", 1);
	gettweetstransition->Init();

	SP<CoreFSMTransition> retrievetweetstransition = KigsCore::GetInstanceOf("retrievetweetstransition", "CoreFSMInternalSetTransition");
	retrievetweetstransition->setValue("TransitionBehavior", "Push");
	retrievetweetstransition->setState("RetrieveTweets", 1);
	retrievetweetstransition->Init();

	KigsCore::Connect(retrievetweetstransition.get(), "ExecuteTransition", this, "setRetrieveTweetsParamsBlock1", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrievetweetsState = fsm->getState("RetrieveTweets", 1)->as<CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)>();
			retrievetweetsState->mUseHashtag = false;

			u64 user = mPanelUserList.getList()[mCurrentTreatedPanelUserIndex].first;
			std::string name = mPanelRetreivedUsers.getUserStruct(user).mName.ToString();

			if (name == "")
			{
				printf("problem here");
			}
			retrievetweetsState->mUserID = user;
			retrievetweetsState->mUserName = name;
			retrievetweetsState->mExcludeRetweets = true; // don't take retweets into account here
			retrievetweetsState->mExcludeReplies = false; // take replies into account here
		});
	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());
	fsm->getState("RetrieveTweets")->addTransition(gettweetstransition);

	// can go to retrieve tweets, get actors or done (or manage actors in analyse block)
	fsm->getState("RetrieveTweetActors")->addTransition(getactorstransition);
	fsm->getState("RetrieveTweetActors")->addTransition(retrievetweetstransition);
	fsm->getState("RetrieveTweetActors")->addTransition(mTransitionList["popwhendone"]);
	fsm->getState("RetrieveTweetActors")->addTransition(mTransitionList["userlistdetailtransition"]);

	// create GetRetweeters state

	auto getactorssstate = new CoreFSMStateClass(TwitterAnalyser, GetRetweeters)();
	getactorssstate->mNeededUserCount = 0; // get them all

	fsm->addState("GetRetweeters", getactorssstate);
	// after GetRetweeters, can wait or pop
	fsm->getState("GetRetweeters")->addTransition(mTransitionList["waittransition"]);

	auto gettweetsstate = new CoreFSMStateClass(TwitterAnalyser, GetTweets)();
	// create GetTweets state
	fsm->addState("GetTweets", gettweetsstate);
	// after GetTweets, can wait or pop
	fsm->getState("GetTweets")->addTransition(mTransitionList["waittransition"]);


	//////////////////////////////////////////////////


	SP<CoreFSMTransition> getuserdatatransition2 = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition2->setValue("TransitionBehavior", "Push");
	getuserdatatransition2->setState("UpdateStats", 1);
	getuserdatatransition2->Init();

	KigsCore::Connect(getuserdatatransition2.get(), "ExecuteTransition", this, "setupdatedataBlock1", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto actorsState = fsm->getState("RetrieveTweetActors", 1)->as<CoreFSMStateClass(TwitterAnalyser, RetrieveTweetActors)>();
			auto updateState = fsm->getState("UpdateStats", 1)->as<CoreFSMStateClass(TwitterAnalyser, UpdateStats)>();
			if (actorsState->mUserlist.size())
			{
				updateState->mUserlist = std::move(actorsState->mUserlist);
				updateState->mIsValid = true;
			}
			else
			{
				updateState->mUserlist.clear();
				updateState->mIsValid = false;
			}
		});

	fsm->getState("RetrieveTweetActors", 1)->addTransition(getuserdatatransition2);

	// create UpdateLikesStats state
	fsm->addState("UpdateStats", new CoreFSMStateClass(TwitterAnalyser, UpdateStats)());
	// no transition here, only pop

}
void	TwitterAnalyser::analyseRetweetedFSM(const std::string& lastState)
{

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

	if (GetUpgrador()->mUserlist.size() >= 500)
	{
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

