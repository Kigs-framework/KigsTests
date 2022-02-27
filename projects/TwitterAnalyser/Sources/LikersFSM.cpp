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
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetLikes)
STARTCOREFSMSTATE_WRAPMETHODS();
#ifdef USE_SCRAPPER
void	manageRetrievedLikers(std::vector<std::string>&		TweetLikers);
#else
void	manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken);
#endif
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedLikers)
END_DECLARE_COREFSMSTATE()

// retrieve favorites
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFavorites)
public:
std::vector<TwitterConnect::favoriteStruct>				mFavorites;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFavorites(std::vector<TwitterConnect::favoriteStruct>& favs);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFavorites)
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

	// transition to GetLikes (push)
	SP<CoreFSMTransition> getlikestransition = KigsCore::GetInstanceOf("getlikestransition", "CoreFSMInternalSetTransition");
	getlikestransition->setValue("TransitionBehavior", "Push");
	getlikestransition->setState("GetLikes");
	getlikestransition->Init();

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
	
	// GetTweets can go to Wait or to GetLikes or pop
	fsm->getState("GetTweets")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetTweets")->addTransition(getlikestransition);
	fsm->getState("GetTweets")->addTransition(mTransitionList["donetransition"]);
	// get tweets can also go to NeedUserListDetail
	fsm->getState("GetTweets")->addTransition(mTransitionList["userlistdetailtransition"]);

	
	// create GetLikes state
	fsm->addState("GetLikes", new CoreFSMStateClass(TwitterAnalyser, GetLikes)());
	// after GetLikes, can go to GetFavorites (or pop)
#ifdef USE_SCRAPPER
	fsm->getState("GetLikes")->addTransition(waitscrappertransition);
#else
	fsm->getState("GetLikes")->addTransition(mTransitionList["waittransition"]);
#endif
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetLikes")->addTransition(mTransitionList["userlistdetailtransition"]);

#ifdef USE_SCRAPPER
	// create webscrapper
	mScrapperManager = KigsCore::GetInstanceOf("mScrapperManager", "ScrapperManager");
	KigsCore::Connect(mScrapperManager.get(), "LikersRetreived", this, "manageRetrievedLikers");
	mScrapperManager->Init();
#else
#endif

	return "GetLikes";
}

