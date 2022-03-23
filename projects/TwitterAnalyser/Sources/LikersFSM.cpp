#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "ScrapperManager.h"


// all the different states

// retrieve user tweets
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetTweets)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// search tweets
START_DECLARE_COREFSMSTATE(TwitterAnalyser, SearchTweets)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// retrieve likes
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetLikers)
public:
	std::vector<u64>		userlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
#ifdef USE_SCRAPPER
void	manageRetrievedLikers(std::vector<std::string>&		TweetLikers);
#else

void	manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken);
void	copyUserList(std::vector<u64>& touserlist);
#endif
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedLikers, copyUserList)
END_DECLARE_COREFSMSTATE()

// retrieve favorites
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFavorites)
public:
std::vector<TwitterConnect::Twts>				mFavorites;
u32												mFavoritesCount=200;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFavorites(std::vector<TwitterConnect::Twts>& favs, const std::string& nexttoken);
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFavorites, copyUserList)
END_DECLARE_COREFSMSTATE()

// update likes statistics
START_DECLARE_COREFSMSTATE(TwitterAnalyser, UpdateLikesStats)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

#ifdef USE_SCRAPPER
// wait for scrapper
START_DECLARE_COREFSMSTATE(TwitterAnalyser, WaitScrapper)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()
#endif


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
#ifdef USE_SCRAPPER
	// go to wait state (push)
	SP<CoreFSMTransition> waitscrappertransition = KigsCore::GetInstanceOf("waitscrappertransition", "CoreFSMOnValueTransition");
	waitscrappertransition->setValue("TransitionBehavior", "Push");
	waitscrappertransition->setValue("ValueName", "NeedWait");
	waitscrappertransition->setState("WaitScrapper");
	waitscrappertransition->Init();
#endif
	// go to GetTweets
	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setState("GetTweets");
	gettweetstransition->Init();
	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(gettweetstransition);
#ifdef USE_SCRAPPER
	// create WaitScrapper state
	fsm->addState("WaitScrapper", new CoreFSMStateClass(TwitterAnalyser, WaitScrapper)());
	fsm->getState("WaitScrapper")->addTransition(mTransitionList["waitendtransition"]);
#endif

	// transition to GetLikers (push)
	SP<CoreFSMTransition> getlikerstransition = KigsCore::GetInstanceOf("getlikerstransition", "CoreFSMInternalSetTransition");
	getlikerstransition->setValue("TransitionBehavior", "Push");
	getlikerstransition->setState("GetLikers");
	getlikerstransition->Init();

	if (mUseHashTags)
	{
		// create GetTweets state
		fsm->addState("GetTweets", new CoreFSMStateClass(TwitterAnalyser, SearchTweets)());
	}
	else
	{
		// create SearchTweets state
		fsm->addState("GetTweets", new CoreFSMStateClass(TwitterAnalyser, GetTweets)());
	}
	
	// GetTweets can go to Wait or to GetLikers or pop
	fsm->getState("GetTweets")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetTweets")->addTransition(getlikerstransition);
	fsm->getState("GetTweets")->addTransition(mTransitionList["donetransition"]);
	// get tweets can also go to NeedUserListDetail
	fsm->getState("GetTweets")->addTransition(mTransitionList["userlistdetailtransition"]);

	
	// create GetLikers state
	fsm->addState("GetLikers", new CoreFSMStateClass(TwitterAnalyser, GetLikers)());
	// after GetLikers, can go to get user data (or pop)
#ifdef USE_SCRAPPER
	fsm->getState("GetLikers")->addTransition(waitscrappertransition);
#else
	fsm->getState("GetLikers")->addTransition(mTransitionList["waittransition"]);
#endif
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetLikers")->addTransition(mTransitionList["userlistdetailtransition"]);

	// when enough likers, pop
	fsm->getState("GetLikers")->addTransition(mTransitionList["popwhendone"]);

