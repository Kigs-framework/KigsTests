#include "GraphDrawer.h"
#include "TwitterConnect.h"
#include "TwitterAnalyser.h"
#include "UI/UIImage.h"
#include "CoreFSM.h"

IMPLEMENT_CLASS_INFO(GraphDrawer)

GraphDrawer::GraphDrawer(const kstl::string& name, CLASS_NAME_TREE_ARG) : CoreModifiable(name, PASS_CLASS_NAME_TREE_ARG)
{

}

void GraphDrawer::InitModifiable()
{
	if (IsInit())
		return;

	CoreModifiable::InitModifiable();

	// create FSM to manage different states
	SP<CoreFSM> fsm = KigsCore::GetInstanceOf("fsm", "CoreFSM");
	// need to add fsm to the object to control
	addItem(fsm);



	// create Percent state
	fsm->addState("Percent", new CoreFSMStateClass(GraphDrawer, Percent)());

	SP<CoreFSMTransition> percentnexttransition = KigsCore::GetInstanceOf("percentnexttransition", "CoreFSMOnValueTransition");
	percentnexttransition->setValue("ValueName", "GoNext");
	
	if (mHasJaccard)
	{
		// create Jaccard state
		fsm->addState("Jaccard", new CoreFSMStateClass(GraphDrawer, Jaccard)());

		SP<CoreFSMTransition> jaccardnexttransition = KigsCore::GetInstanceOf("jaccardnexttransition", "CoreFSMOnValueTransition");
		jaccardnexttransition->setValue("ValueName", "GoNext");
		jaccardnexttransition->setState("Normalized");
		jaccardnexttransition->Init();

		fsm->getState("Jaccard")->addTransition(jaccardnexttransition);
	}
	else
	{
		percentnexttransition->setState("Normalized");
	}

	percentnexttransition->Init();

	fsm->getState("Percent")->addTransition(percentnexttransition);

	// create Normalized state
	fsm->addState("Normalized", new CoreFSMStateClass(GraphDrawer, Normalized)());

	SP<CoreFSMTransition> normalizednexttransition = KigsCore::GetInstanceOf("normalizednexttransition", "CoreFSMOnValueTransition");
	normalizednexttransition->setValue("ValueName", "GoNext");
	normalizednexttransition->setState("Percent");
	normalizednexttransition->Init();
	fsm->getState("Normalized")->addTransition(normalizednexttransition);

	/*
	// create UserStats state
	fsm->addState("UserStats", new CoreFSMStateClass(GraphDrawer, UserStats)());

	// create UserStats state
	fsm->addState("Force", new CoreFSMStateClass(GraphDrawer, Force)());
	*/
	fsm->setStartState("Percent");
	fsm->Init();

	mTwitterAnalyser = static_cast<TwitterAnalyser*>(KigsCore::GetCoreApplication());
	mTwitterAnalyser->AddAutoUpdate(this, 1.0);
	//mTwitterAnalyser->ChangeAutoUpdateFrequency(fsm.get(), 1.0);
}