void	TwitterAnalyser::analyseFavoritesFSM(const std::string& lastState)
{
	SP<CoreFSM> fsm = mFsm;

	SP<CoreFSMTransition> getfavoritestransition = KigsCore::GetInstanceOf("getfavoritestransition", "CoreFSMInternalSetTransition");
	getfavoritestransition->setValue("TransitionBehavior", "Push");
	getfavoritestransition->setState("GetFavorites");
	getfavoritestransition->Init();

	fsm->getState(lastState)->addTransition(getfavoritestransition);

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
	
	// enough user
	if (mValidUserCount == mUserPanelSize)
	{
		GetUpgrador()->activateTransition("donetransition");
		return false;
	}

	if (TwitterConnect::LoadTweetsFile(mTweets,mRetreivedUsers[0].mName.ToString()))
	{
		if (mCurrentTreatedTweetIndex < mTweets.size())
		{
			GetUpgrador()->activateTransition("getlikestransition");
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

	// enough user
	if (mValidUserCount == mUserPanelSize)
	{
		GetUpgrador()->activateTransition("donetransition");
		return false;
	}

	if (TwitterConnect::LoadTweetsFile(mTweets, TwitterConnect::getHashtagFilename(mRetreivedUsers[0].mName.ToString())))
	{
		if (mCurrentTreatedTweetIndex < mTweets.size())
		{
			GetUpgrador()->activateTransition("getlikestransition");
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

void	CoreFSMStartMethod(TwitterAnalyser, GetLikes)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetLikes)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetLikes))
{
	if (mCurrentTreatedTweetIndex < mTweets.size())
	{
		// enough user
		if (mValidUserCount == mUserPanelSize)
		{
			GetUpgrador()->popState();
			return false;
		}

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
				GetUpgrador()->activateTransition("getfavoritestransition");
			}
			else
			{
				std::string stringTweetID = TwitterConnect::GetIDString(tweetID);
				mScrapperManager->launchScrap(username, stringTweetID);
				mNeedWait = true;
				GetUpgrador()->activateTransition("waittransition");
			}
#else
			
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
void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikes)::manageRetrievedLikers(std::vector<std::string>& TweetLikers)
{
	u64 tweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;
	TwitterConnect::randomizeVector(TweetLikers);

	std::string username = TwitterConnect::userNameFromId(mTweets[mCurrentTreatedTweetIndex].mAuthorID);
	TwitterConnect::SaveLikersFile(TweetLikers,tweetID, username);
	
	requestDone();
}
#else
void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikes)::manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken)
{
	u64 tweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;

	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(tweetID) + "_LikersNextCursor.json";
	if (nexttoken != "-1")
	{

		CoreItemSP currentUserJson = MakeCoreMap();
		currentUserJson->set("next-cursor", nexttoken);
		TwitterConnect::SaveJSon(filenamenext_token, currentUserJson);
	}
	else
	{
		ModuleFileManager::RemoveFile(filenamenext_token.c_str());
		TwitterConnect::randomizeVector(TweetLikers);
	}

	KigsCore::Disconnect(mTwitterConnect.get(), "LikersRetrieved", this, "manageRetrievedLikers");
	TwitterConnect::SaveLikersFile(TweetLikers, tweetID);

	requestDone();
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
	std::vector<std::string>& tweetLikers = mScrapperManager->getTweetLikers();
#else
	std::vector<u64>& tweetLikers = mUserList;
#endif
	if ((mCurrentTreatedLikerIndex < tweetLikers.size()) && ((mValidTreatedLikersForThisTweet < mMaxLikersPerTweet) || (!mMaxLikersPerTweet)))
	{

		auto user = tweetLikers[mCurrentTreatedLikerIndex];
		auto found = mFoundUser.find(user);

		if (found != mFoundUser.end()) // this one was already treated
		{
			mCurrentTreatedLikerIndex++; // goto next one
			return false;
		}

		// enough user
		if (mValidUserCount == mUserPanelSize)
		{
			GetUpgrador()->popState();
			return false;
		}

		if (TwitterConnect::LoadFavoritesFile(user, GetUpgrador()->mFavorites))
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
#ifdef USE_SCRAPPER
				CoreFSMStateClass(TwitterAnalyser, GetUserID)* userIDState = (CoreFSMStateClass(TwitterAnalyser, GetUserID)*)fsm->getState("GetUserID");
				userIDState->userName = user;
				userIDState->nextTransition = "updatelikesstatstransition";
				GetUpgrador()->activateTransition("getuseridtransition");
#else
				CoreFSMStateClass(TwitterAnalyser, GetUserDetail)* userDetailState = (CoreFSMStateClass(TwitterAnalyser, GetUserDetail)*)fsm->getState("GetUserDetail");
				userDetailState->nextTransition = "updatelikesstatstransition";
				GetUpgrador()->activateTransition("getuserdetailtransition");
#endif
				
			}
			else
			{
				mCurrentTreatedLikerIndex++; // goto next one
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
	else // treat next tweet
	{
		if (mCurrentTreatedLikerIndex < tweetLikers.size())
		{
			mCanGetMoreUsers = true;
		}
		mCurrentTreatedTweetIndex++;
		tweetLikers.clear();
		mCurrentTreatedLikerIndex = 0;
		mValidTreatedLikersForThisTweet = 0;
		GetUpgrador()->popState();
	}
	
	return false;
}


void	CoreFSMStateClassMethods(TwitterAnalyser, GetFavorites)::manageRetrievedFavorites(std::vector<TwitterConnect::favoriteStruct>& favs)
{
#ifdef USE_SCRAPPER
	std::string user = mScrapperManager->getTweetLikers()[mCurrentTreatedLikerIndex];
	TwitterConnect::SaveFavoritesFile(user,favs);
#else
#endif
	KigsCore::Disconnect(mTwitterConnect.get(), "FavoritesRetrieved", this, "manageRetrievedFavorites");
	requestDone();
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
	std::vector<std::string>& tweetLikers = mScrapperManager->getTweetLikers();

#else
	std::vector<u64>& tweetLikers = mUserList;
#endif

	auto user = tweetLikers[mCurrentTreatedLikerIndex];
	u64 userID = mRetreivedUsers[mUserToUserIndex[user]].mID;

	if (!TwitterConnect::LoadUserStruct(userID, mRetreivedUsers[mUserToUserIndex[user]], false))
	{
		askUserDetail(userID);
	}

	CoreFSMStateClass(TwitterAnalyser, GetFavorites)* favsState = (CoreFSMStateClass(TwitterAnalyser, GetFavorites)*)mFsm->as<CoreFSM>()->getState("GetFavorites");

	const auto& currentFavorites = favsState->mFavorites;
	
	std::map<u64, float>& currentWeightedFavorites = mWeightedData[userID];


	std::map<u64, u64> lFavoritesUsers;
	for (const auto& f : currentFavorites)
	{
		auto fw = currentWeightedFavorites.find(f.userID);
		if (fw != currentWeightedFavorites.end())
		{
			(*fw).second += 1.0f;
		}
		else
		{
			currentWeightedFavorites[f.userID] = 1.0f;
		}

		lFavoritesUsers[f.userID] = f.userID;
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
	mCurrentTreatedLikerIndex++;
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