#ifdef USE_SCRAPPER
	// create webscrapper
	mScrapperManager = KigsCore::GetInstanceOf("mScrapperManager", "ScrapperManager");
	KigsCore::Connect(mScrapperManager.get(), "LikersRetreived", this, "manageRetrievedLikers");
	mScrapperManager->Init();
#else
#endif

	return "GetLikers";
}

std::string TwitterAnalyser::searchFavoritesFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// Init state, check if user was already started and launch next steps
	fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());

	// go to GetTweets
	SP<CoreFSMTransition> getuserfavoritestransition = KigsCore::GetInstanceOf("getuserfavoritestransition", "CoreFSMInternalSetTransition");
	getuserfavoritestransition->setState("GetUserFavorites");
	getuserfavoritestransition->Init();

	KigsCore::Connect(getuserfavoritestransition.get(), "ExecuteTransition", this, "setUserID", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			mUserList.clear();
			mUserList.push_back(mRetreivedUsers[0].mID);
		});

	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(getuserfavoritestransition);

	// create GetLikers state
	fsm->addState("GetUserFavorites", new CoreFSMStateClass(TwitterAnalyser, GetFavorites)());
	// after GetLikers, can go to get user data (or pop)
	fsm->getState("GetUserFavorites")->addTransition(mTransitionList["waittransition"]);
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetUserFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);

	fsm->getState("GetUserFavorites")->addTransition(mTransitionList["donetransition"]);

	((CoreFSMStateClass(TwitterAnalyser, GetFavorites)*)fsm->getState("GetUserFavorites"))->mFavoritesCount = 1000;

	return "GetUserFavorites";
}


void	TwitterAnalyser::analyseFavoritesFSM(const std::string& lastState)
{
	SP<CoreFSM> fsm = mFsm;

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "Push");
	getuserdatatransition->setState("GetFavorites");
	getuserdatatransition->Init();

	fsm->getState(lastState)->addTransition(getuserdatatransition);

	// when going to getuserdatatransition, retreive user list from previous state
	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserList", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mUserList);
		});


	// create GetFavorites state
	fsm->addState("GetFavorites", new CoreFSMStateClass(TwitterAnalyser, GetFavorites)());

#ifdef USE_SCRAPPER
	// create GetUserID transition (Push)
	SP<CoreFSMTransition> getuseridtransition = KigsCore::GetInstanceOf("getuseridtransition", "CoreFSMInternalSetTransition");
	getuseridtransition->setValue("TransitionBehavior", "Push");
	getuseridtransition->setState("GetUserID");
	getuseridtransition->Init();
	// getFavorites -> user detail, wait or pop
	fsm->getState("GetFavorites")->addTransition(getuseridtransition);
#else
	// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();
	// getFavorites -> user detail, wait or pop
	fsm->getState("GetFavorites")->addTransition(getuserdetailtransition);
#endif
	fsm->getState("GetFavorites")->addTransition(mTransitionList["waittransition"]);
	// GetFavorites can also go to NeedUserListDetail
	fsm->getState("GetFavorites")->addTransition(mTransitionList["userlistdetailtransition"]);
	fsm->getState("GetFavorites")->addTransition(mTransitionList["popwhendone"]);

#ifdef USE_SCRAPPER
	// create state GetUserDetail
	fsm->addState("GetUserID", new CoreFSMStateClass(TwitterAnalyser, GetUserID)());
#else
	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());
#endif

	// create updateLikesStatistics transition (Push)
	SP<CoreFSMTransition> updatelikesstatstransition = KigsCore::GetInstanceOf("updatelikesstatstransition", "CoreFSMInternalSetTransition");
	updatelikesstatstransition->setValue("TransitionBehavior", "Push");
	updatelikesstatstransition->setState("UpdateLikesStats");
	updatelikesstatstransition->Init();

#ifdef USE_SCRAPPER
	// GetUserID can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserID")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserID")->addTransition(updatelikesstatstransition);
#else
	// GetUserDetail can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserDetail")->addTransition(updatelikesstatstransition);
