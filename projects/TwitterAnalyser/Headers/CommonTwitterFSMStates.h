#pragma once
#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"

START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetTweets)
public:
	std::string							mUserName="";
	std::string							mHashTag = "";
	u64									mUserID=-1;
	u32									mNeededTweetCount=100;
	bool								mSearchTweets=false;
	bool								mExcludeRetweets = false;
	bool								mExcludeReplies = false;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedTweets)
END_DECLARE_COREFSMSTATE()

// retrieve likes
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetLikers)
public:
	std::vector<u64>		mUserlist;
	u64						mTweetID;
protected:
	STARTCOREFSMSTATE_WRAPMETHODS();
	void	manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedLikers)
END_DECLARE_COREFSMSTATE()


// retrieve likes
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetRetweeters)
public:
	std::vector<u64>		mUserlist;
	u64						mTweetID;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedRetweeters(std::vector<u64>& RTers, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedRetweeters)
END_DECLARE_COREFSMSTATE()

// retrieve favorites
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFavorites)
public:
u64												mUserID;
std::vector<TwitterConnect::Twts>				mFavorites;
u32												mFavoritesCount = 200;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFavorites(std::vector<TwitterConnect::Twts>& favs, const std::string& nexttoken);
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFavorites, copyUserList)
END_DECLARE_COREFSMSTATE()

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

// wait state
START_DECLARE_COREFSMSTATE(TwitterAnalyser, Wait)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// everything is done
START_DECLARE_COREFSMSTATE(TwitterAnalyser, Done)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFollow)
public:
	// the user to retrieve follows 
	u64						userid;
	std::vector<u64>		userlist;
	std::string				followtype;
	int						limitCount = -1;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFollow(std::vector<u64>& follow, const std::string& nexttoken);
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFollow, copyUserList)
END_DECLARE_COREFSMSTATE()


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveTweets)
public:
	bool								mExcludeRetweets = false;
	bool								mExcludeReplies = false;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()