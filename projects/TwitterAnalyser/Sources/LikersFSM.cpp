#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"

// specific states for likers/favorites


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveUserFavorites)
public:
	unsigned int					mStateStep = 0;
	TwitterAnalyser::UserList		mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(TwitterAnalyser::UserList& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveUserLikers)
public:
	unsigned int					mStateStep=0;
	TwitterAnalyser::UserList		mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();	
void	copyUserList(TwitterAnalyser::UserList& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveLikers)
public:
	unsigned int					mStateStep = 0;
	TwitterAnalyser::UserList		mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
	void	copyUserList(TwitterAnalyser::UserList& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()

// retrieve favorites
START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveFavorites)
public:
	unsigned int								mStateStep=0;
	std::vector<TwitterConnect::Twts>			mFavorites;
	u32											mValidTreatedUserForThisTweet=0;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// retrieve some tweets
START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveSomeTweets)
public:
	unsigned int								mStateStep = 0;
	std::vector<TwitterConnect::Twts>			mTweets;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()


std::string TwitterAnalyser::searchLikersFSM()
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

	// transition to RetrieveUserLikers (push)
	SP<CoreFSMTransition> managetweettransition = KigsCore::GetInstanceOf("managetweettransition", "CoreFSMInternalSetTransition");
	managetweettransition->setValue("TransitionBehavior", "Push");
	managetweettransition->setState("RetrieveUserLikers");
	managetweettransition->Init();

	KigsCore::Connect(managetweettransition.get(), "ExecuteTransition", this, "setRetrieveLikersState", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrieveLikersState = getFSMState(fsm, TwitterAnalyser, RetrieveUserLikers);
			retrieveLikersState->mStateStep = 0;
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


	// transition to GetLikers (push)
	SP<CoreFSMTransition> getlikerstransition = KigsCore::GetInstanceOf("getlikerstransition", "CoreFSMInternalSetTransition");
	getlikerstransition->setValue("TransitionBehavior", "Push");
	getlikerstransition->setState("GetLikers");
	getlikerstransition->Init();

	fsm->addState("RetrieveUserLikers", new CoreFSMStateClass(TwitterAnalyser, RetrieveUserLikers)());
	// can go to get likers or pop
	fsm->getState("RetrieveUserLikers")->addTransition(getlikerstransition);
	fsm->getState("RetrieveUserLikers")->addTransition(mTransitionList["userlistdetailtransition"]);
	// when enough likers, pop
	fsm->getState("RetrieveUserLikers")->addTransition(mTransitionList["popwhendone"]);

	// create GetLikers state
	fsm->addState("GetLikers", new CoreFSMStateClass(TwitterAnalyser, GetLikers)());
	// after GetLikers, can go to get user data (or pop)
	fsm->getState("GetLikers")->addTransition(mTransitionList["waittransition"]);

	// get likes can also go to NeedUserListDetail
	fsm->getState("GetLikers")->addTransition(mTransitionList["userlistdetailtransition"]);

	
	return "RetrieveUserLikers";
}

