#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"

// specific states for likers/favorites


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveUserFavorites)
public:
	unsigned int			mStateStep = 0;
	std::vector<u64>		mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveLikers)
public:
	unsigned int			mStateStep=0;
	std::vector<u64>		mUserlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();	
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()

// retrieve favorites
START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveFavorites)
public:
	unsigned int								mStateStep=0;
	std::vector<TwitterConnect::Twts>			mFavorites;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()


// update likes statistics
START_DECLARE_COREFSMSTATE(TwitterAnalyser, UpdateLikesStats)
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
	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrievetweetstransition);

	fsm->addState("RetrieveTweets", new CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)());

	CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)* retreiveTweets = (CoreFSMStateClass(TwitterAnalyser, RetrieveTweets)*)fsm->getState("RetrieveTweets");
	retreiveTweets->mExcludeRetweets = true; // don't take retweets into account here

	// transition to RetrieveLikers (push)
	SP<CoreFSMTransition> managetweettransition = KigsCore::GetInstanceOf("managetweettransition", "CoreFSMInternalSetTransition");
	managetweettransition->setValue("TransitionBehavior", "Push");
	managetweettransition->setState("RetrieveLikers");
	managetweettransition->Init();

	KigsCore::Connect(managetweettransition.get(), "ExecuteTransition", this, "setRetrieveLikersState", [this, fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto retrieveLikersState = getFSMState(fsm, TwitterAnalyser, RetrieveLikers);
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

	// create GetLikers state
	fsm->addState("GetLikers", new CoreFSMStateClass(TwitterAnalyser, GetLikers)());
	// after GetLikers, can go to get user data (or pop)
	fsm->getState("GetLikers")->addTransition(mTransitionList["waittransition"]);

	// get likes can also go to NeedUserListDetail
	fsm->getState("GetLikers")->addTransition(mTransitionList["userlistdetailtransition"]);

	// when enough likers, pop
	fsm->getState("GetLikers")->addTransition(mTransitionList["popwhendone"]);

	return "RetrieveLikers";
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
	SP<CoreFSMTransition> getuserfavoritestransition = KigsCore::GetInstanceOf("getuserfavoritestransition", "CoreFSMInternalSetTransition");
	getuserfavoritestransition->setValue("TransitionBehavior", "Push");
	getuserfavoritestransition->setState("GetUserFavorites");
	getuserfavoritestransition->Init();

	KigsCore::Connect(retrieveuserfavoritestransition.get(), "ExecuteTransition", this, "setUserID", [this,fsm](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			mUserList.clear();
			mUserList.push_back(mRetreivedUsers[0].mID);

			auto userfavs=((CoreFSMStateClass(TwitterAnalyser, GetFavorites)*)fsm->getState("GetUserFavorites"));

			userfavs->mUserID = mRetreivedUsers[0].mID;
			userfavs->mFavorites.clear();
			userfavs->mFavoritesCount = 1000;

		});

	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrieveuserfavoritestransition);

	// create GetUserFavorites state
	fsm->addState("RetrieveUserFavorites", new CoreFSMStateClass(TwitterAnalyser, RetrieveUserFavorites)());
	fsm->getState("RetrieveUserFavorites")->addTransition(getuserfavoritestransition);
	fsm->getState("RetrieveUserFavorites")->addTransition(mTransitionList["donetransition"]);
	// RetrieveFavorites can also go to NeedUserListDetail
	fsm->getState("RetrieveUserFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);
	

	// create GetUserFavorites state
	fsm->addState("GetUserFavorites", new CoreFSMStateClass(TwitterAnalyser, GetFavorites)());
	// after GetUserFavorites, can go to get user data (or pop)
	fsm->getState("GetUserFavorites")->addTransition(mTransitionList["waittransition"]);
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetUserFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);
	
	return "RetrieveUserFavorites";
}