#endif
		// create UpdateLikesStats state
	fsm->addState("UpdateLikesStats", new CoreFSMStateClass(TwitterAnalyser, UpdateLikesStats)());
	// no transition here, only pop

}




void	CoreFSMStartMethod(TwitterAnalyser, GetTweets)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetTweets)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetTweets))
{

	if (TwitterConnect::LoadTweetsFile(mTweets,mRetreivedUsers[0].mName.ToString()))
	{
		if (mCurrentTreatedTweetIndex < mTweets.size())
		{
			GetUpgrador()->activateTransition("getlikerstransition");
			return false;
		}
	}
	
	// need more tweets
	{
		KigsCore::Connect(mTwitterConnect.get(), "TweetRetrieved", this, "manageRetrievedTweets");
		mTwitterConnect->launchGetTweetRequest(mRetreivedUsers[0].mID, mRetreivedUsers[0].mName.ToString());
		mNeedWait = true;
		GetUpgrador()->activateTransition("waittransition");
	}
	return false;
}




void	CoreFSMStartMethod(TwitterAnalyser, SearchTweets)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, SearchTweets)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, SearchTweets))
{

	if (TwitterConnect::LoadTweetsFile(mTweets, TwitterConnect::getHashtagFilename(mRetreivedUsers[0].mName.ToString())))
	{
		if (mCurrentTreatedTweetIndex < mTweets.size())
		{
			GetUpgrador()->activateTransition("getlikerstransition");
			return false;
		}
	}

	// need more tweets
	{
		KigsCore::Connect(mTwitterConnect.get(), "TweetRetrieved", this, "manageRetrievedTweets");
		mTwitterConnect->launchSearchTweetRequest(mRetreivedUsers[0].mName.ToString());
		mNeedWait = true;
		GetUpgrador()->activateTransition("waittransition");
	}
	return false;
}

void	CoreFSMStartMethod(TwitterAnalyser, GetLikers)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetLikers)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetLikers))
{
	if (mCurrentTreatedTweetIndex < mTweets.size())
	{

		if (mTweets[mCurrentTreatedTweetIndex].mLikeCount)
		{
			u64 tweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;
#ifdef USE_SCRAPPER
			std::string username = TwitterConnect::userNameFromId(mTweets[mCurrentTreatedTweetIndex].mAuthorID);
			CoreItemSP likers = TwitterConnect::LoadLikersFile(tweetID, username);
			if (likers)
			{
				std::vector<std::string>& tweetLikers = mScrapperManager->getTweetLikers();
				tweetLikers.clear();
				mCurrentTreatedLikerIndex = 0;
				mValidTreatedLikersForThisTweet = 0;
				for (const auto& l : likers)
				{
					tweetLikers.push_back(l);
				}

				mTweetRetrievedLikerCount[tweetID] = tweetLikers.size();

				//retrieve likers favorites
				GetUpgrador()->activateTransition("getuserdatatransition");
			}
			else
			{
				std::string stringTweetID = TwitterConnect::GetIDString(tweetID);
				mScrapperManager->launchScrap(username, stringTweetID);
				mNeedWait = true;
				GetUpgrador()->activateTransition("waittransition");
			}
#else
			std::string filenamenext_token = "Cache/Tweets/";
			filenamenext_token += std::to_string(tweetID) + "_LikersNextCursor.json";
	
			CoreItemSP nextt=TwitterConnect::LoadJSon(filenamenext_token);

			std::string next_cursor = "-1";

			std::vector<u64>	 v;
			if (nextt)
			{
				next_cursor = nextt["next-cursor"];
			}
			else
			{
				v = TwitterConnect::LoadLikersFile(tweetID);
			}

			if ((v.size() == 0) || (next_cursor != "-1"))
			{
				KigsCore::Connect(mTwitterConnect.get(), "LikersRetrieved", this, "manageRetrievedLikers");
				mTwitterConnect->launchGetLikers(tweetID);
				GetUpgrador()->activateTransition("waittransition");
				mNeedWait = true;
			}
			else
			{
				mCurrentTreatedUserIndex = 0;
				mValidTreatedLikersForThisTweet = 0;
				GetUpgrador()->userlist = std::move(v);
				mTweetRetrievedLikerCount[tweetID] = GetUpgrador()->userlist.size();
				GetUpgrador()->activateTransition("getuserdatatransition");
			}
			
#endif 
		}
		else
		{
			mCurrentTreatedTweetIndex++;
		}
	}
	else
	{
		// need more tweets
		GetUpgrador()->popState();
	}

	
	return false;
}