std::string TwitterAnalyser::searchFavoritesFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// Init state, check if user was already started and launch next steps
	fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());

	// go to RetrieveUserFavorites
	SP<CoreFSMTransition> retrieveuserfavoritestransition = KigsCore::GetInstanceOf("retrieveuserfavoritestransition", "CoreFSMInternalSetTransition");
	retrieveuserfavoritestransition->setState("RetrieveUserFavorites");
	retrieveuserfavoritestransition->Init();

	// create getFavorites transition (Push)
	SP<CoreFSMTransition> getfavoritestransition = KigsCore::GetInstanceOf("getfavoritestransition", "CoreFSMInternalSetTransition");
	getfavoritestransition->setValue("TransitionBehavior", "Push");
	getfavoritestransition->setState("GetFavorites");
	getfavoritestransition->Init();

	KigsCore::Connect(retrieveuserfavoritestransition.get(), "ExecuteTransition", this, "setUserID", [this,fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			
			mPanelUserList.addUser(mPanelRetreivedUsers.getUserStructAtIndex(0).mID);

			auto userfavs=((CoreFSMStateClass(TwitterAnalyser, GetFavorites)*)fsm->getState("GetFavorites"));

			userfavs->mUserID = mPanelRetreivedUsers.getUserStructAtIndex(0).mID;
			userfavs->mFavorites.clear();
			userfavs->mFavoritesCount = 1000;

		});

	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrieveuserfavoritestransition);

	// create RetrieveUserFavorites state
	fsm->addState("RetrieveUserFavorites", new CoreFSMStateClass(TwitterAnalyser, RetrieveUserFavorites)());
	fsm->getState("RetrieveUserFavorites")->addTransition(getfavoritestransition);
	fsm->getState("RetrieveUserFavorites")->addTransition(mTransitionList["donetransition"]);
	// RetrieveFavorites can also go to NeedUserListDetail
	fsm->getState("RetrieveUserFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);
	

	// create GetFavorites state
	fsm->addState("GetFavorites", new CoreFSMStateClass(TwitterAnalyser, GetFavorites)());
	// after GetFavorites, can go to get user data (or pop)
	fsm->getState("GetFavorites")->addTransition(mTransitionList["waittransition"]);
	// GetFavorites can also go to NeedUserListDetail
	fsm->getState("GetFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);
	
	return "RetrieveUserFavorites";
}


void	TwitterAnalyser::analyseFavoritesFSM(const std::string& lastState)
{
	SP<CoreFSM> fsm = mFsm;

	// create new fsm block
	fsm->setCurrentBlock(fsm->addBlock());

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "PushBlock");
	getuserdatatransition->setState("RetrieveFavorites",1); // change block index
	getuserdatatransition->Init();

	fsm->getState(lastState)->addTransition(getuserdatatransition);

	// when going to getuserdatatransition, retreive user list from previous state
	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserList", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mPanelUserList);
		});


	// create GetFavorites state
	fsm->addState("RetrieveFavorites", new CoreFSMStateClass(TwitterAnalyser, RetrieveFavorites)());

	// create getFavorites transition (Push)
	SP<CoreFSMTransition> getfavoritestransition = KigsCore::GetInstanceOf("getfavoritestransition", "CoreFSMInternalSetTransition");
	getfavoritestransition->setValue("TransitionBehavior", "Push");
	getfavoritestransition->setState("GetFavorites");
	getfavoritestransition->Init();

	// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();

	// RetrieveFavorites -> get favorites / user detail, wait or pop
	fsm->getState("RetrieveFavorites")->addTransition(getfavoritestransition);
	fsm->getState("RetrieveFavorites")->addTransition(getuserdetailtransition);
	// RetrieveFavorites can also go to NeedUserListDetail
	fsm->getState("RetrieveFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);
	fsm->getState("RetrieveFavorites")->addTransition(mTransitionList["popwhendone"]);

	fsm->addState("GetFavorites", new CoreFSMStateClass(TwitterAnalyser, GetFavorites)());

	// GetFavorites can go to waittransition or pop
	fsm->getState("GetFavorites")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);

	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());

	// create UpdateStats transition (Push)
	SP<CoreFSMTransition> updatestatstransition = KigsCore::GetInstanceOf("updatestatstransition", "CoreFSMInternalSetTransition");
	updatestatstransition->setValue("TransitionBehavior", "Push");
	updatestatstransition->setState("UpdateStats");
	updatestatstransition->Init();

	// GetUserDetail can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserDetail")->addTransition(updatestatstransition);

		// create UpdateStats state
	fsm->addState("UpdateStats", new CoreFSMStateClass(TwitterAnalyser, UpdateStats)());
	// no transition here, only pop


	KigsCore::Connect(updatestatstransition.get(), "ExecuteTransition", this, "setupstats", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			// get follow state
			auto favState = fsm->getState("RetrieveFavorites")->as<CoreFSMStateClass(TwitterAnalyser, RetrieveFavorites)>();
			auto updateState = fsm->getState("UpdateStats")->as<CoreFSMStateClass(TwitterAnalyser, UpdateStats)>();

			updateState->mUserlist.clear();

			for (auto& f : favState->mFavorites)
			{
				updateState->mUserlist.addUser(f.mAuthorID);
			}

		});
}


