#include <windows.h>
#include <inttypes.h>
#include "TweetFrequency.h"
#include "FilePathManager.h"
#include "NotificationCenter.h"
#include "HTTPRequestModule.h"
#include "JSonFileParser.h"
#include "TinyImage.h"
#include "JPEGClass.h"
#include "PNGClass.h"
#include "GIFClass.h"
#include "TextureFileManager.h"
#include "UI/UIImage.h"
#include "AnonymousModule.h"
#include "KigsBitmap.h"
#include <iostream>

time_t     TweetFrequency::mCurrentTime=0;
double		TweetFrequency::mOldFileLimit = 0.0;

KigsBitmap::KigsBitmapPixel	userColors[4] = { {200,0,0,255},{0,155,0,255},{0,0,200,255},{180,0,180,255} };
v3f							userColorsf[4] = { {0.784f,0.0f,0.0f},{0.0f,0.608f,0.0f},{0,0,0.784f},{0.706f,0,0.706f} };

std::string	GetDate(int fromNowInSeconds)
{
	char yestDt[64];

	time_t now = time(NULL);
	now = now + fromNowInSeconds;
	struct tm* t = localtime(&now);

	sprintf(yestDt, "%d-%02d-%02dT",2000+t->tm_year-100,t->tm_mon+1, t->tm_mday);

	return yestDt;
}


template<typename T>
void	randomizeVector(std::vector<T>& v)
{
	unsigned int s = v.size();
	if (s>1)
	{
		unsigned int mod = s;
		for (unsigned int i = 0; i < (s - 1); i++)
		{
			unsigned int swapIndex = rand() % mod;

			T swap = v[mod - 1];
			v[mod - 1] = v[swapIndex];
			v[swapIndex] = swap;
			mod--;
		}
	}
}


IMPLEMENT_CLASS_INFO(TweetFrequency);

IMPLEMENT_CONSTRUCTOR(TweetFrequency)
{
	srand(time(NULL));
	mState.push_back(WAIT_STATE);
}

int getCreationYear(const std::string& created_date)
{
	char Day[6];
	char Mon[6];

	int	day_date;
	int	hours,minutes,seconds;
	int delta;
	int	year=0;

	sscanf(created_date.c_str(), "%s %s %d %d:%d:%d +%d %d", Day, Mon, &day_date, &hours, &minutes, &seconds,&delta ,&year);

	return year;
}


void	TweetFrequency::ProtectedInit()
{
	// Base modules have been created at this point

	// lets say that the update will sleep 1ms
	SetUpdateSleepTime(1);


	mCurrentTime = time(0);

	SYSTEMTIME	retrieveYear;
	GetSystemTime(&retrieveYear);
	mCurrentYear = retrieveYear.wYear;

	// here don't use files olders than three months.
	mOldFileLimit = 60.0 * 60.0 * 24.0 * 30.0 * 3.0;

	// Init App
	JSonFileParser L_JsonParser;
	CoreItemSP initP = L_JsonParser.Get_JsonDictionary("launchTFParams.json");

	// retreive parameters
	CoreItemSP foundBear;
	int bearIndex = 0;
	do
	{
		char	BearName[64];
		++bearIndex;
		sprintf(BearName, "TwitterBear%d", bearIndex);

		foundBear = initP[(std::string)BearName];

		if (!foundBear.isNil())
		{
			mTwitterBear.push_back("authorization: Bearer " + (std::string)foundBear);
			
		}
	} while (!foundBear.isNil());

	// retreive users to analyse
	CoreItemSP foundUser;
	int userIndex = 0;
	do
	{
		char	userName[64];
		++userIndex;
		sprintf(userName, "UserName%d", userIndex);

		foundUser = initP[(std::string)userName];

		if (!foundUser.isNil())
		{
			mUserNames.push_back(foundUser);

		}
	} while (!foundUser.isNil());


	auto SetMemberFromParam = [&](auto& x, const char* id) {
		if (!initP[id].isNil()) x = initP[id];
	};

	SetMemberFromParam(mFromDate, "FromDate");
	SetMemberFromParam(mToDate, "ToDate");


	if ((mFromDate == "") || (mToDate == "") )
	{
		mNeedExit = true;
		return;
	}

	int oldFileLimitInDays = 3 * 30;
	SetMemberFromParam(oldFileLimitInDays, "OldFileLimitInDays");

	mOldFileLimit = 60.0 * 60.0 * 24.0 * (double)oldFileLimitInDays;

	SP<FilePathManager>& pathManager = KigsCore::Singleton<FilePathManager>();
	// when a path is given, search the file only with this path
	pathManager->setValue("StrictPath", true);
	pathManager->AddToPath(".", "json");

	CoreCreateModule(HTTPRequestModule, 0);

	// init twitter connection
	mTwitterConnect = KigsCore::GetInstanceOf("TwitterConnect", "HTTPConnect");
	mTwitterConnect->setValue("HostName", "api.twitter.com");
	mTwitterConnect->setValue("Type", "HTTPS");
	mTwitterConnect->setValue("Port", "443");
	mTwitterConnect->Init();

	mState.back() = GET_USER_DETAILS;

	// Load AppInit, GlobalConfig then launch first sequence
	DataDrivenBaseApplication::ProtectedInit();
	mLastUpdate = GetApplicationTimer()->GetTime();
}

