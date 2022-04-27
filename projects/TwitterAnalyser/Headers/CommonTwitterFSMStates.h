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
	u32									mNeededTweetCountIncrement = 50;
	bool								mSearchTweets=false;
	bool								mCantGetMoreTweets = false;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedTweets)
END_DECLARE_COREFSMSTATE()


// base state for retrieving user list
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetUsers)
public:

	// Tweet or user id to retrieve data for
	u64								mForID;

	TwitterAnalyser::UserList		mUserlist;
	// if 0 => try to get all of them
	// else try to get at least mNeededUserCount
	u32								mNeededUserCount = 0;
	// when mUserlist>mNeededUserCount, if mNeededUserCountIncrement == 0 then don't do more (pop)
	// else mNeededUserCount+=mNeededUserCountIncrement
	u32								mNeededUserCountIncrement=0;

	// set it to true when mNeededUserCount need to be incremented
	bool							mNeedMoreUsers = false;

	// set to true when no more user can be retreived
	bool							mCantGetMoreUsers = false;

protected:
COREFSMSTATE_WITHOUT_METHODS();
END_DECLARE_COREFSMSTATE()


// retrieve likes
START_INHERITED_COREFSMSTATE(TwitterAnalyser, GetLikers, GetUsers)
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedLikers(std::vector<u64>& TweetLikers, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedLikers)
END_DECLARE_COREFSMSTATE()


// retrieve likes
START_INHERITED_COREFSMSTATE(TwitterAnalyser, GetRetweeters, GetUsers)
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
void	copyUserList(TwitterAnalyser::UserList& touserlist);
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
	u64			mUserID;
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

START_INHERITED_COREFSMSTATE(TwitterAnalyser, GetFollow, GetUsers)
public:
	std::string						mFollowtype;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFollow(std::vector<u64>& follow, const std::string& nexttoken);
void	copyUserList(TwitterAnalyser::UserList& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFollow, copyUserList)
END_DECLARE_COREFSMSTATE()


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveTweets)
public:
	bool								mExcludeRetweets = false;
	bool								mExcludeReplies = false;
	std::string							mUserName="";
	u64									mUserID=0;
	bool								mUseHashtag = false;
	u32									mCurrentTreatedTweetIndex = 0;
	// for each tweet, first => indicated liker or RTter count , second => real liker or RTter retrieved
	std::map<u64, std::pair<u32,u32>>	mTweetRetrievedUserCount;
	std::vector<TwitterConnect::Twts>	mTweets;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

// update statistics
START_DECLARE_COREFSMSTATE(TwitterAnalyser, UpdateStats)
public:
	TwitterAnalyser::UserList		mUserlist;
protected:
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()