void	GraphDrawer::drawSpiral(std::vector<std::tuple<unsigned int,float, u64> >&	toShow)
{
	drawGeneralStats();
	if (mTwitterAnalyser->mValidUserCount < 10)
		return;

	int toShowCount = 0;
	float dangle = 2.0f * KFLOAT_CONST_PI / 7.0f;
	float angle = 0.0f;
	float ray = 0.15f;
	float dray = 0.0117f;
	for (const auto& toS : toShow)
	{
		if (std::get<2>(toS) == mTwitterAnalyser->mRetreivedUsers[0].mID)
		{
			continue;
		}

		auto& toPlace = mTwitterAnalyser->mUsersUserCount[std::get<2>(toS)];

		auto found = mShowedUser.find(std::get<2>(toS));
		if (found != mShowedUser.end())
		{
			const CMSP& toSetup = (*found).second;
			v2f dock(0.53f + ray * cosf(angle), 0.47f + ray * 1.02f * sinf(angle));
			toSetup("Dock") = dock;
			angle += dangle;
			dangle = 2.0f * KFLOAT_CONST_PI / (2.0f + 50.0f * ray);
			ray += dray;
			dray *= 0.98f;
			if (toPlace.second.mName.length())
			{
				toSetup["ChannelName"]("Text") = toPlace.second.mName;
			}
			else
			{
				if (!TwitterConnect::LoadUserStruct(std::get<2>(toS), toPlace.second, true))
				{
					mTwitterAnalyser->askUserDetail(std::get<2>(toS));
				}
			}

			float prescale = 1.0f;

			int score = (int)std::get<1>(toS);
			std::string scorePrint;

			scorePrint = std::to_string(score) + mUnits[mCurrentUnit];

			toSetup["ChannelPercent"]("Text") = scorePrint;

			prescale = score / 80.0f;
			
			prescale = sqrtf(prescale);
			if (prescale > 1.0f)
			{
				prescale = 1.0f;
			}

			prescale *= 1.2f;

			// set ChannnelPercent position depending on where the thumb is in the spiral

			if ((dock.x > 0.45f) && (dock.x < 0.61f))
			{
				toSetup["ChannelPercent"]("Dock") = v2f(0.5, 0.0);
				toSetup["ChannelPercent"]("Anchor") = v2f(0.5, 1.0);
			}
			else if (dock.x <= 0.45f)
			{
				toSetup["ChannelPercent"]("Dock") = v2f(1.0, 0.5);
				toSetup["ChannelPercent"]("Anchor") = v2f(0.0, 0.5);
			}
			else
			{
				toSetup["ChannelPercent"]("Dock") = v2f(0.0, 0.5);
				toSetup["ChannelPercent"]("Anchor") = v2f(1.0, 0.5);
			}

			toSetup("PreScale") = v2f(1.44f * prescale, 1.44f * prescale);

			toSetup("Radius") = ((v2f)toSetup("Size")).x * 1.44f * prescale * 0.5f;

			toSetup["ChannelName"]("PreScale") = v2f(1.0f / (1.44f * prescale), 1.0f / (1.44f * prescale));

			toSetup["ChannelPercent"]("FontSize") = 0.6f * 24.0f / prescale;
			toSetup["ChannelPercent"]("MaxWidth") = 0.6f * 100.0f / prescale;

			toSetup["ChannelPercent"]("PreScale") = v2f(0.8f, 0.8f);
			toSetup["ChannelPercent"]("FontSize") = 0.6f * 24.0f / prescale;
			toSetup["ChannelPercent"]("MaxWidth") = 0.8f * 100.0f / prescale;
				
			toSetup["ChannelName"]("FontSize") = 20.0f;
			toSetup["ChannelName"]("MaxWidth") = 160.0f;

			const SP<UIImage>& checkTexture = toSetup;

			if (toPlace.second.mName.length())
			{
				if (!checkTexture->HasTexture())
				{
					//somethingChanged = true;
					if (toPlace.second.mThumb.mTexture)
					{
						checkTexture->addItem(toPlace.second.mThumb.mTexture);
					}
					else
					{
						TwitterConnect::LoadUserStruct(std::get<2>(toS), toPlace.second, true);
					}
				}
			}

		}
		toShowCount++;
		if (toShowCount >= mTwitterAnalyser->mMaxUserCount)
			break;
	}
	
}
void	GraphDrawer::drawForce()
{

}
void	GraphDrawer::drawStats()
{

}

void	GraphDrawer::nextDrawType()
{
	mGoNext = true;
}