void	TweetFrequency::ProtectedUpdate()
{
	DataDrivenBaseApplication::ProtectedUpdate();


	if (mWaitQuota)
	{
		double dt = GetApplicationTimer()->GetTime() - mStartWaitQuota;
		// 2 minutes
		if (dt > (2.0 * 60.0))
		{

			mWaitQuota = false;
			mAnswer->GetRef(); 
			KigsCore::addAsyncRequest(mAnswer.get());
			mAnswer->Init();
			RequestLaunched(60.5);
		}
		
	}
	else 
	{
		switch (mState.back())
		{
		case WAIT_STATE: // wait

			break;
		case GET_USER_DETAILS:
		{
			std::string currentUserProgress = "Cache/UserNameTF/";
			currentUserProgress += mUserNames[mCurrentUserIndex] + "_" + mFromDate + "_" + mToDate + ".json";
			CoreItemSP currentP = LoadJSon(currentUserProgress, false, false);

			if (currentP.isNil())
			{
				if (CanLaunchRequest())
				{
					std::string url = "1.1/users/show.json?screen_name=" + mUserNames[mCurrentUserIndex];
					mAnswer = mTwitterConnect->retreiveGetAsyncRequest(url.c_str(), "getUserDetails", this);
					mAnswer->AddHeader(mTwitterBear[NextBearer()]);
					mAnswer->AddDynamicAttribute<maInt, int>("BearerIndex", CurrentBearer());
					mState.push_back(WAIT_USERID);
					mAnswer->Init();
					myRequestCount++;
					RequestLaunched(1.1);
				}
			}
			else
			{
				if(mUserIDs.size()<= mCurrentUserIndex)
					mUserIDs.push_back(currentP["id"]);

				TwitterAccount* current = getUser(mUserIDs[mCurrentUserIndex]);
				current->mUserStruct.mName = mUserNames[mCurrentUserIndex];
				mCurrentTreatedAccount = current;
				mState.back() = GET_TWEETS;
			}
		}
		break;
		case GET_TWEETS:

		{
			if (mCurrentTreatedAccount->needMoreTweets())
			{
				if (CanLaunchRequest())
				{
					std::string url = "2/users/" + std::to_string(mCurrentTreatedAccount->mID) + "/tweets?tweet.fields=public_metrics,created_at";
					url += "&start_time=" + mFromDate + "T00:00:00Z";
					url += "&end_time=" + mToDate + "T23:59:59Z";

					url += "&max_results=100";
					if (mCurrentTreatedAccount->mNextTweetsToken != "-1")
					{
						url += "&pagination_token=" + mCurrentTreatedAccount->mNextTweetsToken;
					}
					mAnswer = mTwitterConnect->retreiveGetAsyncRequest(url.c_str(), "getTweets", this);
					mAnswer->AddHeader(mTwitterBear[NextBearer()]);
					mAnswer->AddDynamicAttribute<maInt, int>("BearerIndex", CurrentBearer());
					mAnswer->Init();
					myRequestCount++;
					mState.push_back(WAIT_STATE);
					RequestLaunched(0.5);
				}
			}
			else
			{
				CoreItemSP twts=mCurrentTreatedAccount->loadTweetsFile();
				mCurrentTreatedAccount->addTweets(twts, false);
				mCurrentTreatedAccount->udateTweetsDate(mFromDate,mToDate);

				if (mCurrentUserIndex < (mUserNames.size() - 1))
				{
					mCurrentUserIndex++;
					mState.back() = GET_USER_DETAILS;
				}
				else
				{
					if (mMainInterface.isNil())
						break;

					AddUserLegend();

					DrawInBitmap();

					Upgrade("TickerUpgrador");
					setValue("TickerFrequency", 1.0f/5.0f);
					setValue("TickerFunction", "DrawInBitmap");

					mState.back() = EVERYTHING_DONE;
				}
			}
		}
		break;

		} // switch end
	}
	// update graphics
	double dt = GetApplicationTimer()->GetTime() - mLastUpdate;
	if (dt > 1.0)
	{
		mLastUpdate = GetApplicationTimer()->GetTime();
		if (mMainInterface)
		{
			char textBuffer[256];
			sprintf(textBuffer, "Treated account : %d", mCurrentUserIndex+1);
			mMainInterface["TreatedAccount"]("Text") = textBuffer;
			
			mMainInterface["FromDate"]("Text") = mFromDate;		
			mMainInterface["ToDate"]("Text") = mToDate;

			if (mState.back() != EVERYTHING_DONE)
			{
				sprintf(textBuffer, "Twitter API requests : %d", myRequestCount);
				mMainInterface["RequestCount"]("Text") = textBuffer;

				if (mWaitQuota)
				{
					sprintf(textBuffer, "Wait quota count: %d", mWaitQuotaCount);
				}
				else
				{
					double requestWait = mNextRequestDelay - (mLastUpdate - mLastRequestTime);
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
			}
		}
		
	}

}


void	TweetFrequency::AddUserLegend()
{

	// define drawing zone
	v2i	zoneStart(128, 128);
	v2i	zoneEnd(1920 - 128, 1080 - 128);

	int columnCount = mUserIDs.size();
	u32 columnSize = (zoneEnd.x - zoneStart.x) / columnCount;

	for (int i = 0; i < columnCount;i++)
	{
		CMSP uiPanel = KigsCore::GetInstanceOf("drawuColor", "UIPanel");
		uiPanel->setValue("Priority", 10);
		uiPanel->setValue("Anchor", v2f(0.5f, 0.5f));
		uiPanel->setValue("SizeX", 32);
		uiPanel->setValue("SizeY", 16);

		v2f dock((float)(zoneStart.x + columnSize / 4 + columnSize * i) / 1920.0f, 0.06f);

		uiPanel->setValue("Dock", dock);
		uiPanel->setValue("Color", userColorsf[i]);

		CMSP uiuName = KigsCore::GetInstanceOf("drawuName", "UIText");
		
		uiuName->setValue("Priority", 10);
		uiuName->setValue("Anchor", v2f(0.0f, 0.5f));
		uiuName->setValue("Dock", v2f(2.0f, 0.5f));
		uiuName->setValue("SizeX", -1);
		uiuName->setValue("SizeY", -1);
		uiuName->setValue("Text", mUserNames[i]);
		uiuName->setValue("Font", "Calibri.ttf");
		uiuName->setValue("FontSize", 32);
		uiuName->setValue("MaxWidth", 300);
		uiuName->setValue("Color", v3f( 0.1, 0.2, 0.2 ));

		uiPanel->addItem(uiuName);

		mMainInterface->addItem(uiPanel);
		uiPanel->Init();
		uiuName->Init();
	}

	

}


void TweetFrequency::DrawBackground()
{
	KigsBitmap* bitmap = static_cast<KigsBitmap*>(mBitmap.get());
	bitmap->Clear({ 0,0,0,0 });

	KigsBitmap::KigsBitmapPixel	black(0, 0, 0, 255);

	// define drawing zone
	v2i	zoneStart(128, 128);
	v2i	zoneEnd(1920 - 128, 1080 - 128);

	auto v = mCurrentTreatedAccount->getTweetsDate();
	int columnCount = v.size();
	u32 columnSize = (zoneEnd.x - zoneStart.x) / columnCount;

	// readjust end
	zoneEnd.x = zoneStart.x + columnSize * columnCount;

	KigsBitmap::KigsBitmapPixel	greys[4] = { {250, 250, 250, 255},{235, 235, 235, 255},{235, 255, 255, 255},{225, 245, 245, 255} };

	u32 firstwday = TwitterAccount::getWeekDay(mFromDate + "T00:00:00.000Z");

	unsigned char days[7] = { 'S','M','T','W','T','F','S' };

	for (int i = 0; i < columnCount; i++)
	{
		u32 currentDay = (firstwday + i) % 7;

		bitmap->Box(zoneStart.x + i * columnSize, zoneStart.y, columnSize, zoneEnd.y - zoneStart.y, greys[currentDay ? (i & 1):(2 + (i&1))]);

		std::string day = "";
		day += days[currentDay];

		bitmap->Print(day, zoneStart.x + i * columnSize + columnSize / 2 - 8, zoneEnd.y + 8, 1, 48, 16, "Calibri.ttf", 0, black);
	}

	bitmap->Line(zoneStart.x - 1, zoneStart.y - 16, zoneStart.x - 1, zoneEnd.y + 1, black);
	bitmap->Line(zoneStart.x - 1, zoneEnd.y + 1, zoneEnd.x + 16, zoneEnd.y + 1, black);

}

void TweetFrequency::DrawTweetsTime()
{
	DrawBackground();
	KigsBitmap* bitmap = static_cast<KigsBitmap*>(mBitmap.get());
	
	// define drawing zone
	v2i	zoneStart(128, 128);
	v2i	zoneEnd(1920 - 128, 1080 - 128);

	auto v = mCurrentTreatedAccount->getTweetsDate();
	int columnCount = v.size();
	u32 columnSize = (zoneEnd.x - zoneStart.x) / columnCount;

	// readjust end
	zoneEnd.x = zoneStart.x + columnSize * columnCount;

	KigsBitmap::KigsBitmapPixel	grey(0, 0, 0, 64);

	std::vector<u32>	columnZoneStartX;
	std::vector<u32>	lineStartX;
	for (int i = 0; i < columnCount; i++)
	{
		columnZoneStartX.push_back(zoneStart.x + i * columnSize);
	}

	u32 linesZoneSize = (columnSize * 80) / 100;
	u32 lineSize = linesZoneSize;
	lineSize /= mUserIDs.size();

	for (int i = 0; i < mUserIDs.size(); i++)
	{
		lineStartX.push_back((columnSize - linesZoneSize) / 2 + lineSize * i);
	}

	double ratio = (double)(zoneEnd.y - zoneStart.y) / (double)(3600 * 24);

	for (int i = 3; i < 24; i += 3)
	{
		u32 y = zoneEnd.y - (i * 3600.0f) * ratio;

		bitmap->Print(std::to_string(i), zoneStart.x - 48, y-8, 1, 48, 16, "Calibri.ttf", 0, KigsBitmap::KigsBitmapPixel(0, 0, 0, 255));

		bitmap->Line(zoneStart.x - 2, y, zoneEnd.x + 2, y, grey);
	}

	int userIndex = 0;

	

	for (auto uID : mUserIDs)
	{

		TwitterAccount* current = getUser(uID);
		v = current->getTweetsDate();

		for (int i = 0; i < columnCount; i++)
		{
			u32 x = columnZoneStartX[i] + lineStartX[userIndex];

			for (u32 t : v[i])
			{
				u32 y = zoneEnd.y - ratio * t;
				bitmap->Line(x, y, x + lineSize, y, userColors[userIndex]);
			}
		}
		userIndex++;
	}

}

void TweetFrequency::DrawTweetsCount()
{
	DrawBackground();
	KigsBitmap* bitmap = static_cast<KigsBitmap*>(mBitmap.get());
	// define drawing zone
	v2i	zoneStart(128, 128);
	v2i	zoneEnd(1920 - 128, 1080 - 128);

	auto v = mCurrentTreatedAccount->getTweetsDate();
	int columnCount = v.size();
	u32 columnSize = (zoneEnd.x - zoneStart.x) / columnCount;

	// readjust end
	zoneEnd.x = zoneStart.x + columnSize * columnCount;

	std::vector<u32>	columnZoneStartX;
	
	for (int i = 0; i < columnCount; i++)
	{
		columnZoneStartX.push_back(zoneStart.x + i * columnSize);
	}

	u32 maxTwts = 0;
	// find max tweets per day
	for (auto uID : mUserIDs)
	{
		TwitterAccount* current = getUser(uID);
		v = current->getTweetsDate();
		for (const auto& s : v)
		{
			if (s.size() > maxTwts)
				maxTwts = s.size();
		}
	}
	KigsBitmap::KigsBitmapPixel	grey(0, 0, 0, 64);
	double ratio = (double)(zoneEnd.y - zoneStart.y) / (double)(maxTwts);

	for (int i = 4; i > 0; i--)
	{
		u32 cnt = ((maxTwts * i) / 4);
		u32 y = zoneEnd.y - 8 - ratio * cnt;

		bitmap->Print(std::to_string(cnt), zoneStart.x - 48, y, 1, 48, 16, "Calibri.ttf", 0, KigsBitmap::KigsBitmapPixel(0, 0, 0, 255));
		bitmap->Line(zoneStart.x - 2, y+8, zoneEnd.x + 2, y+8, grey);
	}
	int userIndex = 0;

	KigsBitmap::KigsBitmapPixel	userColors[4] = { {200,0,0,255},{0,155,0,255},{0,0,200,255},{180,0,180,255} };

	for (auto uID : mUserIDs)
	{

		TwitterAccount* current = getUser(uID);
		v = current->getTweetsDate();

		for (int i = 0; i < columnCount-1; i++)
		{
			u32 x = columnZoneStartX[i] + columnSize/2;

			u32 y1 = zoneEnd.y - ratio * v[i].size();
			u32 y2 = zoneEnd.y - ratio * v[i+1].size();

			bitmap->Line(x, y1, x + columnSize, y2, userColors[userIndex]);
			
		}
		userIndex++;
	}

}

void TweetFrequency::DrawInBitmap()
{
	mDisplayCount = !mDisplayCount;
	if (mDisplayCount)
	{
		DrawTweetsCount();

	}
	else
	{
		DrawTweetsTime();
	}
}


void	TweetFrequency::ProtectedClose()
{
	DataDrivenBaseApplication::ProtectedClose();
	clearUsers();
	mTwitterConnect = nullptr;
	mAnswer = nullptr;
}

void	TweetFrequency::ProtectedInitSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mMainInterface = GetFirstInstanceByName("UIItem", "Interface");

		CMSP uiImage = KigsCore::GetInstanceOf("drawuiImage", "UIImage");
		uiImage->setValue("Priority", 10);
		uiImage->setValue("Anchor", v2f(0.0f, 0.0f));
		uiImage->setValue("Dock", v2f(0.0f, 0.0f));
		uiImage->setValue("Color", v3f(1.0f, 1.0f,1.0f));
		CMSP drawtexture = KigsCore::GetInstanceOf("drawTexture", "Texture");
		drawtexture->setValue("FileName", "");
		
		mBitmap = KigsCore::GetInstanceOf("mBitmap", "KigsBitmap");
		mBitmap->setValue("Size", v2f(2048, 2048));
		drawtexture->addItem(mBitmap);

		mMainInterface->addItem(uiImage);
		uiImage->Init();

		drawtexture->Init();
		mBitmap->Init();
		uiImage->addItem(drawtexture);
		
	}
}
void	TweetFrequency::ProtectedCloseSequence(const kstl::string& sequence)
{
	if (sequence == "sequencemain")
	{
		mMainInterface = nullptr;	

	}
}