void	TwitterAnalyser::analyseLikersFSM(const std::string& lastState)
{
	SP<CoreFSM> fsm = mFsm;

	// create new fsm block
	fsm->setCurrentBlock(fsm->addBlock());

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "PushBlock");
	getuserdatatransition->setState("RetrieveSomeTweets",1);
	getuserdatatransition->Init();

	fsm->getState(lastState)->addTransition(getuserdatatransition);

	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserDataParams", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mPanelUserList);
			auto retrievesometweetsState = getFSMState(fsm, TwitterAnalyser, RetrieveSomeTweets);
			retrievesometweetsState->mStateStep = 0;
		});

	// create RetrieveSomeTweets state
	fsm->addState("RetrieveSomeTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveSomeTweets)());

	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setValue("TransitionBehavior", "Push");
	gettweetstransition->setState("GetTweets");
	gettweetstransition->Init();

	SP<CoreFSMTransition> retrievelikerstransition = KigsCore::GetInstanceOf("retrievelikerstransition", "CoreFSMInternalSetTransition");
	retrievelikerstransition->setValue("TransitionBehavior", "Push");
	retrievelikerstransition->setState("RetrieveLikers");
	retrievelikerstransition->Init();

	// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();

	fsm->getState("RetrieveSomeTweets")->addTransition(gettweetstransition);
	fsm->getState("RetrieveSomeTweets")->addTransition(retrievelikerstransition);

	fsm->getState("RetrieveSomeTweets")->addTransition(mTransitionList["popwhendone"]);
	fsm->getState("RetrieveSomeTweets")->addTransition(mTransitionList["userlistdetailtransition"]);

	// create GetTweets state
	fsm->addState("GetTweets", new CoreFSMStateClass(TwitterAnalyser, GetTweets)());

	// GetTweets can go to Wait or pop
	fsm->getState("GetTweets")->addTransition(mTransitionList["waittransition"]);
	// get tweets can also go to NeedUserListDetail
	fsm->getState("GetTweets")->addTransition(mTransitionList["userlistdetailtransition"]);


	// transition to GetLikers (push)
	SP<CoreFSMTransition> getlikerstransition = KigsCore::GetInstanceOf("getlikerstransition", "CoreFSMInternalSetTransition");
	getlikerstransition->setValue("TransitionBehavior", "Push");
	getlikerstransition->setState("GetLikers");
	getlikerstransition->Init();


	fsm->addState("RetrieveLikers", new CoreFSMStateClass(TwitterAnalyser, RetrieveLikers)());
	// can go to get likers or pop
	fsm->getState("RetrieveLikers")->addTransition(getlikerstransition);
	fsm->getState("RetrieveLikers")->addTransition(mTransitionList["userlistdetailtransition"]);
	fsm->getState("RetrieveLikers")->addTransition(getuserdetailtransition);

	// create GetLikers state
	fsm->addState("GetLikers", new CoreFSMStateClass(TwitterAnalyser, GetLikers)());
	// after GetLikers, can go to get user data (or pop)
	fsm->getState("GetLikers")->addTransition(mTransitionList["waittransition"]);


	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());

	// create UpdateStats transition (Push)
	SP<CoreFSMTransition> updatestatstransition = KigsCore::GetInstanceOf("updatestatstransition", "CoreFSMInternalSetTransition");
	updatestatstransition->setValue("TransitionBehavior", "Push");
	updatestatstransition->setState("UpdateStats");
	updatestatstransition->Init();

	// GetUserDetail can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserDetail")->addTransition(updatestatstransition);

	// create UpdateLikesStats state
	fsm->addState("UpdateStats", new CoreFSMStateClass(TwitterAnalyser, UpdateStats)());
	// no transition here, only pop


	KigsCore::Connect(updatestatstransition.get(), "ExecuteTransition", this, "setupstats", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			// get follow state
			auto likersState = fsm->getState("RetrieveLikers")->as<CoreFSMStateClass(TwitterAnalyser, RetrieveLikers)>();
			auto updateState = fsm->getState("UpdateStats")->as<CoreFSMStateClass(TwitterAnalyser, UpdateStats)>();

			updateState->mUserlist = likersState->mUserlist;

		});

}


void	CoreFSMStartMethod(TwitterAnalyser, RetrieveUserFavorites)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveUserFavorites)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveUserFavorites))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}

	if (GetUpgrador()->mStateStep == 0)
	{
		GetUpgrador()->mStateStep = 1;
		GetUpgrador()->activateTransition("getfavoritestransition");
	}
	else
	{
		SP<CoreFSM> fsm = mFsm;
		auto userfavs = ((CoreFSMStateClass(TwitterAnalyser, GetFavorites)*)fsm->getState("GetFavorites"));

		GetUpgrador()->mUserlist.clear();
		for (const auto& u : userfavs->mFavorites)
		{
			GetUpgrador()->mUserlist.addUser(u.mAuthorID);
		}

		GetUpgrador()->activateTransition("getuserdatatransition");

		GetUpgrador()->mStateStep = 0;
	}


	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveUserFavorites)::copyUserList(TwitterAnalyser::UserList& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}

