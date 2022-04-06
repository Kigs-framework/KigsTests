#include "TwitterAnalyser.h"
#include "CoreFSMState.h"
#include "CoreFSM.h"
#include "CommonTwitterFSMStates.h"

START_DECLARE_COREFSMSTATE(TwitterAnalyser, UpdateTopStats)
COREFSMSTATE_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()



void TwitterAnalyser::TopFSM(const std::string& laststate)
{
	SP<CoreFSM> fsm = mFsm;
	
	// generic get user data transition
	SP<CoreFSMTransition> getuserdatatransition = KigsCore::GetInstanceOf("getuserdatatransition", "CoreFSMInternalSetTransition");
	getuserdatatransition->setValue("TransitionBehavior", "Push");
	getuserdatatransition->setState("UpdateTopStats");
	getuserdatatransition->Init();

	fsm->getState(laststate)->addTransition(getuserdatatransition);

	// when going to getuserdatatransition, retreive user list from previous state
	KigsCore::Connect(getuserdatatransition.get(), "ExecuteTransition", this, "setUserList", [this](CoreFSMTransition* t, CoreFSMStateBase* from)
		{
			SimpleCall("copyUserList", mUserList);
		});
	
	// create UpdateStats state
	fsm->addState("UpdateTopStats", new CoreFSMStateClass(TwitterAnalyser, UpdateTopStats)());

	// create GetUserDetail transition (Push)
	SP<CoreFSMTransition> getuserdetailtransition = KigsCore::GetInstanceOf("getuserdetailtransition", "CoreFSMInternalSetTransition");
	getuserdetailtransition->setValue("TransitionBehavior", "Push");
	getuserdetailtransition->setState("GetUserDetail");
	getuserdetailtransition->Init();

	// getFavorites -> user detail, wait or pop
	fsm->getState("UpdateTopStats")->addTransition(getuserdetailtransition);

	fsm->getState("UpdateTopStats")->addTransition(mTransitionList["waittransition"]);
	// GetFavorites can also go to NeedUserListDetail
	fsm->getState("UpdateTopStats")->addTransition(mTransitionList["userlistdetailtransition"]);
	fsm->getState("UpdateTopStats")->addTransition(mTransitionList["popwhendone"]);

	fsm->addState("GetUserDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserDetail)());

	// GetUserDetail can only wait (or pop)
	fsm->getState("GetUserDetail")->addTransition(mTransitionList["waittransition"]);

}

void	CoreFSMStartMethod(TwitterAnalyser, UpdateTopStats)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, UpdateTopStats)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, UpdateTopStats))
{
	std::vector<u64>& userlist = mUserList;
	
	switch (mPanelType)
	{
		case dataType::Likers:
		case dataType::Favorites:
		{
			for (auto u : userlist)
			{
				if (mUsersUserCount.find(u) == mUsersUserCount.end())
				{
					TwitterConnect::UserStruct	toAdd;
					toAdd.mW = 0.0f;
					mUsersUserCount[u] = std::pair<unsigned int, TwitterConnect::UserStruct>(1, toAdd);
				}
				else
				{
					mUsersUserCount[u].first++;
				}
			}

			mValidUserCount = mUsersUserCount.size();
			mUserPanelSize = mValidUserCount;
			mCurrentTreatedTweetIndex++;
			userlist.clear();
			mCurrentTreatedUserIndex = 0;
			break;
		}

		case dataType::Followers:
		case dataType::Following:
		{
			if (mCurrentTreatedUserIndex < userlist.size())
			{
				u64 user = userlist[mCurrentTreatedUserIndex];

				TwitterConnect::UserStruct	newuser;
				if (!TwitterConnect::LoadUserStruct(user, newuser, false))
				{
					newuser.mID = 0;
					mRetreivedUsers.push_back(newuser);
					mCurrentUserIndex = mRetreivedUsers.size() - 1;
					mUserToUserIndex[user] = mCurrentUserIndex;

					SP<CoreFSM> fsm = mFsm;

					mRetreivedUsers[mCurrentUserIndex].mID = user;
					auto userDetailState = getFSMState(fsm, TwitterAnalyser, GetUserDetail);

					userDetailState->nextTransition = "Pop";

					GetUpgrador()->activateTransition("getuserdetailtransition");
				}
				else
				{
					TwitterConnect::UserStruct	toAdd;
					mUsersUserCount[user] = std::pair<unsigned int, TwitterConnect::UserStruct>(newuser.mFollowersCount, toAdd);
					mCurrentTreatedUserIndex++;
					mTreatedUserCount++;
					if(newuser.mFollowersCount)
						mValidUserCount++;
				}
			}
			else
			{
				mUserPanelSize = mValidUserCount;
			}
			break;

		}
	}

	return false;
}