CoreItemSP	TweetFrequency::RetrieveJSON(CoreModifiable* sender)
{
	void* resultbuffer = nullptr;
	sender->getValue("ReceivedBuffer", resultbuffer);

	if (resultbuffer)
	{
		CoreRawBuffer* r = (CoreRawBuffer*)resultbuffer;
		std::string_view received(r->data(), r->size());

		std::string validstring(received);

		if (validstring.length() == 0)
		{
			
		}
		else
		{

			usString	utf8string((UTF8Char*)validstring.c_str());

			JSonFileParserUTF16 L_JsonParser;
			CoreItemSP result = L_JsonParser.Get_JsonDictionaryFromString(utf8string);

			if (!result["error"].isNil())
			{
				return nullptr;
			}
			if (!result["errors"].isNil())
			{
				if (!result["errors"][0]["code"].isNil())
				{
					int code = result["errors"][0]["code"];
					mApiErrorCode = code;
					if (code == 88)
					{
						mWaitQuota = true;
						mWaitQuotaCount++;
						mStartWaitQuota = GetApplicationTimer()->GetTime();
					}
				
					if (code == 32)
					{
						HTTPAsyncRequest* request = (HTTPAsyncRequest*)sender;
						mWaitQuota = true;
						mWaitQuotaCount++;
						mStartWaitQuota = GetApplicationTimer()->GetTime();
						request->ClearHeaders();
						int bearerIndex = request->getValue<int>("BearerIndex");
						// remove bearer from list
						mTwitterBear.erase(mTwitterBear.begin() + bearerIndex);
						mAnswer->AddHeader(mTwitterBear[NextBearer()]);
						mAnswer->setValue("BearerIndex", CurrentBearer());
					}
					
				}

				return nullptr;
			}
			
			return result;
			
		}
	}

	return nullptr;
}