void	GraphDrawer::drawGeneralStats()
{
	char textBuffer[256];
	if (mTwitterAnalyser->mUseLikes)
	{
		sprintf(textBuffer, "Treated Likers : %d", mTwitterAnalyser->mTreatedUserCount);
	}
	else
	{
		sprintf(textBuffer, "Treated Users : %d", mTwitterAnalyser->mTreatedUserCount);
	}
	mMainInterface["TreatedFollowers"]("Text") = textBuffer;

	if (mTwitterAnalyser->mUseLikes)
	{
		sprintf(textBuffer, "Liked user count : %d", (int)mTwitterAnalyser->mUsersUserCount.size());
	}
	else
	{
		sprintf(textBuffer, "Found followings : %d", (int)mTwitterAnalyser->mUsersUserCount.size());
	}
	mMainInterface["FoundFollowings"]("Text") = textBuffer;

	if (mTwitterAnalyser->mUseLikes)
	{
		sprintf(textBuffer, "Invalid likers count : %d", mTwitterAnalyser->mTreatedUserCount - mTwitterAnalyser->mValidUserCount);
	}
	else
	{
		sprintf(textBuffer, "Inactive Followers : %d", mTwitterAnalyser->mTreatedUserCount - mTwitterAnalyser->mValidUserCount);
	}
	mMainInterface["FakeFollowers"]("Text") = textBuffer;

	if (!mEverythingDone)
	{
		sprintf(textBuffer, "Twitter API requests : %d", mTwitterAnalyser->mTwitterConnect->getRequestCount());
		mMainInterface["RequestCount"]("Text") = textBuffer;

		if (mTwitterAnalyser->mTwitterConnect->mWaitQuota)
		{
			sprintf(textBuffer, "Wait quota count: %d", mTwitterAnalyser->mTwitterConnect->mWaitQuotaCount);
		}
		else
		{
			double requestWait = mTwitterAnalyser->mTwitterConnect->getDelay();
			if (requestWait < 0.0)
			{
				requestWait = 0.0;
			}
			sprintf(textBuffer, "Next request in : %f", requestWait);
		}


		mMainInterface["RequestWait"]("Text") = textBuffer;
	}
	else
	{
		mMainInterface["RequestCount"]("Text") = "";
		mMainInterface["RequestWait"]("Text") = "";
		mMainInterface["switchForce"]("IsHidden") = false;
		mMainInterface["switchForce"]("IsTouchable") = true;
	}

	if (mTwitterAnalyser->mRetreivedUsers[0].mThumb.mTexture && mMainInterface["thumbnail"])
	{
		const SP<UIImage>& tmp = mMainInterface["thumbnail"];

		if (!tmp->HasTexture())
		{
			tmp->addItem(mTwitterAnalyser->mRetreivedUsers[0].mThumb.mTexture);
			mMainInterface["thumbnail"]["UserName"]("Text") = mTwitterAnalyser->mRetreivedUsers[0].mName;
		}
	}
	else if (mMainInterface["thumbnail"])
	{
		TwitterConnect::LoadUserStruct(mTwitterAnalyser->mRetreivedUsers[0].mID, mTwitterAnalyser->mRetreivedUsers[0], true);
	}
}


void CoreFSMStartMethod(GraphDrawer, Percent)
{
	mGoNext = false;
	mCurrentUnit = 0;
}

void CoreFSMStopMethod(GraphDrawer, Percent)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(GraphDrawer, Percent))
{
	float wantedpercent = mTwitterAnalyser->mValidUserPercent;

	std::vector<std::tuple<unsigned int,float, u64>>	toShow;
	for (auto c : mTwitterAnalyser->mUsersUserCount)
	{
		if (c.first != mTwitterAnalyser->mRetreivedUsers[0].mID)
		{
			if (c.second.first > 3)
			{
				float percent = (float)c.second.first / (float)mTwitterAnalyser->mValidUserCount;
				if (percent > wantedpercent)
				{
					toShow.push_back({ c.second.first,percent*100.0f,c.first });
				}
			}
		}
	}

	if (toShow.size() == 0)
	{
		return false;
	}
	std::sort(toShow.begin(), toShow.end(), [&](const std::tuple<unsigned int, float,u64>& a1, const std::tuple<unsigned int, float,u64>& a2)
		{
			if (std::get<0>(a1) == std::get<0>(a2))
			{
				return std::get<2>(a1) > std::get<2>(a2);
			}
			return (std::get<0>(a1) > std::get<0>(a2));
		}
	);

	std::unordered_map<u64, unsigned int>	currentShowedChannels;
	int toShowCount = 0;
	for (const auto& tos : toShow)
	{
		currentShowedChannels[std::get<2>(tos)] = 1;
		toShowCount++;

		const auto& a1User = mTwitterAnalyser->mUsersUserCount[std::get<2>(tos)];

		if (toShowCount >= mTwitterAnalyser->mMaxUserCount)
			break;
	}

	for (const auto& s : mShowedUser)
	{
		if (currentShowedChannels.find(s.first) == currentShowedChannels.end())
		{
			currentShowedChannels[s.first] = 0;
		}
		else
		{
			currentShowedChannels[s.first]++;
		}
	}

	// add / remove items
	for (const auto& update : currentShowedChannels)
	{
		if (update.second == 0) // to remove 
		{
			auto toremove = mShowedUser.find(update.first);
			mMainInterface->removeItem((*toremove).second);
			mShowedUser.erase(toremove);
			//somethingChanged = true;
		}
		else if (update.second == 1) // to add
		{
			std::string thumbName = "thumbNail_" + TwitterConnect::GetIDString(update.first);
			CMSP toAdd = CoreModifiable::Import("Thumbnail.xml", false, false, nullptr, thumbName);
			toAdd->AddDynamicAttribute<maFloat, float>("Radius", 1.0f);
			mShowedUser[update.first] = toAdd;
			mMainInterface->addItem(toAdd);
			//somethingChanged = true;
		}
	}

	drawSpiral(toShow);
	return false;
}


