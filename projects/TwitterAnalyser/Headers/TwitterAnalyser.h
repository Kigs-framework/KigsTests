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

		// panel or analysed types
		RTters		= 6,
		RTted		= 7,
	};
protected:



	void	requestDone();
	void	mainUserDone(TwitterConnect::UserStruct& CurrentUserStruct);
	void	switchForce();
	void	switchDisplay();

	void	initLogos();

	void	manageRetrievedUserDetail(TwitterConnect::UserStruct& CurrentUserStruct);
	bool	checkDone();

	WRAP_METHODS(requestDone, mainUserDone, switchDisplay, switchForce, manageRetrievedUserDetail, checkDone);

	void		commonStatesFSM();
	std::unordered_map<KigsID, SP<CoreFSMTransition>>	mTransitionList;

	std::string	searchLikersFSM();
	std::string	searchRetweetersFSM();
	std::string	searchRetweetedFSM();
	std::string	searchFavoritesFSM();
	std::string	searchFollowFSM(const std::string& followtype);
	void		TopFSM(const std::string& laststate);

	void	analyseFavoritesFSM(const std::string& lastState);
	void	analyseFollowFSM(const std::string& lastState, const std::string& followtype);
	void	analyseRetweetersFSM(const std::string& lastState);
	void	analyseRetweetedFSM(const std::string& lastState);


	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;

	SP<TwitterConnect>			mTwitterConnect=nullptr;
	CMSP						mMainInterface=nullptr;

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

	std::set<u64>					mFoundUser;

	bool							mCanGetMoreUsers = false;

	// analyse type
	dataType		mPanelType = dataType::Followers;
	dataType		mAnalysedType = dataType::Following;

	bool			mUseHashTags = false;
	float			mValidUserPercent;
	// when retrieving followers
	u32				mWantedTotalPanelSize = 100000;

	// wait request was treated
	maBool	mNeedWait = BASE_ATTRIBUTE(NeedWait, false);

	// user 0 is main user if needed
	std::vector<TwitterConnect::UserStruct>		mRetreivedUsers;


	std::unordered_map<u64, u32>				mUserToUserIndex;


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