void		TweetFrequency::SaveJSon(const std::string& fname,const CoreItemSP& json, bool utf16)
{


	if (utf16)
	{
		JSonFileParserUTF16 L_JsonParser;
		L_JsonParser.Export((CoreMap<usString>*)json.get(), fname);
	}
	else
	{
		JSonFileParser L_JsonParser;
		L_JsonParser.Export((CoreMap<std::string>*)json.get(), fname);
	}

}

bool TweetFrequency::checkValidFile(const std::string& fname, SmartPointer<::FileHandle>& filenamehandle,double OldFileLimit)
{
	auto& pathManager = KigsCore::Singleton<FilePathManager>();
	filenamehandle = pathManager->FindFullName(fname);


	if ((filenamehandle->mStatus & FileHandle::Exist))
	{
		if (OldFileLimit > 1.0)
		{
			// Windows specific code 
#ifdef WIN32
			struct _stat resultbuf;

			if (_stat(filenamehandle->mFullFileName.c_str(), &resultbuf) == 0)
			{
				auto mod_time = resultbuf.st_mtime;

				double diffseconds = difftime(mCurrentTime, mod_time);

				if (diffseconds > OldFileLimit)
				{
					return false;
				}
			}
#endif
		}
	
		return true;
		
	}


	return false;
}

