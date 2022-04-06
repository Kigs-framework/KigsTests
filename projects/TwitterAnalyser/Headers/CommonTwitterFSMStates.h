#pragma once
#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"

START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetTweets)
public:
	std::string							mUserName="";
	u64									mUserID=-1;
	u32									mNeededTweetCount=0;
	bool								mSearchTweets=false;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedTweets)
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

