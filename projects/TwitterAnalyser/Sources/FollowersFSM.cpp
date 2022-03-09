#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"


START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFollowers)
public:
	// the user to retrieve followers
	u64						userid;
	std::vector<u64>		userlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFollowers(std::vector<u64>& followers, const std::string& nexttoken);
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFollowers, copyUserList)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFollowing)
public:
	// the user to retrieve followers
	u64						userid;
	std::vector<u64>		userlist;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	manageRetrievedFollowing(std::vector<u64>& following, const std::string& nexttoken);
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFollowing, copyUserList)
END_DECLARE_COREFSMSTATE()

std::string TwitterAnalyser::searchFollowersFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// Init state, check if user was already started and launch next steps
	// no hashtag for follower
	fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());

	// go to GetTweets
	SP<CoreFSMTransition> getfollowersstransition = KigsCore::GetInstanceOf("getfollowersstransition", "CoreFSMInternalSetTransition");
	getfollowersstransition->setState("GetFollowers");
	getfollowersstransition->Init();

	// when going to GetFollowers, set userid first
	KigsCore::Connect(getfollowersstransition.get(), "ExecuteTransition", this, "setUserID", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto  followersState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFollowers);
			followersState->userid = mRetreivedUsers[0].mID;
		});

	// Init can go to Wait or GetFollowers
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(getfollowersstransition);

	// create GetLikers state
	fsm->addState("GetFollowers", new CoreFSMStateClass(TwitterAnalyser, GetFollowers)());
	// after GetFollowers, can go to get user data (or pop)
	fsm->getState("GetFollowers")->addTransition(mTransitionList["waittransition"]);
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetFollowers")->addTransition(mTransitionList["userlistdetailtransition"]);

	return "GetFollowers";
	
}

std::string	TwitterAnalyser::searchFollowingFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// Init state, check if user was already started and launch next steps
	// no hashtag for follower
	fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());

	// go to GetTweets
	SP<CoreFSMTransition> getfollowingtransition = KigsCore::GetInstanceOf("getfollowingtransition", "CoreFSMInternalSetTransition");
	getfollowingtransition->setState("GetFollowing");
	getfollowingtransition->Init();

	// when going to GetFollowing, set userid first
	KigsCore::Connect(getfollowingtransition.get(), "ExecuteTransition", this, "setUserID", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto  followingState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFollowing);
			followingState->userid = mRetreivedUsers[0].mID;
		});

	// Init can go to Wait or GetFollowers
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(getfollowingtransition);

	// create GetLikers state
	fsm->addState("GetFollowing", new CoreFSMStateClass(TwitterAnalyser, GetFollowing)());
	// after GetFollowers, can go to get user data (or pop)
	fsm->getState("GetFollowing")->addTransition(mTransitionList["waittransition"]);
	// get likes can also go to NeedUserListDetail
	fsm->getState("GetFollowing")->addTransition(mTransitionList["userlistdetailtransition"]);

	return "GetFollowing";
}

void	TwitterAnalyser::analyseFollowersFSM(const std::string& lastState)
{

}
void	TwitterAnalyser::analyseFollowingFSM(const std::string& lastState)
{

}



void	CoreFSMStartMethod(TwitterAnalyser, GetFollowers)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFollowers)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFollowers))
{
	
	// enough user
	if (mValidUserCount == mUserPanelSize)
	{
		GetUpgrador()->popState();
		return false;
	}

	std::string filenamenext_token = "Cache/Users/"+TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid)+"/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_FollowersNextCursor.json";

	CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

	std::string next_cursor = "-1";

	std::vector<u64>	 v;
	bool				hasFollowerFile = false;
	if (nextt)
	{
		next_cursor = nextt["next-cursor"];
	}
	else
	{
		hasFollowerFile = TwitterConnect::LoadFollowersFile(GetUpgrador()->userid,v);
	}

	if ((!hasFollowerFile) || (next_cursor != "-1"))
	{
		mTwitterConnect->mNextCursor = next_cursor;
		KigsCore::Connect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollowers");
		mTwitterConnect->launchGetFollow(GetUpgrador()->userid,"followers");
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else
	{
		GetUpgrador()->userlist = std::move(v);
		GetUpgrador()->activateTransition("getuserdatatransition");
	}
	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollowers)::manageRetrievedFollowers(std::vector<u64>& followers, const std::string& nexttoken)
{
	std::string filenamenext_token = "Cache/Users/" + TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid) + "/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_FollowersNextCursor.json";

	std::vector<u64> v;
	bool fexist = TwitterConnect::LoadFollowersFile(GetUpgrador()->userid,v);
	v.insert(v.end(), followers.begin(), followers.end());

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

	KigsCore::Disconnect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollowers");
	TwitterConnect::SaveFollowersFile(GetUpgrador()->userid, v);

	requestDone();
}

void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollowers)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->userlist);
}

void	CoreFSMStartMethod(TwitterAnalyser, GetFollowing)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFollowing)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFollowing))
{

	// enough user
	if (mValidUserCount == mUserPanelSize)
	{
		GetUpgrador()->popState();
		return false;
	}

	std::string filenamenext_token = "Cache/Users/" + TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid) + "/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_FollowingNextCursor.json";

	CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

	std::string next_cursor = "-1";

	std::vector<u64>	 v;
	bool				hasFollowingFile = false;
	if (nextt)
	{
		next_cursor = nextt["next-cursor"];
	}
	else
	{
		hasFollowingFile = TwitterConnect::LoadFollowingFile(GetUpgrador()->userid, v);
	}

	if ((!hasFollowingFile) || (next_cursor != "-1"))
	{
		mTwitterConnect->mNextCursor = next_cursor;
		KigsCore::Connect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollowing");
		mTwitterConnect->launchGetFollow(GetUpgrador()->userid,"following");
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else
	{
		GetUpgrador()->userlist = std::move(v);
		GetUpgrador()->activateTransition("getuserdatatransition");
	}
	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollowing)::manageRetrievedFollowing(std::vector<u64>& following, const std::string& nexttoken)
{
	std::string filenamenext_token = "Cache/Users/" + TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid) + "/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_FollowingNextCursor.json";

	std::vector<u64> v;
	bool fexist = TwitterConnect::LoadFollowingFile(GetUpgrador()->userid, v);
	v.insert(v.end(), following.begin(), following.end());

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

	KigsCore::Disconnect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollowing");
	TwitterConnect::SaveFollowersFile(GetUpgrador()->userid, v);

	requestDone();
}
void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollowing)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->userlist);
}