#ifdef USE_SCRAPPER
void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikers)::manageRetrievedLikers(std::vector<std::string>& TweetLikers)
{
	u64 tweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;
	TwitterConnect::randomizeVector(TweetLikers);

	std::string username = TwitterConnect::userNameFromId(mTweets[mCurrentTreatedTweetIndex].mAuthorID);
	TwitterConnect::SaveLikersFile(TweetLikers,tweetID, username);
	
	requestDone();
}
#else
void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikers)::manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken)
{
	u64 tweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;

	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(tweetID) + "_LikersNextCursor.json";

	auto v=TwitterConnect::LoadLikersFile(tweetID);
	v.insert(v.end(), TweetLikers.begin(), TweetLikers.end());

	if (nexttoken != "-1")
	{

		CoreItemSP currentUserJson = MakeCoreMap();
		currentUserJson->set("next-cursor", nexttoken);
		TwitterConnect::SaveJSon(filenamenext_token, currentUserJson);
	}
	else
	{
		ModuleFileManager::RemoveFile(filenamenext_token.c_str());
		TwitterConnect::randomizeVector(v);
	}

	KigsCore::Disconnect(mTwitterConnect.get(), "LikersRetrieved", this, "manageRetrievedLikers");
	TwitterConnect::SaveLikersFile(v, tweetID);

	requestDone();
}
void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikers)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->userlist);
}

#endif

void	CoreFSMStartMethod(TwitterAnalyser, GetFavorites)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFavorites)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFavorites))
{
#ifdef USE_SCRAPPER
	std::vector<std::string>& userlist = mScrapperManager->getTweetLikers();
#else
	std::vector<u64>& userlist = mUserList;
#endif

	if ((mCurrentTreatedUserIndex < userlist.size()) && ((mValidTreatedLikersForThisTweet < mMaxLikersPerTweet) || (!mMaxLikersPerTweet)))
	{

		auto user = userlist[mCurrentTreatedUserIndex];
		auto found = mFoundUser.find(user);

		if (found != mFoundUser.end()) // this one was already treated
		{
			mCurrentTreatedUserIndex++; // goto next one
			return false;
		}

		std::string filenamenext_token = "Cache/Tweets/";
		filenamenext_token += std::to_string(user) + "_FavsNextCursor.json";

		CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

		std::string next_cursor = "-1";
		if (nextt)
		{
			next_cursor = nextt["next-cursor"];
		}
		bool hasFavoriteList = TwitterConnect::LoadFavoritesFile(user, GetUpgrador()->mFavorites);

		if (hasFavoriteList && ( (GetUpgrador()->mFavorites.size() >= GetUpgrador()->mFavoritesCount) || (next_cursor == "-1")))
		{
			mFoundUser.insert(user);
			// if favorites were retrieved
			if (GetUpgrador()->mFavorites.size())
			{
				TwitterConnect::UserStruct	newuser;
				newuser.mID = 0;
				mRetreivedUsers.push_back(newuser);
				mCurrentUserIndex = mRetreivedUsers.size() - 1;
				mUserToUserIndex[user] = mCurrentUserIndex;

				SP<CoreFSM> fsm = mFsm;

				if (GetUpgrador()->getTransition("getuserdatatransition"))
				{
					GetUpgrador()->activateTransition("getuserdatatransition");
				}
				else
				{

#ifdef USE_SCRAPPER
					auto userIDState = getFSMState(fsm, TwitterAnalyser, GetUserID);
					userIDState->userName = user;
					userIDState->nextTransition = "updatelikesstatstransition";
					GetUpgrador()->activateTransition("getuseridtransition");
#else
					mRetreivedUsers[mCurrentUserIndex].mID = user;
					auto userDetailState = getFSMState(fsm, TwitterAnalyser, GetUserDetail);
					userDetailState->nextTransition = "updatelikesstatstransition";
					GetUpgrador()->activateTransition("getuserdetailtransition");
#endif
				}
			}
			else
			{
				mCurrentTreatedUserIndex++; // goto next one
				mTreatedUserCount++;
			}
		}
		else
		{
			KigsCore::Connect(mTwitterConnect.get(), "FavoritesRetrieved", this, "manageRetrievedFavorites");
			mTwitterConnect->launchGetFavoritesRequest(user);
			mNeedWait = true;
			GetUpgrador()->activateTransition("waittransition");
		}
	}
	else if(userlist.size())// treat next tweet
	{
		if (mCurrentTreatedUserIndex < userlist.size())
		{
			mCanGetMoreUsers = true;
		}
		mCurrentTreatedTweetIndex++;
		userlist.clear();
		mCurrentTreatedUserIndex = 0;
		mValidTreatedLikersForThisTweet = 0;
		GetUpgrador()->popState();
	}
	
	return false;
}