void CoreFSMStartMethod(GraphDrawer, Jaccard)
{
	mGoNext = false;
}

void CoreFSMStopMethod(GraphDrawer, Jaccard)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(GraphDrawer, Jaccard))
{
	/*float wantedpercent = mTwitterAnalyser->mValidUserPercent;

	std::vector<std::tuple<unsigned int, float, u64>>	toShow;
	for (auto c : mTwitterAnalyser->mUsersUserCount)
	{
		if (c.first != mTwitterAnalyser->mRetreivedUsers[0].mID)
		{
			if (c.second.first > 3)
			{
				float percent = (float)c.second.first / (float)mTwitterAnalyser->mValidUserCount;
				if (percent > wantedpercent)
				{
					toShow.push_back({ c.second.first,percent * 100.0f,c.first });
				}
			}
		}
	}

	if (toShow.size() == 0)
	{
		return false;
	}
	std::sort(toShow.begin(), toShow.end(), [&](const std::tuple<unsigned int, float, u64>& a1, const std::tuple<unsigned int, float, u64>& a2)
		{
			if (std::get<0>(a1) == std::get<0>(a2))
			{
				return std::get<2>(a1) > std::get<2>(a2);
			}
			return (std::get<0>(a1) > std::get<0>(a2));
		}
	);

	std::unordered_map<u64, unsigned int>	currentShowedChannels;
	int toShowCount = 0;
	for (const auto& tos : toShow)
	{
		currentShowedChannels[std::get<2>(tos)] = 1;
		toShowCount++;

		const auto& a1User = mTwitterAnalyser->mUsersUserCount[std::get<2>(tos)];

		if (toShowCount >= mTwitterAnalyser->mMaxUserCount)
			break;
	}

	for (const auto& s : mShowedUser)
	{
		if (currentShowedChannels.find(s.first) == currentShowedChannels.end())
		{
			currentShowedChannels[s.first] = 0;
		}
		else
		{
			currentShowedChannels[s.first]++;
		}
	}

	// add / remove items
	for (const auto& update : currentShowedChannels)
	{
		if (update.second == 0) // to remove 
		{
			auto toremove = mShowedUser.find(update.first);
			mMainInterface->removeItem((*toremove).second);
			mShowedUser.erase(toremove);
			//somethingChanged = true;
		}
		else if (update.second == 1) // to add
		{
			std::string thumbName = "thumbNail_" + TwitterConnect::GetIDString(update.first);
			CMSP toAdd = CoreModifiable::Import("Thumbnail.xml", false, false, nullptr, thumbName);
			toAdd->AddDynamicAttribute<maFloat, float>("Radius", 1.0f);
			mShowedUser[update.first] = toAdd;
			mMainInterface->addItem(toAdd);
			//somethingChanged = true;
		}
	}

	drawSpiral(toShow);*/
	return false;
}

void CoreFSMStartMethod(GraphDrawer, Normalized)
{
	mGoNext = false;
	mCurrentUnit = 2;
}

void CoreFSMStopMethod(GraphDrawer, Normalized)
{
}

DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(GraphDrawer, Normalized))
{
	float wantedpercent = mTwitterAnalyser->mValidUserPercent;

	std::vector<std::tuple<unsigned int, float, u64>>	toShow;
	for (auto c : mTwitterAnalyser->mUsersUserCount)
	{
		if (c.first != mTwitterAnalyser->mRetreivedUsers[0].mID)
		{
			if (c.second.first > 3)
			{
				float percent = (float)c.second.first / (float)mTwitterAnalyser->mValidUserCount;
				if (percent > wantedpercent)
				{
					toShow.push_back({ c.second.first,percent * 100.0f,c.first });
				}
			}
		}
	}

	

	if (toShow.size() == 0)
	{
		return false;
	}
	std::sort(toShow.begin(), toShow.end(), [&](const std::tuple<unsigned int, float, u64>& a1, const std::tuple<unsigned int, float, u64>& a2)
		{
			auto& a1User = mTwitterAnalyser->mUsersUserCount[std::get<2>(a1)];
			auto& a2User = mTwitterAnalyser->mUsersUserCount[std::get<2>(a2)];

			float a1fcount = (a1User.second.mFollowersCount < 10) ? logf(10.0f) : logf((float)a1User.second.mFollowersCount);
			float a2fcount = (a2User.second.mFollowersCount < 10) ? logf(10.0f) : logf((float)a2User.second.mFollowersCount);

			float A1_w = ((float)std::get<0>(a1) / a1fcount);
			float A2_w = ((float)std::get<0>(a2) / a2fcount);
			if (A1_w == A2_w)
			{
				return std::get<2>(a1) > std::get<2>(a2);
			}
			return (A1_w > A2_w);
		}
	);

	// once sorted, take the first one
	float NormalizeFollowersCountForShown = 1.0f;

	for (const auto& toS : toShow)
	{
		if (std::get<2>(toS) == mTwitterAnalyser->mRetreivedUsers[0].mID)
		{
			continue;
		}

		auto& toPlace = mTwitterAnalyser->mUsersUserCount[std::get<2>(toS)];
		float toplacefcount = (toPlace.second.mFollowersCount < 10) ? logf(10.0f) : logf((float)toPlace.second.mFollowersCount);
		NormalizeFollowersCountForShown = toplacefcount;
		break;
	}

	std::unordered_map<u64, unsigned int>	currentShowedChannels;
	int toShowCount = 0;
	for (auto& tos : toShow)
	{
		currentShowedChannels[std::get<2>(tos)] = 1;
		toShowCount++;

		// compute normalized percent
		float fpercent = std::get<1>(tos);
		int followcount = mTwitterAnalyser->mUsersUserCount[std::get<2>(tos)].second.mFollowersCount;
		float toplacefcount = (followcount < 10) ? logf(10.0f) : logf((float)followcount);
		fpercent *= NormalizeFollowersCountForShown / toplacefcount;

		std::get<1>(tos) = fpercent;

		if (toShowCount >= mTwitterAnalyser->mMaxUserCount)
			break;
	}

	for (const auto& s : mShowedUser)
	{
		if (currentShowedChannels.find(s.first) == currentShowedChannels.end())
		{
			currentShowedChannels[s.first] = 0;
		}
		else
		{
			currentShowedChannels[s.first]++;
		}
	}

	// add / remove items
	for (const auto& update : currentShowedChannels)
	{
		if (update.second == 0) // to remove
		{
			auto toremove = mShowedUser.find(update.first);
			mMainInterface->removeItem((*toremove).second);
			mShowedUser.erase(toremove);
			//somethingChanged = true;
		}
		else if (update.second == 1) // to add
		{
			std::string thumbName = "thumbNail_" + TwitterConnect::GetIDString(update.first);
			CMSP toAdd = CoreModifiable::Import("Thumbnail.xml", false, false, nullptr, thumbName);
			toAdd->AddDynamicAttribute<maFloat, float>("Radius", 1.0f);
			mShowedUser[update.first] = toAdd;
			mMainInterface->addItem(toAdd);
			//somethingChanged = true;
		}
	}

	drawSpiral(toShow);
	return false;
}
