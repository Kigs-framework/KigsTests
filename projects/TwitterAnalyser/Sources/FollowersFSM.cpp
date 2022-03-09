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

}


void	CoreFSMStartMethod(TwitterAnalyser, GetFollow)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetFollow)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetFollow))
{
	
	// enough user
	if (mValidUserCount == mUserPanelSize)
	{
		GetUpgrador()->activateTransition("donetransition");
		return false;
	}

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