CoreItemSP	TweetFrequency::LoadJSon(const std::string& fname, bool utf16, bool checkfileDate)
{
	SmartPointer<::FileHandle> filenamehandle;

	CoreItemSP initP(nullptr);
	if (checkValidFile(fname, filenamehandle, checkfileDate?mOldFileLimit:0.0))
	{
		if (utf16)
		{
			JSonFileParserUTF16 L_JsonParser;
			initP = L_JsonParser.Get_JsonDictionary(filenamehandle);
		}
		else
		{
			JSonFileParser L_JsonParser;
			initP = L_JsonParser.Get_JsonDictionary(filenamehandle);
		}
	}
	return initP;
}



DEFINE_METHOD(TweetFrequency, getTweets)
{
	auto json = RetrieveJSON(sender);

	if (!json.isNil())
	{
		mCurrentTreatedAccount->addTweets(json,true);
		mState.pop_back();
		
	}
	
	return true;
}


DEFINE_METHOD(TweetFrequency, getUserDetails)
{
	auto json = RetrieveJSON(sender);

	if (!json.isNil())
	{

		u64			currentID= json["id"];
		std::string user = json["screen_name"];

		TwitterAccount* current=getUser(currentID);

		if (mCurrentTreatedAccount == nullptr) // start account ?
		{
			mCurrentTreatedAccount = current;
		}

		current->mUserStruct.mName = user;
		current->mID = currentID;
		current->mUserStruct.mFollowersCount = json["followers_count"];
		current->mUserStruct.mFollowingCount = json["friends_count"];
		current->mUserStruct.mStatuses_count = json["statuses_count"];
		current->mUserStruct.UTCTime = json["created_at"];

	
		if (mState.back() == WAIT_USERID)
		{
			if (mUserIDs.size() <= mCurrentUserIndex)
				mUserIDs.push_back(currentID);

			JSonFileParser L_JsonParser;
			CoreItemSP initP = CoreItemSP::getCoreItemOfType<CoreMap<std::string>>();
			CoreItemSP idP = CoreItemSP::getCoreItemOfType<CoreValue<u64>>();
			idP = currentID;
			initP->set("id", idP);
			std::string filename = "Cache/UserNameTF/" + mUserNames[mCurrentUserIndex] + "_" + mFromDate + "_" + mToDate + ".json";
			L_JsonParser.Export((CoreMap<std::string>*)initP.get(), filename);
		}

		mState.pop_back();
		
	}
	

	return true;
}

