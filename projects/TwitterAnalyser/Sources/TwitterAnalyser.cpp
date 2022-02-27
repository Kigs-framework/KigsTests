#include <TwitterAnalyser.h>
#include <FilePathManager.h>
#include <NotificationCenter.h>
#include "HTTPRequestModule.h"
#include "JSonFileParser.h"
#include "CoreFSM.h"
#include "TextureFileManager.h"

IMPLEMENT_CLASS_INFO(TwitterAnalyser);

IMPLEMENT_CONSTRUCTOR(TwitterAnalyser)
{

}

void		TwitterAnalyser::commonStatesFSM()
{
	SP<CoreFSM> fsm = mFsm;

	// go to wait state (push)
	SP<CoreFSMTransition> waittransition = KigsCore::GetInstanceOf("waittransition", "CoreFSMOnValueTransition");
	waittransition->setValue("TransitionBehavior", "Push");
	waittransition->setValue("ValueName", "NeedWait");
	waittransition->setState("Wait");
	waittransition->Init();

	mTransitionList["waittransition"] = waittransition;

	// this one is needed for all cases
	fsm->addState("GetUserListDetail", new CoreFSMStateClass(TwitterAnalyser, GetUserListDetail)());
	// only wait or pop
	fsm->getState("GetUserListDetail")->addTransition(waittransition);

	// go to GetUserListDetail state (push)
	SP<CoreFSMTransition> userlistdetailtransition = KigsCore::GetInstanceOf("userlistdetailtransition", "CoreFSMOnValueTransition");
	userlistdetailtransition->setValue("TransitionBehavior", "Push");
	userlistdetailtransition->setValue("ValueName", "NeedUserListDetail");
	userlistdetailtransition->setState("GetUserListDetail");
	userlistdetailtransition->Init();

	mTransitionList["userlistdetailtransition"] = userlistdetailtransition;

	// this one is needed for all cases
	fsm->addState("Done", new CoreFSMStateClass(TwitterAnalyser, Done)());
	// only wait or pop
	fsm->getState("Done")->addTransition(userlistdetailtransition);

	// pop wait state transition
	SP<CoreFSMTransition> waitendtransition = KigsCore::GetInstanceOf("waitendtransition", "CoreFSMOnValueTransition");
	waitendtransition->setValue("TransitionBehavior", "Pop");
	waitendtransition->setValue("ValueName", "NeedWait");
	waitendtransition->setValue("NotValue", true); // end wait when NeedWait is false
	waitendtransition->Init();

	mTransitionList["waitendtransition"] = waitendtransition;

	// create Wait state
	fsm->addState("Wait", new CoreFSMStateClass(TwitterAnalyser, Wait)());
	// Wait state can pop back to previous state
	fsm->getState("Wait")->addTransition(waitendtransition);

	// transition to done state
	SP<CoreFSMTransition> donetransition = KigsCore::GetInstanceOf("donetransition", "CoreFSMInternalSetTransition");
	donetransition->setState("Done");
	donetransition->Init();

	mTransitionList["donetransition"] = donetransition;

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
		TwitterConnect::UserStruct	mainuser;
		mainuser.mID = 0;
		mRetreivedUsers.push_back(mainuser);
		auto textureManager = KigsCore::Singleton<TextureFileManager>();
		mRetreivedUsers[0].mThumb.mTexture = textureManager->GetTexture("keyw.png");
		mRetreivedUsers[0].mName = mHashTag;
	}
	else
	{
		// look for dates (only if not hashtag)

		std::string fromdate, todate;
		SetMemberFromParam(fromdate, "FromDate");
		SetMemberFromParam(todate, "ToDate");

		if (fromdate.length() && todate.length())
		{
			TwitterConnect::initDates(fromdate, todate);
		}

		TwitterConnect::UserStruct	mainuser;
		mainuser.mID = 0;
		mRetreivedUsers.push_back(mainuser);
		SetMemberFromParam(mRetreivedUsers[0].mName, "UserName");
	}
	u32 PanelType;
	u32 AnalysedType;
	SetMemberFromParam(PanelType, "PanelType");
	mPanelType = (dataType)PanelType;
	SetMemberFromParam(AnalysedType, "AnalysedType");
	mAnalysedType = (dataType)AnalysedType;

	SetMemberFromParam(mUserPanelSize, "UserPanelSize");
	SetMemberFromParam(mValidUserPercent, "ValidUserPercent");
	SetMemberFromParam(mWantedTotalPanelSize, "WantedTotalPanelSize");
	SetMemberFromParam(mMaxUserCount, "MaxUserCount");

	initCoreFSM();

	std::string lastState;

	// add FSM
	SP<CoreFSM> fsm = KigsCore::GetInstanceOf("fsm", "CoreFSM");
	// need to add fsm to the object to control
	addItem(fsm);
	mFsm = fsm;

	commonStatesFSM();

	switch (mPanelType)
	{
	case dataType::Likers:
		lastState = searchLikersFSM();
		SetMemberFromParam(mMaxLikersPerTweet, "MaxLikersPerTweet");
		break;
	case dataType::Followers:
		lastState = searchFollowersFSM();
		break;
	case dataType::Following:
		lastState = searchFollowingFSM();
		break;
	}

	switch (mAnalysedType)
	{
	case dataType::Favorites:
		analyseFavoritesFSM(lastState);
		break;
	case dataType::Followers:
		analyseFollowersFSM(lastState);
		break;
	case dataType::Following:
		analyseFollowingFSM(lastState);
		break;
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
		if (mPanelType == dataType::Likers)
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

void	TwitterAnalyser::switchForce()
{
	bool currentDrawForceState=mGraphDrawer->getValue<bool>("DrawForce");
	if (!currentDrawForceState)
	{
		mMainInterface["thumbnail"]("Dock") = v2f(0.94f, 0.08f);

		mMainInterface["switchV"]("IsHidden") = true;
		mMainInterface["switchV"]("IsTouchable") = false;

		mMainInterface["switchForce"]("Dock") = v2f(0.050f, 0.950f);

	}
	else
	{
		mMainInterface["thumbnail"]("Dock") = v2f(0.52f, 0.44f);

		mMainInterface["switchV"]("IsHidden") = false;
		mMainInterface["switchV"]("IsTouchable") = true;

		mMainInterface["switchForce"]("IsHidden") = true;
		mMainInterface["switchForce"]("IsTouchable") = false;
		mMainInterface["switchForce"]("Dock") = v2f(0.950f, 0.050f);
	}
	currentDrawForceState = !currentDrawForceState;
	mGraphDrawer->setValue("DrawForce", currentDrawForceState);

}


void	TwitterAnalyser::manageRetrievedTweets(std::vector<TwitterConnect::Twts>& twtlist, const std::string& nexttoken)
{
	bool newtweet = false;
	if (twtlist.size())
	{
		TwitterConnect::randomizeVector(twtlist);
		for (const auto& twt : twtlist)
		{
			if (!TwitterConnect::searchDuplicateTweet(twt.mTweetID, mTweets))
			{
				mTweets.push_back(twt);
				newtweet = true;
			}
		}

	}
	if((newtweet == false) && (nexttoken == "-1"))// no more tweets can be retreived
	{
		if (mCanGetMoreUsers) // do it again with more users
		{
			mCanGetMoreUsers = false;
			mCurrentTreatedTweetIndex = 0;
		}
		else
		{
			mUserPanelSize = mValidUserCount;
		}
	}
	KigsCore::Disconnect(mTwitterConnect.get(), "TweetRetrieved", this, "manageRetrievedTweets");
	requestDone();

	std::string userfilename = mRetreivedUsers[0].mName.ToString();

	if (mUseHashTags)
	{
		userfilename = TwitterConnect::getHashtagFilename(mRetreivedUsers[0].mName.ToString());
	}

	TwitterConnect::SaveTweetsFile(mTweets, userfilename);

	{
		std::string filename = "Cache/UserName/";
		if (TwitterConnect::useDates())
		{
			filename += "_" + TwitterConnect::getDate(0) + "_" + TwitterConnect::getDate(1) + "_";
		}
		filename += userfilename + "_TweetsNextCursor.json";

		if (nexttoken != "-1")
		{

			CoreItemSP currentUserJson = MakeCoreMap();
			currentUserJson->set("next-cursor", nexttoken);
			TwitterConnect::SaveJSon(filename, currentUserJson);
		}
		else
		{
			ModuleFileManager::RemoveFile(filename.c_str());
		}
	}

}

void CoreFSMStartMethod(TwitterAnalyser, InitHashTag)
{
#ifdef USE_SCRAPPER
	mUserToUserIndex[mRetreivedUsers[0].mName.ToString()] = 0;
#endif
}

void CoreFSMStopMethod(TwitterAnalyser, InitHashTag)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, InitHashTag))
{
	// nothing more to do, just go to next state
	auto availableTransitions = GetUpgrador()->getTransitionList();
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
	return false;
}

void CoreFSMStartMethod(TwitterAnalyser, InitUser)
{
#ifdef USE_SCRAPPER
	mUserToUserIndex[mRetreivedUsers[0].mName.ToString()] = 0;
#endif
}

void CoreFSMStopMethod(TwitterAnalyser, InitUser)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(TwitterAnalyser, InitUser))
{
	std::string currentUserProgress = "Cache/";
	currentUserProgress += "UserName/";
	currentUserProgress += mRetreivedUsers[0].mName.ToString() + ".json";
	CoreItemSP currentP = TwitterConnect::LoadJSon(currentUserProgress);

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

#ifdef USE_SCRAPPER

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
#endif

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


