#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "ScrapperManager.h"


// all the different states

// retrieve tweets
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetTweets)
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedTweets)
END_DECLARE_COREFSMSTATE()

// retrieve likes
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetLikes)
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedLikers(std::vector<std::string>&		TweetLikers);
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

// wait for scrapper
START_DECLARE_COREFSMSTATE(TwitterAnalyser, WaitScrapper)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()




void TwitterAnalyser::createLikersFSM()
{
	// add FSM
	SP<CoreFSM> fsm = KigsCore::GetInstanceOf("fsm", "CoreFSM");
	// need to add fsm to the object to control
	addItem(fsm);
	mFsm = fsm;

	// Init state, check if user was already started and launch next steps
	if (mUseHashTags)
	{
		fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitHashTag)());
	}
	else
	{
		fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());
	}
	// go to wait state (push)
	SP<CoreFSMTransition> waittransition = KigsCore::GetInstanceOf("waittransition", "CoreFSMOnValueTransition");
	waittransition->setValue("TransitionBehavior", "Push");
	waittransition->setValue("ValueName", "NeedWait");
	waittransition->setState("Wait");
	waittransition->Init();

	// go to wait state (push)
	SP<CoreFSMTransition> waitscrappertransition = KigsCore::GetInstanceOf("waitscrappertransition", "CoreFSMOnValueTransition");
	waitscrappertransition->setValue("TransitionBehavior", "Push");
	waitscrappertransition->setValue("ValueName", "NeedWait");
	waitscrappertransition->setState("WaitScrapper");
	waitscrappertransition->Init();

	// this one is needed for all cases
	fsm->addState("GetUserListDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserListDetail)());
	// only wait or pop
	fsm->getState("GetUserListDetail")->addTransition(waittransition);

	// go to GetUserListDetail state (push)
	SP<CoreFSMTransition> userlistdetailtransition = KigsCore::GetInstanceOf("userlistdetailtransition", "CoreFSMOnValueTransition");
	userlistdetailtransition->setValue("TransitionBehavior", "Push");
	userlistdetailtransition->setValue("ValueName", "NeedUserListDetail");
	userlistdetailtransition->setState("GetUserListDetail");
	userlistdetailtransition->Init();
	
	// this one is needed for all cases
	fsm->addState("Done", new CoreFSMStateClass(TwitterAnalyser, Done)());
	// only wait or pop
	fsm->getState("Done")->addTransition(userlistdetailtransition);

	// pop wait state transition
	SP<CoreFSMTransition> waitendtransition = KigsCore::GetInstanceOf("waitendtransition", "CoreFSMOnValueTransition");
	waitendtransition->setValue("TransitionBehavior", "Pop");
	waitendtransition->setValue("ValueName", "NeedWait");
	waitendtransition->setValue("NotValue", true); // end wait when NeedWait is false
	waitendtransition->Init();

	// go to GetTweets
	SP<CoreFSMTransition> gettweetstransition = KigsCore::GetInstanceOf("gettweetstransition", "CoreFSMInternalSetTransition");
	gettweetstransition->setState("GetTweets");
	gettweetstransition->Init();
	// Init can go to Wait or GetTweets
	fsm->getState("Init")->addTransition(waittransition);
	fsm->getState("Init")->addTransition(gettweetstransition);

	// create WaitScrapper state
	fsm->addState("WaitScrapper", new CoreFSMStateClass(TwitterAnalyser, WaitScrapper)());
	fsm->getState("WaitScrapper")->addTransition(waitendtransition);

	// create Wait state
	fsm->addState("Wait", new CoreFSMStateClass(TwitterAnalyser, Wait)());
	// Wait state can pop back to previous state
	fsm->getState("Wait")->addTransition(waitendtransition);

	// transition to GetLikes (push)
	SP<CoreFSMTransition> getlikestransition = KigsCore::GetInstanceOf("getlikestransition", "CoreFSMInternalSetTransition");
	getlikestransition->setValue("TransitionBehavior", "Push");
	getlikestransition->setState("GetLikes");
	getlikestransition->Init();

	// transition to done state
	SP<CoreFSMTransition> donetransition = KigsCore::GetInstanceOf("donetransition", "CoreFSMInternalSetTransition");
	donetransition->setState("Done");
	donetransition->Init();

	// create GetTweets state
	fsm->addState("GetTweets", new CoreFSMStateClass(TwitterAnalyser, GetTweets)());
	// GetTweets can go to Wait or to GetLikes or pop
	fsm->getState("GetTweets")->addTransition(waittransition);
	fsm->getState("GetTweets")->addTransition(getlikestransition);
	fsm->getState("GetTweets")->addTransition(donetransition);
	// get tweets can also go to NeedUserListDetail
	fsm->getState("GetTweets")->addTransition(userlistdetailtransition);

	

	// create GetLikes state
	fsm->addState("GetLikes", new CoreFSMStateClass(TwitterAnalyser, GetLikes)());

	SP<CoreFSMTransition> getfavoritestransition = KigsCore::GetInstanceOf("getfavoritestransition", "CoreFSMInternalSetTransition");
	getfavoritestransition->setValue("TransitionBehavior", "Push");
	getfavoritestransition->setState("GetFavorites");
	getfavoritestransition->Init();

	// after GetLikes, can go to GetFavorites (or pop)
	fsm->getState("GetLikes")->addTransition(waitscrappertransition);
	fsm->getState("GetLikes")->addTransition(getfavoritestransition);
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetLikes")->addTransition(userlistdetailtransition);

	// create GetFavorites state
	fsm->addState("GetFavorites", new CoreFSMStateClass(TwitterAnalyser, GetFavorites)());

	// create GetUserID transition (Push)
	SP<CoreFSMTransition> getuseridtransition = KigsCore::GetInstanceOf("getuseridtransition", "CoreFSMInternalSetTransition");
	getuseridtransition->setValue("TransitionBehavior", "Push");
	getuseridtransition->setState("GetUserID");
	getuseridtransition->Init();
	// getFavorites -> user detail, wait or pop
	fsm->getState("GetFavorites")->addTransition(getuseridtransition);
	fsm->getState("GetFavorites")->addTransition(waittransition);
	// GetFavorites can also go to NeedUserListDetail
	fsm->getState("GetFavorites")->addTransition(userlistdetailtransition);


	// create state GetUserDetail
	fsm->addState("GetUserID", new CoreFSMStateClass(TwitterAnalyser, GetUserID)());
	
	// create updateLikesStatistics transition (Push)
	SP<CoreFSMTransition> updatelikesstatstransition = KigsCore::GetInstanceOf("updatelikesstatstransition", "CoreFSMInternalSetTransition");
	updatelikesstatstransition->setValue("TransitionBehavior", "Push");
	updatelikesstatstransition->setState("UpdateLikesStats");
	updatelikesstatstransition->Init();

	/*// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();*/
	
	// GetUserID can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserID")->addTransition(waittransition);
	fsm->getState("GetUserID")->addTransition(updatelikesstatstransition);


/*	// create GetUserDetail state
	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());



	// GetUserDetail can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(waittransition);
	fsm->getState("GetUserDetail")->addTransition(updatelikesstatstransition);*/

	// create UpdateLikesStats state
	fsm->addState("UpdateLikesStats", new CoreFSMStateClass(TwitterAnalyser, UpdateLikesStats)());
	// no transition here, only pop


	// create webscrapper
	mScrapperManager = KigsCore::GetInstanceOf("mScrapperManager", "ScrapperManager");
	KigsCore::Connect(mScrapperManager.get(), "LikersRetreived", this, "manageRetrievedLikers");
	mScrapperManager->Init();
}