void ReplaceStr(std::string& str,
	const std::string& oldStr,
	const std::string& newStr)
{
	std::string::size_type pos = 0u;
	while ((pos = str.find(oldStr, pos)) != std::string::npos) 
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}


std::string  TweetFrequency::CleanURL(const std::string& url)
{
	std::string result = url;
	size_t pos;

	do
	{
		pos = result.find("\\/");
		if (pos != std::string::npos)
		{
			result = result.replace(pos, 2, "/", 1);
		}

	} while (pos != std::string::npos);

	return result;
}



std::string	TweetFrequency::GetIDString(u64 id)
{
	char	idstr[64];
	sprintf(idstr,"%llu", id);

	return idstr;
}


v3f TweetFrequency::getRandomColor()
{

	auto normalizeRand = []()->float
	{
		float fresult = ((float)(rand() % 1001)) / 1000.0f;
		return fresult;
	};


	v3f schemas[6] = { {1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0},{1.0,1.0,0.0},{1.0,0.0,1.0},{0.0,0.0,0.0} };

	v3f result;
	bool foundColorOK = true;
	
	int countLoop = 0;

	do
	{
		result = schemas[rand() % 6];

		for (size_t i = 0; i < 3; i++)
		{
			if (result[i] > 0.5f)
			{
				result[i] = 0.6f + normalizeRand() * 0.4f;
			}
			else
			{
				result[i] = normalizeRand() * 0.4f;
			}
		}

		foundColorOK = true;

		for (auto c : mAlreadyFoundColors)
		{
			v3f diff(c - result);
			if (Norm(diff) < 0.15f)
			{
				foundColorOK = false;
				break;
			}
		}



		countLoop++;

	} while ((!foundColorOK)&&(countLoop<8));


	mAlreadyFoundColors.push_back(result);

	return result;

}