void	TwitterAnalyser::analyseFavoritesFSM(const std::string& lastState)
{
	SP<CoreFSM> fsm = mFsm;

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "Push");
	getuserdatatransition->setState("RetrieveFavorites");
	getuserdatatransition->Init();

	fsm->getState(lastState)->addTransition(getuserdatatransition);

	// when going to getuserdatatransition, retreive user list from previous state
	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserList", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mUserList);
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

	// create updateLikesStatistics transition (Push)
	SP<CoreFSMTransition> updatelikesstatstransition = KigsCore::GetInstanceOf("updatelikesstatstransition", "CoreFSMInternalSetTransition");
	updatelikesstatstransition->setValue("TransitionBehavior", "Push");
	updatelikesstatstransition->setState("UpdateLikesStats");
	updatelikesstatstransition->Init();

	// GetUserDetail can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserDetail")->addTransition(updatelikesstatstransition);

		// create UpdateLikesStats state
	fsm->addState("UpdateLikesStats", new CoreFSMStateClass(TwitterAnalyser, UpdateLikesStats)());
	// no transition here, only pop

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
		GetUpgrador()->activateTransition("getuserfavoritestransition");
	}
	else
	{
		SP<CoreFSM> fsm = mFsm;
		auto userfavs = ((CoreFSMStateClass(TwitterAnalyser, GetFavorites)*)fsm->getState("GetUserFavorites"));

		GetUpgrador()->mUserlist.clear();
		for (const auto& u : userfavs->mFavorites)
		{
			GetUpgrador()->mUserlist.push_back(u.mAuthorID);
		}

		GetUpgrador()->activateTransition("getuserdatatransition");

		GetUpgrador()->mStateStep = 0;
	}


	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveUserFavorites)::copyUserList(std::vector<u64>& touserlist)
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
	std::vector<u64>& userlist = mUserList;

	if (GetUpgrador()->mStateStep == 0)
	{
		if ((mCurrentTreatedUserIndex < userlist.size()) && ((mValidTreatedLikersForThisTweet < mMaxLikersPerTweet) || (!mMaxLikersPerTweet)))
		{
			auto user = userlist[mCurrentTreatedUserIndex];
			auto found = mFoundUser.find(user);

			if (found != mFoundUser.end()) // this one was already treated
			{
				mCurrentTreatedUserIndex++; // goto next one
				return false;
			}

			SP<CoreFSM> fsm = mFsm;
			auto getGetFavoritesState = getFSMState(fsm, TwitterAnalyser, GetFavorites);

			getGetFavoritesState->mUserID = user;
			getGetFavoritesState->mFavorites.clear();
			getGetFavoritesState->mFavoritesCount = 200;

			mFoundUser.insert(user);

			GetUpgrador()->activateTransition("getfavoritestransition");

			GetUpgrador()->mStateStep = 1;
		}
		else if (userlist.size())// treat next tweet
		{
			if (mCurrentTreatedUserIndex < userlist.size())
			{
				mCanGetMoreUsers = true;
			}
			userlist.clear();
			mCurrentTreatedUserIndex = 0;
			mValidTreatedLikersForThisTweet = 0;
			GetUpgrador()->popState();
		}
	}
	else if(GetUpgrador()->mStateStep == 1)
	{
		auto user = userlist[mCurrentTreatedUserIndex];
		SP<CoreFSM> fsm = mFsm;
		auto getGetFavoritesState = getFSMState(fsm, TwitterAnalyser, GetFavorites);

		GetUpgrador()->mFavorites = std::move(getGetFavoritesState->mFavorites);

		// if favorites were retrieved
		if(GetUpgrador()->mFavorites.size())
		{
			TwitterConnect::UserStruct	newuser;
			newuser.mID = 0;
			mRetreivedUsers.push_back(newuser);
			mCurrentUserIndex = mRetreivedUsers.size() - 1;
			mUserToUserIndex[user] = mCurrentUserIndex;

			SP<CoreFSM> fsm = mFsm;

			mRetreivedUsers[mCurrentUserIndex].mID = user;
			auto userDetailState = getFSMState(fsm, TwitterAnalyser, GetUserDetail);
			userDetailState->nextTransition = "updatelikesstatstransition";
			GetUpgrador()->activateTransition("getuserdetailtransition");
			
			GetUpgrador()->mStateStep = 0;
		}
		else
		{
			mCurrentTreatedUserIndex++; // goto next one
			mTreatedUserCount++;
			GetUpgrador()->mStateStep = 0;
		}
	}
	
	
	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, UpdateLikesStats)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, UpdateLikesStats)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, UpdateLikesStats))
{

	std::vector<u64>& userlist = mUserList;

	auto user = userlist[mCurrentTreatedUserIndex];
	u64 userID = mRetreivedUsers[mUserToUserIndex[user]].mID;

	if (!TwitterConnect::LoadUserStruct(userID, mRetreivedUsers[mUserToUserIndex[user]], false))
	{
		askUserDetail(userID);
	}

	auto  favsState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, RetrieveFavorites);

	const auto& currentFavorites = favsState->mFavorites;
	
	std::map<u64, float>& currentWeightedFavorites = mWeightedData[userID];


	std::map<u64, u64> lFavoritesUsers;
	for (const auto& f : currentFavorites)
	{
		auto fw = currentWeightedFavorites.find(f.mAuthorID);
		if (fw != currentWeightedFavorites.end())
		{
			(*fw).second += 1.0f;
		}
		else
		{
			currentWeightedFavorites[f.mAuthorID] = 1.0f;
		}

		lFavoritesUsers[f.mAuthorID] = f.mAuthorID;
	}


	float mainAccountWeight = 1.0f;
	if (!mUseHashTags)
	{
		auto fw = currentWeightedFavorites.find(mRetreivedUsers[0].mID);
		if (fw != currentWeightedFavorites.end())
		{
			mainAccountWeight = (*fw).second;
		}
	}

	std::vector<u64>& currentUserLikes = mCheckedUserList[userID];

	for (auto f : lFavoritesUsers)
	{
		currentUserLikes.push_back(f.first);

		auto alreadyfound = mUsersUserCount.find(f.first);
		if (alreadyfound != mUsersUserCount.end())
		{
			(*alreadyfound).second.first++;
		}
		else
		{
			TwitterConnect::UserStruct	toAdd;
			toAdd.mW = 0.0f;
			
			mUsersUserCount[f.first] = std::pair<unsigned int, TwitterConnect::UserStruct>(1, toAdd);
		}
	}


	for (auto& toWeight : currentWeightedFavorites)
	{
		float currentW = 1.0f - fabsf(toWeight.second - mainAccountWeight) / (float)(toWeight.second + mainAccountWeight);

		toWeight.second = currentW * mainAccountWeight;
		mUsersUserCount[toWeight.first].second.mW += toWeight.second;
	}

	// this one is done
	mCurrentTreatedUserIndex++;
	mValidTreatedLikersForThisTweet++;
	mValidUserCount++;
	mTreatedUserCount++;

	GetUpgrador()->popState();

	return false;
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
	if (mCurrentTreatedTweetIndex < mTweets.size())
	{
		if (mTweets[mCurrentTreatedTweetIndex].mLikeCount)
		{
			SP<CoreFSM> fsm = mFsm;
			auto getLikersState = getFSMState(fsm, TwitterAnalyser, GetLikers);

			if (GetUpgrador()->mStateStep==0)
			{
				getLikersState->mTweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;
				GetUpgrador()->mStateStep = 1;
				GetUpgrador()->activateTransition("getlikerstransition");
			}
			else if (GetUpgrador()->mStateStep == 1)
			{
				mCurrentTreatedUserIndex = 0;
				mValidTreatedLikersForThisTweet = 0;
				GetUpgrador()->mStateStep = 2;
				GetUpgrador()->mUserlist = std::move(getLikersState->mUserlist);
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

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveLikers)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}