void CoreFSMStartMethod(TwitterAnalyser, InitHashTag)
{
}

void CoreFSMStopMethod(TwitterAnalyser, InitHashTag)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, InitHashTag))
{
	// TODO
	return false;
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



void	CoreFSMStateClassMethods(TwitterAnalyser, GetTweets)::manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken)
{
	if (twtlist.size())
	{
		TwitterConnect::randomizeVector(twtlist);
		mTweets.insert(mTweets.end(), twtlist.begin(), twtlist.end());
	}
	else
	{
		// can not get more tweets
		GetUpgrador()->activateTransition("donetransition");
	}
	KigsCore::Disconnect(mTwitterConnect.get(), "TweetRetrieved", this, "manageRetrievedTweets");
	requestDone();

	TwitterConnect::SaveTweetsFile(mTweets, mRetreivedUsers[0].mName.ToString());

	if (nexttoken != "-1")
	{
		std::string filename = "Cache/UserName/";
		filename += mRetreivedUsers[0].mName.ToString() + "_TweetsNextCursor.json";
		CoreItemSP currentUserJson = MakeCoreMap();
		currentUserJson->set("next-cursor", nexttoken);
		TwitterConnect::SaveJSon(filename, currentUserJson);
	}
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

			std::string username = mRetreivedUsers[0].mName.ToString();

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



void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikes)::manageRetrievedLikers(std::vector<std::string>& TweetLikers)
{
	u64 tweetID = mTweets[mCurrentTreatedTweetIndex].mTweetID;
	TwitterConnect::randomizeVector(TweetLikers);

	std::string username = mRetreivedUsers[0].mName.ToString();
	TwitterConnect::SaveLikersFile(TweetLikers,tweetID, username);
	
	requestDone();
}


void	CoreFSMStartMethod(TwitterAnalyser, GetFavorites)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFavorites)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFavorites))
{
	std::vector<std::string>& tweetLikers = mScrapperManager->getTweetLikers();
	if ((mCurrentTreatedLikerIndex < tweetLikers.size()) && ((mValidTreatedLikersForThisTweet < mMaxLikersPerTweet) || (!mMaxLikersPerTweet)))
	{
		std::string user = tweetLikers[mCurrentTreatedLikerIndex];
		auto found = mFoundLiker.find(user);
		if (found != mFoundLiker.end()) // this one was already treated
		{
			(*found).second++;
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
			mFoundLiker[user] = 0;
			// if favorites were retrieved
			if (GetUpgrador()->mFavorites.size())
			{
				TwitterConnect::UserStruct	newuser;
				newuser.mID = 0;
				mRetreivedUsers.push_back(newuser);
				mCurrentUserIndex = mRetreivedUsers.size() - 1;
				mNameToUserIndex[user] = mCurrentUserIndex;

				SP<CoreFSM> fsm = mFsm;
				CoreFSMStateClass(TwitterAnalyser, GetUserID)* userIDState = (CoreFSMStateClass(TwitterAnalyser, GetUserID)*)fsm->getState("GetUserID");
				userIDState->userName = user;
				userIDState->nextTransition = "updatelikesstatstransition";

				GetUpgrador()->activateTransition("getuseridtransition");
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
	std::string user = mScrapperManager->getTweetLikers()[mCurrentTreatedLikerIndex];
	TwitterConnect::SaveFavoritesFile(user,favs);
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
	std::vector<std::string>& tweetLikers = mScrapperManager->getTweetLikers();
	std::string user = tweetLikers[mCurrentTreatedLikerIndex];
	u64 userID = mRetreivedUsers[mNameToUserIndex[user]].mID;

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