void	CoreFSMStateClassMethods(TwitterAnalyser, GetFavorites)::manageRetrievedFavorites(std::vector<TwitterConnect::Twts>& favs, const std::string& nexttoken)
{
#ifdef USE_SCRAPPER
	std::string user = mScrapperManager->getTweetLikers()[mCurrentTreatedLikerIndex];
	TwitterConnect::SaveFavoritesFile(user,favs);
#else
	auto user = mUserList[mCurrentTreatedUserIndex];

	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(user) + "_FavsNextCursor.json";
	std::vector<TwitterConnect::Twts> v;
	TwitterConnect::LoadFavoritesFile(user,v);
	v.insert(v.end(), favs.begin(), favs.end());

	if (nexttoken != "-1")
	{
		CoreItemSP currentUserJson = MakeCoreMap();
		currentUserJson->set("next-cursor", nexttoken);
		TwitterConnect::SaveJSon(filenamenext_token, currentUserJson);
	}
	else
	{
		ModuleFileManager::RemoveFile(filenamenext_token.c_str());
	}

	TwitterConnect::SaveFavoritesFile(user, v);
#endif
	KigsCore::Disconnect(mTwitterConnect.get(), "FavoritesRetrieved", this, "manageRetrievedFavorites");
	requestDone();
}

void	CoreFSMStateClassMethods(TwitterAnalyser, GetFavorites)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist.clear();
	for (const auto& f : GetUpgrador()->mFavorites)
	{
		touserlist.push_back(f.mAuthorID);
	}
}

void	CoreFSMStartMethod(TwitterAnalyser, UpdateLikesStats)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, UpdateLikesStats)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, UpdateLikesStats))
{

#ifdef USE_SCRAPPER
	std::vector<std::string>& userlist = mScrapperManager->getTweetLikers();

#else
	std::vector<u64>& userlist = mUserList;
#endif

	auto user = userlist[mCurrentTreatedUserIndex];
	u64 userID = mRetreivedUsers[mUserToUserIndex[user]].mID;

	if (!TwitterConnect::LoadUserStruct(userID, mRetreivedUsers[mUserToUserIndex[user]], false))
	{
		askUserDetail(userID);
	}

	auto  favsState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFavorites);

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
	if (!mHashTag.length())
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
#ifdef USE_SCRAPPER
void	CoreFSMStartMethod(TwitterAnalyser, WaitScrapper)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, WaitScrapper)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, WaitScrapper))
{
	if (mScrapperManager->getInactiveTime()>10.0)
	{
		// launch again
		mScrapperManager->protectedLaunchScrap();
	}
	return false;
}
#endif