void	CoreFSMStartMethod(TwitterAnalyser, RetrieveFavorites)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveFavorites)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveFavorites))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}
	auto userlist = mPanelUserList.getList();

	if (GetUpgrador()->mStateStep == 0)
	{
		if ((mCurrentTreatedPanelUserIndex < userlist.size()) && ((GetUpgrador()->mValidTreatedUserForThisTweet < mMaxUsersPerTweet) || (!mMaxUsersPerTweet)))
		{
			auto user = userlist[mCurrentTreatedPanelUserIndex].first;
			
			SP<CoreFSM> fsm = mFsm;
			auto getGetFavoritesState = getFSMState(fsm, TwitterAnalyser, GetFavorites);

			getGetFavoritesState->mUserID = user;
			getGetFavoritesState->mFavorites.clear();
			getGetFavoritesState->mFavoritesCount = 200;

			GetUpgrador()->activateTransition("getfavoritestransition");

			GetUpgrador()->mStateStep = 1;
		}
		else if (userlist.size())// treat next tweet
		{
			if (mCurrentTreatedPanelUserIndex < userlist.size())
			{
				mCanGetMoreUsers = true;
			}
			userlist.clear();
			GetUpgrador()->mValidTreatedUserForThisTweet = 0;
			GetUpgrador()->popState();
		}
	}
	else if(GetUpgrador()->mStateStep == 1)
	{
		auto user = userlist[mCurrentTreatedPanelUserIndex].first;
		SP<CoreFSM> fsm = mFsm;
		auto getGetFavoritesState = getFSMState(fsm, TwitterAnalyser, GetFavorites);

		GetUpgrador()->mFavorites = std::move(getGetFavoritesState->mFavorites);

		// if favorites were retrieved
		if(GetUpgrador()->mFavorites.size())
		{
			mPanelRetreivedUsers.addUser(user);
			
			SP<CoreFSM> fsm = mFsm;

			auto userDetailState = getFSMState(fsm, TwitterAnalyser, GetUserDetail);
			userDetailState->mUserID = user;
			userDetailState->nextTransition = "updatestatstransition";
			GetUpgrador()->activateTransition("getuserdetailtransition");
			
			GetUpgrador()->mStateStep = 0;
		}
		else
		{
			mCurrentTreatedPanelUserIndex++; // goto next one
			mTreatedUserCount++;
			GetUpgrador()->mStateStep = 0;
		}
	}
	
	
	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, RetrieveUserLikers)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveUserLikers)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveUserLikers))
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
		if (currentTweet.mLikeCount)
		{
			auto getLikersState = getFSMState(fsm, TwitterAnalyser, GetLikers);

			if (GetUpgrador()->mStateStep==0)
			{
				getLikersState->mForID = currentTweet.mTweetID;
				GetUpgrador()->mStateStep = 1;
				GetUpgrador()->activateTransition("getlikerstransition");
			}
			else if (GetUpgrador()->mStateStep == 1)
			{
				GetUpgrador()->mStateStep = 2;
				GetUpgrador()->mUserlist = std::move(getLikersState->mUserlist);
				tweetsState->mTweetRetrievedUserCount[currentTweet.mTweetID] = { currentTweet.mLikeCount, GetUpgrador()->mUserlist.size() };
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

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveUserLikers)::copyUserList(TwitterAnalyser::UserList& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}

void	CoreFSMStartMethod(TwitterAnalyser, RetrieveLikers)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveLikers)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveLikers))
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
		if (currentTweet.mLikeCount)
		{
			auto getLikersState = getFSMState(fsm, TwitterAnalyser, GetLikers);

			if (GetUpgrador()->mStateStep == 0)
			{
				getLikersState->mForID = currentTweet.mTweetID;
				GetUpgrador()->mStateStep = 1;
				GetUpgrador()->activateTransition("getlikerstransition");
			}
			else if (GetUpgrador()->mStateStep == 1)
			{
				GetUpgrador()->mStateStep = 2;
				GetUpgrador()->mUserlist = std::move(getLikersState->mUserlist);
				tweetsState->mTweetRetrievedUserCount[currentTweet.mTweetID] = { currentTweet.mLikeCount, GetUpgrador()->mUserlist.size() };
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

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveLikers)::copyUserList(TwitterAnalyser::UserList& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}

void	CoreFSMStartMethod(TwitterAnalyser, RetrieveSomeTweets)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveSomeTweets)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveSomeTweets))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}


	if (GetUpgrador()->mStateStep == 0)
	{
		if (mCurrentTreatedPanelUserIndex < mPanelUserList.size())
		{
			GetUpgrador()->activateTransition("retrievetweetstransition");

			GetUpgrador()->mStateStep = 1;
		}
		else
		{
			// TODO ?
		}
	}
	else if (GetUpgrador()->mStateStep == 1)
	{
		mCurrentTreatedPanelUserIndex++;
		GetUpgrador()->mStateStep = 0;
	}

	return false;
}

