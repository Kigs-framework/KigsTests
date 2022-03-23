#pragma once

#include <DataDrivenBaseApplication.h>
#include "CoreFSMState.h"
#include "TwitterConnect.h"
#include "ScrapperManager.h"
#include "GraphDrawer.h"

//#define USE_SCRAPPER

class TwitterAnalyser : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(TwitterAnalyser, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(TwitterAnalyser);

	// give direct access to members
	friend class GraphDrawer;
	enum class dataType
	{
		// panel types
		Likers		= 0,			// with or without hashtag, with or without period
		Posters		= 1,			// with hashtag, with or without period
		// panel or analysed types
		Followers	= 2,			// no hashtag, no period here
		Following	= 3,			// no hashtag, no period here
		// analysed types only
		Favorites	= 4,			// with or without hashtag, with or without period
		TOP			= 5,			// if used for data type, then just work with panel
	};
protected:



	void	requestDone();
	void	mainUserDone(TwitterConnect::UserStruct& CurrentUserStruct);
	void	switchForce();
	void	switchDisplay();
	void	manageRetrievedUserDetail(TwitterConnect::UserStruct& CurrentUserStruct);
	bool	checkDone();


	WRAP_METHODS(requestDone, mainUserDone, switchDisplay, switchForce,manageRetrievedTweets, manageRetrievedUserDetail, checkDone);

	void		commonStatesFSM();
	std::unordered_map<KigsID, SP<CoreFSMTransition>>	mTransitionList;

	std::string	searchLikersFSM();
	std::string	searchFavoritesFSM();
	std::string	searchFollowFSM(const std::string& followtype);
	void		TopFSM(const std::string& laststate);

	void	analyseFavoritesFSM(const std::string& lastState);
	void	analyseFollowFSM(const std::string& lastState, const std::string& followtype);

	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	SP<TwitterConnect>			mTwitterConnect=nullptr;
	CMSP						mMainInterface=nullptr;
#ifdef USE_SCRAPPER
	SP<ScrapperManager>		mScrapperManager=nullptr;
#endif
	SP<GraphDrawer>			mGraphDrawer = nullptr;

	// list of tweets
	std::vector<TwitterConnect::Twts>	mTweets;

	// list of users (for panel)
	std::vector<u64>					mUserList;

	// retrieved likers count per tweet			
	std::map<u64, u32>					mTweetRetrievedLikerCount;
	u32									mCurrentTreatedTweetIndex = 0;
	// list of likers
	u32									mCurrentTreatedUserIndex=0;
	u32									mValidTreatedLikersForThisTweet = 0;
	u32									mValidUserCount=0;
	u32									mTreatedUserCount = 0;
	u32									mMaxLikersPerTweet = 0;
	u32									mMaxUserCount=45;
	u32									mUserPanelSize=500;
#ifdef USE_SCRAPPER
	std::set<std::string>			mFoundUser;
#else
	std::set<u64>					mFoundUser;
#endif
	bool							mCanGetMoreUsers = false;

	// analyse type
	dataType		mPanelType = dataType::Followers;
	dataType		mAnalysedType = dataType::Following;

	bool			mUseHashTags = false;
	std::string		mHashTag;
	float			mValidUserPercent;
	// when retrieving followers
	u32				mWantedTotalPanelSize = 100000;

	// wait request was treated
	maBool	mNeedWait = BASE_ATTRIBUTE(NeedWait, false);

	// user 0 is main user if needed
	std::vector<TwitterConnect::UserStruct>		mRetreivedUsers;

#ifdef USE_SCRAPPER
	std::unordered_map<std::string, u32>		mUserToUserIndex;
#else
	std::unordered_map<u64, u32>				mUserToUserIndex;
#endif

	// per user map of favorites or following 
	std::map <u64, std::map<u64, float> >										mWeightedData;
	std::map<u64, std::vector<u64>>												mCheckedUserList;
	std::map<u64, std::pair<unsigned int, TwitterConnect::UserStruct>>			mUsersUserCount;

	u32			mCurrentUserIndex = 0;

	void	askUserDetail(u64 userID)
	{
		if (mAlreadyAskedUserDetail.find(userID) == mAlreadyAskedUserDetail.end())
		{
			mAlreadyAskedUserDetail.insert(userID);
			mUserDetailsAsked.push_back(userID);
			mNeedUserListDetail = true;
		}
	}

	const TwitterConnect::UserStruct& getRetreivedUser(u64 uid)
	{
		for(const auto& u: mRetreivedUsers)
		{
			if (u.mID == uid)
			{
				return u;
			}
		}
		return mRetreivedUsers[0];
	}

	void	manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken);

	// user detail asked
	std::vector<u64>			mUserDetailsAsked;
	std::set<u64>				mAlreadyAskedUserDetail;
	maBool	mNeedUserListDetail = BASE_ATTRIBUTE(NeedUserListDetail, false);

	CMSP mFsm;

	bool	isUserOf(u64 follower, u64 account) const
	{
		const auto& found = mCheckedUserList.find(follower);
		if (found != mCheckedUserList.end())
		{
			for (auto& c : (*found).second)
			{
				if (c == account)
				{
					return true;
				}
			}
		}
		return false;
	}
};

// different states

// Init from hashtag 
START_DECLARE_COREFSMSTATE(TwitterAnalyser, InitHashTag)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// Init from User
START_DECLARE_COREFSMSTATE(TwitterAnalyser, InitUser)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// retrieve user details
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetUserDetail)
public:
	std::string	nextTransition;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// retrieve user details for each one in the list
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetUserListDetail)
TwitterConnect::UserStruct	mTmpUserStruct;
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

#ifdef USE_SCRAPPER
// get user id from user name
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetUserID)
public:
	std::string	nextTransition;
	std::string	userName;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()
#endif

// wait state
START_DECLARE_COREFSMSTATE(TwitterAnalyser, Wait)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// everything is done
START_DECLARE_COREFSMSTATE(TwitterAnalyser, Done)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

