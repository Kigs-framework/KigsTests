#include <TwitterAnalyser.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>
#include "HTTPRequestModule.h"
#include "JSonFileParser.h"
#include "CoreFSM.h"

IMPLEMENT_CLASS_INFO(TwitterAnalyser);

IMPLEMENT_CONSTRUCTOR(TwitterAnalyser)
{

}

void	TwitterAnalyser::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);

	SP<FilePathManager> pathManager = KigsCore::Singleton<FilePathManager>();
	pathManager->AddToPath(".", "xml");
	// when a path is given, search the file only with this path
	pathManager->setValue("StrictPath", true);
	pathManager->AddToPath(".", "json");

	CoreCreateModule(HTTPRequestModule, 0);

	// Init App
	JSonFileParser L_JsonParser;
	CoreItemSP initP = L_JsonParser.Get_JsonDictionary("launchParams.json");

	// generic twitter management class
	DECLARE_FULL_CLASS_INFO(KigsCore::Instance(), TwitterConnect, TwitterConnect, TwitterAnalyser);
	DECLARE_FULL_CLASS_INFO(KigsCore::Instance(), ScrapperManager, ScrapperManager, TwitterAnalyser);
	DECLARE_FULL_CLASS_INFO(KigsCore::Instance(), GraphDrawer, GraphDrawer, TwitterAnalyser);

	mTwitterConnect = KigsCore::GetInstanceOf("mTwitterConnect", "TwitterConnect");
	mTwitterConnect->initBearer(initP);

	mGraphDrawer = KigsCore::GetInstanceOf("mGraphDrawer", "GraphDrawer");

	auto SetMemberFromParam = [&](auto& x, const char* id) {
		if (initP[id]) x = initP[id].value<std::remove_reference<decltype(x)>::type>();
	};

	int oldFileLimitInDays = 3 * 30;
	SetMemberFromParam(oldFileLimitInDays, "OldFileLimitInDays");
	
	// check the kind of configuration we need

	SetMemberFromParam(mHashTag, "HashTag");
	if (mHashTag.length())
	{
		mUseHashTags = true;
	}
	else
	{
		TwitterConnect::UserStruct	mainuser;
		mainuser.mID = 0;
		mRetreivedUsers.push_back(mainuser);
		SetMemberFromParam(mRetreivedUsers[0].mName, "UserName");
	}
	SetMemberFromParam(mUseLikes, "UseLikes");
	SetMemberFromParam(mUserPanelSize, "UserPanelSize");
	SetMemberFromParam(mValidUserPercent, "ValidUserPercent");
	SetMemberFromParam(mWantedTotalPanelSize, "WantedTotalPanelSize");
	SetMemberFromParam(mMaxUserCount, "MaxUserCount");

	initCoreFSM();

	if (mUseLikes)
	{
		createLikersFSM();
		SetMemberFromParam(mMaxLikersPerTweet, "MaxLikersPerTweet");
	}
	else
	{
		createFollowersFSM();
	}
	
	mTwitterConnect->initConnection(60.0 * 60.0 * 24.0 * (double)oldFileLimitInDays);

	// connect done msg
	KigsCore::Connect(mTwitterConnect.get(), "done", this, "requestDone");

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
}

void	TwitterAnalyser::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();
}

void	TwitterAnalyser::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
}

void	TwitterAnalyser::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mMainInterface = GetFirstInstanceByName("UIItem", "Interface");
		mMainInterface["switchForce"]("IsHidden") = true;
		mMainInterface["switchForce"]("IsTouchable") = false;
		if (mUseLikes)
			mMainInterface["heart"]("IsHidden") = false;

		// launch fsm
		SP<CoreFSM> fsm = mFsm;
		fsm->setStartState("Init");
		fsm->Init();

		mGraphDrawer->setInterface(mMainInterface);
		mGraphDrawer->Init();
	}
}
void	TwitterAnalyser::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mGraphDrawer = nullptr;
		SP<CoreFSM> fsm = mFsm;
		mFsm = nullptr;
		removeItem(fsm);
		mMainInterface = nullptr;
		mRetreivedUsers.clear();
	}
}

void	TwitterAnalyser::requestDone()
{
	mNeedWait = false;
}

void TwitterAnalyser::mainUserDone()
{
	// save user
	JSonFileParser L_JsonParser;
	CoreItemSP initP = MakeCoreMap();
	initP->set("id", mRetreivedUsers[0].mID);
	std::string filename = "Cache/UserName/";
	filename += mRetreivedUsers[0].mName.ToString() + ".json";
	L_JsonParser.Export((CoreMap<std::string>*)initP.get(), filename);

	KigsCore::Disconnect(mTwitterConnect.get(), "MainUserDone", this, "mainUserDone");

	requestDone();
}

