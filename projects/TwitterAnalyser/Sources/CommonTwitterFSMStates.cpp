#include "CommonTwitterFSMStates.h"


void CoreFSMStartMethod(TwitterAnalyser, InitUser)
{

}

void CoreFSMStopMethod(TwitterAnalyser, InitUser)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, InitUser))
{
	std::string currentUserProgress = "Cache/";
	currentUserProgress += "UserName/";
	currentUserProgress += mRetreivedUsers[0].mName.ToString() + ".json";
	CoreItemSP currentP = TwitterConnect::LoadJSon(currentUserProgress);

	if (!currentP) // new user
	{
		KigsCore::Connect(mTwitterConnect.get(), "UserDetailRetrieved", this, "mainUserDone");
		mTwitterConnect->launchUserDetailRequest(mRetreivedUsers[0].mName.ToString(), mRetreivedUsers[0]);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else // load current user
	{
		u64 userID = currentP["id"];
		mTwitterConnect->LoadUserStruct(userID, mRetreivedUsers[0], true);
		auto availableTransitions = GetUpgrador()->getTransitionList();
		for (const auto& t : availableTransitions)
		{
			// Init only have 2 transitions : "waittransition" and another
			// so active the other one
			if (t != "waittransition")
			{
				GetUpgrador()->activateTransition(t);
				break;
			}
		}
	}
	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, GetUserDetail)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetUserDetail)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetUserDetail))
{
	// back from next transition ?
	if (GetUpgrador()->nextTransition == "")
	{
		GetUpgrador()->popState();
		return false;
	}

	u64 userID = mRetreivedUsers[mCurrentUserIndex].mID;

	if (!mTwitterConnect->LoadUserStruct(userID, mRetreivedUsers[mCurrentUserIndex], false))
	{
		KigsCore::Connect(mTwitterConnect.get(), "UserDetailRetrieved", this, "manageRetrievedUserDetail");
		mTwitterConnect->launchUserDetailRequest(userID, mRetreivedUsers[mCurrentUserIndex]);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else
	{
		GetUpgrador()->activateTransition(GetUpgrador()->nextTransition);
		// go to next only once
		GetUpgrador()->nextTransition = "";
	}
	return false;
}

void	CoreFSMStartMethod(TwitterAnalyser, Wait)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, Wait)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, Wait))
{

	return false;
}

void	CoreFSMStartMethod(TwitterAnalyser, GetUserListDetail)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetUserListDetail)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetUserListDetail))
{
	if (!mUserDetailsAsked.size())
	{
		mNeedUserListDetail = false;
		GetUpgrador()->popState();
		return false;
	}

	u64 userID = mUserDetailsAsked.back();

	if (!TwitterConnect::LoadUserStruct(userID, GetUpgrador()->mTmpUserStruct, false))
	{
		KigsCore::Connect(mTwitterConnect.get(), "UserDetailRetrieved", this, "manageRetrievedUserDetail");
		mTwitterConnect->launchUserDetailRequest(userID, GetUpgrador()->mTmpUserStruct);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
		mUserDetailsAsked.pop_back();
	}

	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, Done)
{
	mGraphDrawer->setEverythingDone();
}
void	CoreFSMStopMethod(TwitterAnalyser, Done)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, Done))
{

	return false;
}




void CoreFSMStartMethod(TwitterAnalyser, InitHashTag)
{

}

