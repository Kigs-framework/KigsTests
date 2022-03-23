#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"


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


// retrieve favorites
START_DECLARE_COREFSMSTATE(TwitterAnalyser, GetFollowData)
public:
	u64									userid;
	std::vector<u64>					followData;
	std::string							followtype;
protected:
	STARTCOREFSMSTATE_WRAPMETHODS();
	void	manageRetrievedFollowData(std::vector<u64>& followdata	, const std::string& nexttoken);
	ENDCOREFSMSTATE_WRAPMETHODS(manageRetrievedFollowData)
END_DECLARE_COREFSMSTATE()

// update statistics
START_DECLARE_COREFSMSTATE(TwitterAnalyser, UpdateStats)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

std::string TwitterAnalyser::searchFollowFSM(const std::string& followtype)
{
	SP<CoreFSM> fsm = mFsm;

	// Init state, check if user was already started and launch next steps
	// no hashtag for follower
	fsm->addState("Init", new CoreFSMStateClass(TwitterAnalyser, InitUser)());

	// go to GetTweets
	SP<CoreFSMTransition> getfollowtransition = KigsCore::GetInstanceOf("getfollowtransition", "CoreFSMInternalSetTransition");
	getfollowtransition->setState("GetFollow");
	getfollowtransition->Init();

	// when going to GetFollow, set userid first
	KigsCore::Connect(getfollowtransition.get(), "ExecuteTransition", this, "setUserID", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto  followState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFollow);
			followState->userid = mRetreivedUsers[0].mID;
			followState->limitCount = mWantedTotalPanelSize;
		});

	// Init can go to Wait or GetFollow
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(getfollowtransition);

	// create GetFollow state
	fsm->addState("GetFollow", new CoreFSMStateClass(TwitterAnalyser, GetFollow)());
	// after GetFollow, can go to get user data (or pop)
	fsm->getState("GetFollow")->addTransition(mTransitionList["waittransition"]);
	// GetFollow can also go to NeedUserListDetail or done
	fsm->getState("GetFollow")->addTransition(mTransitionList["userlistdetailtransition"]);
	fsm->getState("GetFollow")->addTransition(mTransitionList["donetransition"]);

	auto toinit=getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFollow);
	toinit->followtype = followtype;

	return "GetFollow";
	
}

void	TwitterAnalyser::analyseFollowFSM(const std::string& lastState, const std::string& followtype)
{
	SP<CoreFSM> fsm = mFsm;

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "Push");
	getuserdatatransition->setState("GetFollowData");
	getuserdatatransition->Init();

	fsm->getState(lastState)->addTransition(getuserdatatransition);

	// when going to getuserdatatransition, retreive user list from previous state
	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserList", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mUserList);
		});


	// create GetFollowData state
	fsm->addState("GetFollowData", new CoreFSMStateClass(TwitterAnalyser, GetFollowData)());

	auto toinit = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFollowData);
	toinit->followtype = followtype;

	// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();
	// getFavorites -> user detail, wait or pop
	fsm->getState("GetFollowData")->addTransition(getuserdetailtransition);

	fsm->getState("GetFollowData")->addTransition(mTransitionList["waittransition"]);
	// GetFollowData can also go to NeedUserListDetail
	fsm->getState("GetFollowData")->addTransition(mTransitionList["userlistdetailtransition"]);
	// GetFollowData must pop when enough data
	fsm->getState("GetFollowData")->addTransition(mTransitionList["popwhendone"]);


	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());

	// create updateLikesStatistics transition (Push)
	SP<CoreFSMTransition> updatestatstransition = KigsCore::GetInstanceOf("updatestatstransition", "CoreFSMInternalSetTransition");
	updatestatstransition->setValue("TransitionBehavior", "Push");
	updatestatstransition->setState("UpdateStats");
	updatestatstransition->Init();


	// GetUserDetail can go to UpdateLikesStats, wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetUserDetail")->addTransition(updatestatstransition);

	// create UpdateLikesStats state
	fsm->addState("UpdateStats", new CoreFSMStateClass(TwitterAnalyser, UpdateStats)());
	// no transition here, only pop

}


void	CoreFSMStartMethod(TwitterAnalyser, GetFollow)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFollow)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFollow))
{
	
	std::string filenamenext_token = "Cache/Users/"+TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid)+"/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_" + GetUpgrador() ->followtype +"_NextCursor.json";

	CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

	std::string next_cursor = "-1";

	std::vector<u64>	 v;
	bool				hasFollowFile = false;
	if (nextt)
	{
		next_cursor = nextt["next-cursor"];
	}
	else
	{
		hasFollowFile = TwitterConnect::LoadFollowFile(GetUpgrador()->userid,v, GetUpgrador()->followtype);
	}

	if (GetUpgrador()->limitCount != -1)
	{
		// check limit count
		if (hasFollowFile && (v.size() >= GetUpgrador()->limitCount)) // if enough, set next_cursor to -1
		{
			next_cursor = "-1";
		}
	}

	if ((!hasFollowFile) || (next_cursor != "-1"))
	{
		mTwitterConnect->mNextCursor = next_cursor;
		KigsCore::Connect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollow");
		mTwitterConnect->launchGetFollow(GetUpgrador()->userid, GetUpgrador()->followtype);
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

void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollow)::manageRetrievedFollow(std::vector<u64>& follow, const std::string& nexttoken)
{
	std::string filenamenext_token = "Cache/Users/" + TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid) + "/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_" + GetUpgrador()->followtype + "_NextCursor.json";

	std::vector<u64> v;
	bool fexist = TwitterConnect::LoadFollowFile(GetUpgrador()->userid,v, GetUpgrador()->followtype);
	v.insert(v.end(), follow.begin(), follow.end());

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

	KigsCore::Disconnect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollow");
	TwitterConnect::SaveFollowFile(GetUpgrador()->userid, v, GetUpgrador()->followtype);

	requestDone();
}

