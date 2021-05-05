#pragma once
#include "Texture.h"
#include "Timer.h"
class TweetFrequency;

class TwitterAccount
{
protected:

	friend class TweetFrequency;

	class UserStruct
	{
	public:
		usString					mName = std::string("");
		unsigned int				mFollowersCount = 0;
		unsigned int				mFollowingCount = 0;
		unsigned int				mStatuses_count = 0;
		std::string					UTCTime = "";
	};

	struct tweet
	{
		u64							mID;
		std::string					mCreatedAt;
	};

	UserStruct			mUserStruct;
	std::vector<tweet>	mTweetList;
	u64					mID;

	std::string			mNextTweetsToken="-1";


	CoreItemSP	loadTweetsFile();
	void		saveTweetsFile(CoreItemSP toSave);

	void		updateTweetList(CoreItemSP currentTwt);

	TweetFrequency* mSettings = nullptr;

	std::vector<std::vector<u32>>	mTweetsDate;

public:

	static time_t	getDateAndTime(const std::string& d);
	static u32		getWeekDay(const std::string& d);


	TwitterAccount(TweetFrequency* settings) : mSettings(settings)
	{

	}

	bool		needMoreTweets();
	void		addTweets(CoreItemSP json, bool addtofile);

	void		udateTweetsDate(const std::string& start, const std::string& end);

	const std::vector<std::vector<u32>>& getTweetsDate()
	{
		return mTweetsDate;
	}
	
};