void CoreFSMStopMethod(TwitterAnalyser, InitHashTag)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, InitHashTag))
{
	// nothing more to do, just go to next state
	auto availableTransitions = GetUpgrador()->getTransitionList();
	for (const auto& t : availableTransitions)
	{
		// Init only have 2 transitions : "waittransition" and another
		// so active the other one
		if (t != "waittransition")
		{
			GetUpgrador()->activateTransition(t);
			break;
		}
	}
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
	bool needMoreTweet=true;
	bool hasTweetFile=false;
	std::vector<TwitterConnect::Twts>	v;
	if (TwitterConnect::LoadTweetsFile(v, GetUpgrador()->mUserName))
	{
		hasTweetFile = true;
		if (GetUpgrador()->mNeededTweetCount >= v.size())
		{
			needMoreTweet = false;
		}
	}

	if (needMoreTweet)
	{
		std::string nextCursor = "-1";
		if (hasTweetFile)
		{
			std::string filenamenext_token = "Cache/UserName/";
			if (TwitterConnect::useDates())
			{
				filenamenext_token += "_" + TwitterConnect::getDate(0) + "_" + TwitterConnect::getDate(1) + "_";
			}
			filenamenext_token += GetUpgrador()->mUserName + "_TweetsNextCursor.json";
			auto nxtTokenJson = TwitterConnect::LoadJSon(filenamenext_token);

			if (nxtTokenJson)
			{
				nextCursor = nxtTokenJson["next-cursor"];
			}
		}

		// more tweets
		if ( (nextCursor != -1) || (!hasTweetFile))
		{
			KigsCore::Connect(mTwitterConnect.get(), "TweetRetrieved", this, "manageRetrievedTweets");
			if (GetUpgrador()->mSearchTweets)
			{
				mTwitterConnect->launchSearchTweetRequest(GetUpgrador()->mUserName, nextCursor);
			}
			else
			{
				mTwitterConnect->launchGetTweetRequest(GetUpgrador()->mUserID, GetUpgrador()->mUserName, nextCursor);
			}
			mNeedWait = true;
			GetUpgrador()->activateTransition("waittransition");
			return false;
		}
	}
	GetUpgrador()->popState();
	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, GetTweets)::manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken)
{
	std::string filenamenext_token = "Cache/UserName/";
	if (TwitterConnect::useDates())
	{
		filenamenext_token += "_" + TwitterConnect::getDate(0) + "_" + TwitterConnect::getDate(1) + "_";
	}
	filenamenext_token += GetUpgrador()->mUserName + "_TweetsNextCursor.json";

	std::vector<TwitterConnect::Twts>	v;
	TwitterConnect::LoadTweetsFile(v, GetUpgrador()->mUserName);
	v.insert(v.end(), twtlist.begin(), twtlist.end());

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

	KigsCore::Disconnect(mTwitterConnect.get(), "TweetRetrieved", this, "manageRetrievedTweets");
	TwitterConnect::SaveTweetsFile(v, GetUpgrador()->mUserName);

	requestDone();
}

void	CoreFSMStartMethod(TwitterAnalyser, GetLikers)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetLikers)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetLikers))
{
	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(GetUpgrador()->mTweetID) + "_LikersNextCursor.json";

	CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

	std::string next_cursor = "-1";

	std::vector<u64>	 v;
	if (nextt)
	{
		next_cursor = nextt["next-cursor"];
	}
	else
	{
		v = TwitterConnect::LoadLikersFile(GetUpgrador()->mTweetID);
	}

	if ((v.size() == 0) || (next_cursor != "-1"))
	{
		KigsCore::Connect(mTwitterConnect.get(), "LikersRetrieved", this, "manageRetrievedLikers");
		mTwitterConnect->launchGetLikers(GetUpgrador()->mTweetID, next_cursor);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else
	{
		GetUpgrador()->mUserlist = std::move(v);
		GetUpgrador()->popState();
	}
		
	return false;
}


void	CoreFSMStateClassMethods(TwitterAnalyser, GetLikers)::manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken)
{
	u64 tweetID = GetUpgrador()->mTweetID;

	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(tweetID) + "_LikersNextCursor.json";

	auto v = TwitterConnect::LoadLikersFile(tweetID);
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


void	CoreFSMStartMethod(TwitterAnalyser, GetFavorites)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFavorites)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFavorites))
{
	auto user = GetUpgrador()->mUserID;
		
	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(user) + "_FavsNextCursor.json";

	CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

	std::string next_cursor = "-1";
	if (nextt)
	{
		next_cursor = nextt["next-cursor"];
	}
	bool hasFavoriteList = TwitterConnect::LoadFavoritesFile(user, GetUpgrador()->mFavorites);

	// favorite file exist and enought found or not anymore
	// =>pop
	if (hasFavoriteList && ((GetUpgrador()->mFavorites.size() >= GetUpgrador()->mFavoritesCount) || (next_cursor == "-1")))
	{
		// if favorites were retrieved
		GetUpgrador()->popState();
	}
	else
	{
		KigsCore::Connect(mTwitterConnect.get(), "FavoritesRetrieved", this, "manageRetrievedFavorites");
		mTwitterConnect->launchGetFavoritesRequest(user, next_cursor);
		mNeedWait = true;
		GetUpgrador()->activateTransition("waittransition");
	}

	return false;
}


void	CoreFSMStateClassMethods(TwitterAnalyser, GetFavorites)::manageRetrievedFavorites(std::vector<TwitterConnect::Twts>& favs, const std::string& nexttoken)
{

	auto user = GetUpgrador()->mUserID;

	std::string filenamenext_token = "Cache/Tweets/";
	filenamenext_token += std::to_string(user) + "_FavsNextCursor.json";
	std::vector<TwitterConnect::Twts> v;
	TwitterConnect::LoadFavoritesFile(user, v);
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

	KigsCore::Disconnect(mTwitterConnect.get(), "FavoritesRetrieved", this, "manageRetrievedFavorites");
	requestDone();
}