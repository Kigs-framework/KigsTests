#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveUserFollow)
public:
	unsigned int			mStateStep = 0;
	u64						mUserID;
	std::vector<u64>		mUserlist;
	std::string				mFollowType;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
END_DECLARE_COREFSMSTATE()


START_DECLARE_COREFSMSTATE(TwitterAnalyser, RetrieveFollow)
public:
	unsigned int			mStateStep = 0;
	u64						mUserID;
	std::vector<u64>		mUserlist;
	std::string				mFollowType;
protected:
STARTCOREFSMSTATE_WRAPMETHODS();
void	copyUserList(std::vector<u64>& touserlist);
ENDCOREFSMSTATE_WRAPMETHODS(copyUserList)
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

	// go to retrieveuserfollow
	SP<CoreFSMTransition> retrieveuserfollowtransition = KigsCore::GetInstanceOf("retrieveuserfollowtransition", "CoreFSMInternalSetTransition");
	retrieveuserfollowtransition->setState("RetrieveUserFollow");
	retrieveuserfollowtransition->Init();

	// create getUserFollow transition (Push)
	SP<CoreFSMTransition> getuserfollowtransition = KigsCore::GetInstanceOf("getuserfollowtransition", "CoreFSMInternalSetTransition");
	getuserfollowtransition->setValue("TransitionBehavior", "Push");
	getuserfollowtransition->setState("GetUserFollow");
	getuserfollowtransition->Init();

	// when going to GetFollow, set userid first
	KigsCore::Connect(retrieveuserfollowtransition.get(), "ExecuteTransition", this, "setUserID", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			auto  followState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, RetrieveUserFollow);
			followState->mUserID = mRetreivedUsers[0].mID;
		});

	// Init can go to Wait or GetFollow
	fsm->getState("Init")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("Init")->addTransition(retrieveuserfollowtransition);

	// create RetrieveUserFollow state
	fsm->addState("RetrieveUserFollow", new CoreFSMStateClass(TwitterAnalyser, RetrieveUserFollow)());
	// GetFollow can also go to NeedUserListDetail or done
	fsm->getState("RetrieveUserFollow")->addTransition(getuserfollowtransition);
	fsm->getState("RetrieveUserFollow")->addTransition(mTransitionList["userlistdetailtransition"]);
	fsm->getState("RetrieveUserFollow")->addTransition(mTransitionList["donetransition"]);
	
	// create GetFollow state
	fsm->addState("GetUserFollow", new CoreFSMStateClass(TwitterAnalyser, GetFollow)());
	// after GetFollow, can go to get user data (or pop)
	fsm->getState("GetUserFollow")->addTransition(mTransitionList["waittransition"]);
	// GetFollow can also go to NeedUserListDetail or done
	fsm->getState("GetUserFollow")->addTransition(mTransitionList["userlistdetailtransition"]);


	auto toinit=getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, RetrieveUserFollow);
	toinit->mFollowType = followtype;

	return "RetrieveUserFollow";
	
}

void	TwitterAnalyser::analyseFollowFSM(const std::string& lastState, const std::string& followtype)
{
	SP<CoreFSM> fsm = mFsm;

	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "Push");
	getuserdatatransition->setState("RetrieveFollow");
	getuserdatatransition->Init();


	// create getFollow transition (Push)
	SP<CoreFSMTransition> getfollowtransition = KigsCore::GetInstanceOf("getfollowtransition", "CoreFSMInternalSetTransition");
	getfollowtransition->setValue("TransitionBehavior", "Push");
	getfollowtransition->setState("GetFollow");
	getfollowtransition->Init();

	fsm->getState(lastState)->addTransition(getuserdatatransition);

	// when going to getuserdatatransition, retreive user list from previous state
	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserList", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mUserList);
		});


	// create RetrieveFollow state
	fsm->addState("RetrieveFollow", new CoreFSMStateClass(TwitterAnalyser, RetrieveFollow)());

	auto toinit = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, RetrieveFollow);
	toinit->mFollowType = followtype;

	// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();
	// getFavorites -> user detail, wait or pop
	fsm->getState("RetrieveFollow")->addTransition(getfollowtransition);
	fsm->getState("RetrieveFollow")->addTransition(getuserdetailtransition);
	fsm->getState("RetrieveFollow")->addTransition(mTransitionList["waittransition"]);
	// RetrieveFollow can also go to NeedUserListDetail
	fsm->getState("RetrieveFollow")->addTransition(mTransitionList["userlistdetailtransition"]);
	// RetrieveFollow must pop when enough data
	fsm->getState("RetrieveFollow")->addTransition(mTransitionList["popwhendone"]);

	fsm->addState("GetFollow", new CoreFSMStateClass(TwitterAnalyser, GetFollow)());
	// GetFavorites can go to waittransition or pop
	fsm->getState("GetFollow")->addTransition(mTransitionList["waittransition"]);
	fsm->getState("GetFollow")->addTransition(mTransitionList["userlistdetailtransition"]);


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

	auto  followDataState = getFSMState(mFsm->as<CoreFSM>(), TwitterAnalyser, RetrieveFollow);

	const auto& currentData = followDataState->mUserlist;

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

