#pragma once

#include <DataDrivenBaseApplication.h>
#include "CoreFSMState.h"
#include "TwitterConnect.h"
#include "ScrapperManager.h"
#include "GraphDrawer.h"

class TwitterAnalyser : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(TwitterAnalyser, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(TwitterAnalyser);

	// give direct access to members
	friend class GraphDrawer;

protected:

	enum class dataType
	{
		Likers = 0,
		Followers = 1,
		Following = 2,
		Favorites = 3
	};

	void	requestDone();
	void	mainUserDone();
	void	switchForce();
	void	switchDisplay();

	WRAP_METHODS(requestDone, mainUserDone, switchDisplay, switchForce,manageRetrievedTweets);

	void		commonStatesFSM();
	std::unordered_map<KigsID, SP<CoreFSMTransition>>	mTransitionList;

	std::string	searchLikersFSM();
	std::string	searchFollowersFSM();
	std::string	searchFollowingFSM();

	void	analyseFavoritesFSM(const std::string& lastState);
	void	analyseFollowersFSM(const std::string& lastState);
	void	analyseFollowingFSM(const std::string& lastState);


	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	SP<TwitterConnect>			mTwitterConnect=nullptr;
	CMSP						mMainInterface=nullptr;

	SP<ScrapperManager>		mScrapperManager=nullptr;
	SP<GraphDrawer>			mGraphDrawer = nullptr;

	// list of tweets
	std::vector<TwitterConnect::Twts>	mTweets;
	u32									mCurrentTreatedTweetIndex = 0;
	// list of likers
	u32									mCurrentTreatedLikerIndex=0;
	u32									mValidTreatedLikersForThisTweet = 0;
	u32									mValidUserCount=0;
	u32									mTreatedUserCount = 0;
	u32									mMaxLikersPerTweet = 0;
	u32									mMaxUserCount=45;
	u32									mUserPanelSize=500;
	std::map<std::string, u32>			mFoundLiker;

	// analyse type
	dataType		mPanelType = dataType::Followers;
	dataType		mAnalysedType = dataType::Followers;

	bool			mUseHashTags = false;
	std::string		mHashTag;
	float			mValidUserPercent;
	// when retrieving followers
	u32				mWantedTotalPanelSize = 100000;

	// wait request was treated
	maBool	mNeedWait = BASE_ATTRIBUTE(NeedWait, false);

	// user 0 is main user if needed
	std::vector<TwitterConnect::UserStruct>		mRetreivedUsers;
	std::unordered_map<std::string, u32>		mNameToUserIndex;

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

// get user id from user name
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetUserID)
public:
	std::string	nextTransition;
	std::string	userName;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// wait state
START_DECLARE_COREFSMSTATE(TwitterAnalyser, Wait)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// everything is done
START_DECLARE_COREFSMSTATE(TwitterAnalyser, Done)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