void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollow)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->userlist);
}


void	CoreFSMStartMethod(TwitterAnalyser, UpdateStats)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, UpdateStats)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, UpdateStats))
{
	std::vector<u64>& userlist = mUserList;

	auto user = userlist[mCurrentTreatedUserIndex];
	u64 userID = mRetreivedUsers[mUserToUserIndex[user]].mID;

	if (!TwitterConnect::LoadUserStruct(userID, mRetreivedUsers[mUserToUserIndex[user]], false))
	{
		askUserDetail(userID);
	}

	auto  followDataState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, GetFollowData);

	const auto& currentData = followDataState->followData;

	std::vector<u64>& currentUserFollowData = mCheckedUserList[userID];

	for (auto f : currentData)
	{
		currentUserFollowData.push_back(f);

		auto alreadyfound = mUsersUserCount.find(f);
		if (alreadyfound != mUsersUserCount.end())
		{
			(*alreadyfound).second.first++;
		}
		else
		{
			TwitterConnect::UserStruct	toAdd;
			toAdd.mW = 0.0f;

			mUsersUserCount[f] = std::pair<unsigned int, TwitterConnect::UserStruct>(1, toAdd);
		}
	}

	// this one is done
	mCurrentTreatedUserIndex++;
	mValidUserCount++;
	mTreatedUserCount++;

	GetUpgrador()->popState();

	return false;
}

void	CoreFSMStartMethod(TwitterAnalyser, GetFollowData)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFollowData)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFollowData))
{

	std::vector<u64>& userlist = mUserList;

	if (mCurrentTreatedUserIndex < userlist.size()) 
	{
		auto user = userlist[mCurrentTreatedUserIndex];
		auto found = mFoundUser.find(user);

		if (found != mFoundUser.end()) // this one was already treated
		{
			mCurrentTreatedUserIndex++; // goto next one
			return false;
		}

		GetUpgrador()->userid = user;

		std::string filenamenext_token = "Cache/Users/" + TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid) + "/";
		filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_" + GetUpgrador()->followtype + "_NextCursor.json";

		CoreItemSP nextt = TwitterConnect::LoadJSon(filenamenext_token);

		std::string next_cursor = "-1";
		GetUpgrador()->followData.clear();

		bool				hasFollowFile = false;
		if (nextt)
		{
			next_cursor = nextt["next-cursor"];
		}
		else
		{
			hasFollowFile = TwitterConnect::LoadFollowFile(GetUpgrador()->userid, GetUpgrador()->followData, GetUpgrador()->followtype);
		}

		if ((!hasFollowFile) || (next_cursor != "-1"))
		{
			mTwitterConnect->mNextCursor = next_cursor;
			KigsCore::Connect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollowData");
			mTwitterConnect->launchGetFollow(GetUpgrador()->userid, GetUpgrador()->followtype);
			GetUpgrador()->activateTransition("waittransition");
			mNeedWait = true;
		}
		else
		{
			mFoundUser.insert(user);
			// if favorites were retrieved
			if (GetUpgrador()->followData.size())
			{
				TwitterConnect::UserStruct	newuser;
				newuser.mID = 0;
				mRetreivedUsers.push_back(newuser);
				mCurrentUserIndex = mRetreivedUsers.size() - 1;
				mUserToUserIndex[user] = mCurrentUserIndex;

				SP<CoreFSM> fsm = mFsm;

				mRetreivedUsers[mCurrentUserIndex].mID = user;
				auto userDetailState = getFSMState(fsm, TwitterAnalyser, GetUserDetail);
				userDetailState->nextTransition = "updatestatstransition";
				GetUpgrador()->activateTransition("getuserdetailtransition");

			}
			else
			{
				mCurrentTreatedUserIndex++; // goto next one
				mTreatedUserCount++;
			}
		}

	}
	else 
	{	
		GetUpgrador()->popState();
	}

	return false;
}


void	CoreFSMStateClassMethods(TwitterAnalyser, GetFollowData)::manageRetrievedFollowData(std::vector<u64>& follow, const std::string& nexttoken)
{

	std::string filenamenext_token = "Cache/Users/" + TwitterConnect::GetUserFolderFromID(GetUpgrador()->userid) + "/";
	filenamenext_token += TwitterConnect::GetIDString(GetUpgrador()->userid) + "_" + GetUpgrador()->followtype + "_NextCursor.json";

	std::vector<u64> v;
	bool fexist = TwitterConnect::LoadFollowFile(GetUpgrador()->userid, v, GetUpgrador()->followtype);
	v.insert(v.end(), follow.begin(), follow.end());

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

	KigsCore::Disconnect(mTwitterConnect.get(), "FollowRetrieved", this, "manageRetrievedFollowData");
	TwitterConnect::SaveFollowFile(GetUpgrador()->userid, v, GetUpgrador()->followtype);

	requestDone();
}