void	CoreFSMStartMethod(TwitterAnalyser, RetrieveFollow)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveFollow)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveFollow))
{

	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}

	std::vector<u64>& userlist = mUserList;
	SP<CoreFSM> fsm = mFsm;
	auto getGetFollowState = getFSMState(fsm, TwitterAnalyser, GetFollow);
	if (GetUpgrador()->mStateStep == 0)
	{
		if (mCurrentTreatedUserIndex < userlist.size())
		{
			auto user = userlist[mCurrentTreatedUserIndex];
			auto found = mFoundUser.find(user);

			if (found != mFoundUser.end()) // this one was already treated
			{
				mCurrentTreatedUserIndex++; // goto next one
				return false;
			}

			GetUpgrador()->mUserID = user;

			getGetFollowState->userid = user;
			getGetFollowState->userlist.clear();
			getGetFollowState->limitCount = -1;
			getGetFollowState->followtype = GetUpgrador()->mFollowType;

			mFoundUser.insert(user);

			GetUpgrador()->activateTransition("getfollowtransition");

			GetUpgrador()->mStateStep = 1;
		}
		else
		{
			userlist.clear();
			GetUpgrador()->popState();
		}
	}
	else if (GetUpgrador()->mStateStep == 1)
	{
		GetUpgrador()->mUserlist = std::move(getGetFollowState->userlist);

		// if follow were retrieved
		if (GetUpgrador()->mUserlist.size())
		{
			TwitterConnect::UserStruct	newuser;
			newuser.mID = GetUpgrador()->mUserID;
			mRetreivedUsers.push_back(newuser);
			mCurrentUserIndex = mRetreivedUsers.size() - 1;
			mUserToUserIndex[GetUpgrador()->mUserID] = mCurrentUserIndex;

			SP<CoreFSM> fsm = mFsm;

			auto userDetailState = getFSMState(fsm, TwitterAnalyser, GetUserDetail);
			userDetailState->nextTransition = "updatestatstransition";
			GetUpgrador()->activateTransition("getuserdetailtransition");

		}
		else
		{
			mCurrentTreatedUserIndex++; // goto next one
			mTreatedUserCount++;
		}

		GetUpgrador()->mStateStep = 0;
	}

	return false;
}


void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveFollow)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}


void	CoreFSMStartMethod(TwitterAnalyser, RetrieveUserFollow)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, RetrieveUserFollow)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, RetrieveUserFollow))
{
	// if an active transition already exist, just activate it
	if (GetUpgrador()->hasActiveTransition(this))
	{
		return false;
	}

	SP<CoreFSM> fsm = mFsm;
	auto userfollow = ((CoreFSMStateClass(TwitterAnalyser, GetFollow)*)fsm->getState("GetUserFollow"));

	if (GetUpgrador()->mStateStep == 0)
	{
		userfollow->followtype = GetUpgrador()->mFollowType;
		userfollow->userid = GetUpgrador()->mUserID;

		GetUpgrador()->mStateStep = 1;
		GetUpgrador()->activateTransition("getuserfollowtransition");
	}
	else
	{

		GetUpgrador()->mUserlist.clear();
		GetUpgrador()->mUserlist = std::move(userfollow->userlist);

		GetUpgrador()->activateTransition("getuserdatatransition");

		GetUpgrador()->mStateStep = 0;
	}


	return false;
}

void	CoreFSMStateClassMethods(TwitterAnalyser, RetrieveUserFollow)::copyUserList(std::vector<u64>& touserlist)
{
	touserlist = std::move(GetUpgrador()->mUserlist);
}