void	TwitterAnalyser::switchDisplay()
{
	mMainInterface["switchForce"]("IsHidden") = true;
	mMainInterface["switchForce"]("IsTouchable") = false;

	mGraphDrawer->nextDrawType();
}



void CoreFSMStartMethod(TwitterAnalyser, InitUser)
{
	mNameToUserIndex[mRetreivedUsers[0].mName.ToString()] = 0;
}

void CoreFSMStopMethod(TwitterAnalyser, InitUser)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, InitUser))
{
	std::string currentUserProgress = "Cache/";
	currentUserProgress += "UserName/";
	currentUserProgress += mRetreivedUsers[0].mName.ToString() + ".json";
	CoreItemSP currentP = mTwitterConnect->LoadJSon(currentUserProgress);

	if (!currentP) // new user
	{
		KigsCore::Connect(mTwitterConnect.get(), "MainUserDone", this, "mainUserDone");
		mTwitterConnect->launchUserDetailRequest(mRetreivedUsers[0].mName.ToString(), mRetreivedUsers[0],true, "MainUserDone");
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else // load current user
	{
		u64 userID = currentP["id"];
		mTwitterConnect->LoadUserStruct(userID, mRetreivedUsers[0], true);
		auto availableTransitions=GetUpgrador()->getTransitionList();
		for (const auto& t : availableTransitions)
		{
			// Init only have 2 transitions : "waittransition" and another
			// so active the other one
			if (t != "waittransition")
			{
				GetUpgrador()->activateTransition(t);
				break;
			}
		}
	}
	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, GetUserDetail)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetUserDetail)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetUserDetail))
{
	// back from next transition ?
	if (GetUpgrador()->nextTransition == "")
	{
		GetUpgrador()->popState();
		return false;
	}

	u64 userID = mRetreivedUsers[mCurrentUserIndex].mID;
	
	if (!mTwitterConnect->LoadUserStruct(userID, mRetreivedUsers[mCurrentUserIndex], false))
	{
		mTwitterConnect->launchUserDetailRequest(userID, mRetreivedUsers[mCurrentUserIndex], false);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else
	{
		GetUpgrador()->activateTransition(GetUpgrador()->nextTransition);
		// go to next only once
		GetUpgrador()->nextTransition = "";
	}
	return false;
}

void	CoreFSMStartMethod(TwitterAnalyser, GetUserID)
{
	
}
void	CoreFSMStopMethod(TwitterAnalyser, GetUserID)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetUserID))
{
	// back from next transition ?
	if (GetUpgrador()->nextTransition == "")
	{
		GetUpgrador()->popState();
		return false;
	}
	
	std::string filename = "Cache/Tweets/" + GetUpgrador()->userName.substr(0, 4) + "/" + GetUpgrador()->userName + ".json";
	// user id doesn't expire
	CoreItemSP currentUserJson = TwitterConnect::LoadJSon(filename,false);
	if (!currentUserJson)
	{
		mTwitterConnect->launchUserDetailRequest(GetUpgrador()->userName, mRetreivedUsers[mCurrentUserIndex],false);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
	}
	else
	{
		mRetreivedUsers[mCurrentUserIndex].mID = currentUserJson["id"];
		GetUpgrador()->activateTransition(GetUpgrador()->nextTransition);
		// go to next only once
		GetUpgrador()->nextTransition = "";
	}
	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, Wait)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, Wait)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, Wait))
{

	return false;
}

void	CoreFSMStartMethod(TwitterAnalyser, GetUserListDetail)
{

}
void	CoreFSMStopMethod(TwitterAnalyser, GetUserListDetail)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, GetUserListDetail))
{
	if (!mUserDetailsAsked.size())
	{
		mNeedUserListDetail = false;
		GetUpgrador()->popState();
		return false;
	}

	u64 userID=mUserDetailsAsked.back();

	if (!TwitterConnect::LoadUserStruct(userID, GetUpgrador()->mTmpUserStruct, false))
	{
		mTwitterConnect->launchUserDetailRequest(userID, GetUpgrador()->mTmpUserStruct, false);
		GetUpgrador()->activateTransition("waittransition");
		mNeedWait = true;
		mUserDetailsAsked.pop_back();
	}
	
	return false;
}


void	CoreFSMStartMethod(TwitterAnalyser, Done)
{
	mGraphDrawer->setEverythingDone();
}
void	CoreFSMStopMethod(TwitterAnalyser, Done)
{

}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, Done))
{
	
	return false;
}
