#pragma once

#include <DataDrivenBaseApplication.h>
#include "Texture.h"
#include "HTTPConnect.h"
#include "TwitterAccount.h"

class AnonymousModule;

class TweetFrequency : public DataDrivenBaseApplication
{
public:
	DECLARE_CLASS_INFO(TweetFrequency, DataDrivenBaseApplication, Core);
	DECLARE_CONSTRUCTOR(TweetFrequency);

	const std::string& getFromDate() const 
	{
		return mFromDate;
	}
	const std::string& getToDate() const
	{
		return mToDate;
	}

protected:



	SP<HTTPConnect>									mTwitterConnect = nullptr;
	SP<HTTPAsyncRequest>							mAnswer = nullptr;

	void	ProtectedInit() override;
	void	ProtectedUpdate() override;
	void	ProtectedClose() override;

	DECLARE_METHOD(getTweets);
	DECLARE_METHOD(getUserDetails);
	COREMODIFIABLE_METHODS(getTweets,  getUserDetails);


	CoreItemSP	RetrieveJSON(CoreModifiable* sender);

	void	ProtectedInitSequence(const kstl::string& sequence) override;
	void	ProtectedCloseSequence(const kstl::string& sequence) override;
	static CoreItemSP	LoadJSon(const std::string& fname, bool utf16 = false,bool checkfileDate=true);
	static void			SaveJSon(const std::string& fname,const CoreItemSP& json, bool utf16 = false);

	std::vector<std::string>	mTwitterBear;
	// to check youtube channels
	unsigned int				mCurrentBearer = 0;
	unsigned int				NextBearer()
	{
		mCurrentBearer = (mCurrentBearer + (unsigned int)1) % mTwitterBear.size();
		return mCurrentBearer;
	}
	unsigned int				CurrentBearer()
	{
		return mCurrentBearer;
	}

	std::string			mFromDate;
	std::string			mToDate;

	std::vector<std::string>	mUserNames;
	std::vector<u64>			mUserIDs;
	u32							mCurrentUserIndex = 0;
	TwitterAccount*		mCurrentTreatedAccount=nullptr;

	enum AppStates
	{
		WAIT_STATE =					0,
		GET_USER_DETAILS =				1,
		GET_TWEETS =					2,
		WAIT_USERID =					3,
		EVERYTHING_DONE =				17
	};
	// current application state 
	std::vector< AppStates>					mState;

	CMSP			mMainInterface;

	TwitterAccount* getUser(u64 uID)
	{
		auto f = mUserAccountList.find(uID);
		if (f != mUserAccountList.end())
			return ((*f).second);


		TwitterAccount* toInsert = new TwitterAccount(this);
		toInsert->mID = uID;
		mUserAccountList.insert({ uID,toInsert });
		
		return toInsert;
	}

	void clearUsers()
	{
		for (auto i : mUserAccountList)
		{
			delete i.second;
		}
	}


	std::map<u64,TwitterAccount*>				mUserAccountList;

	template<typename T>
	bool	LoadDataFile(const std::string& filename, std::vector<T>& loaded)
	{
		SmartPointer<::FileHandle> L_File;
		
		if (checkValidFile(filename, L_File, mOldFileLimit))
		{
			if (Platform_fopen(L_File.get(), "rb"))
			{
				// get file size
				Platform_fseek(L_File.get(), 0, SEEK_END);
				long filesize = Platform_ftell(L_File.get());
				Platform_fseek(L_File.get(), 0, SEEK_SET);

				loaded.resize(filesize / sizeof(T));

				Platform_fread(loaded.data(), sizeof(T), loaded.size(), L_File.get());
				Platform_fclose(L_File.get());
				return true;
			}
		}
		

		return false;
	}

	template<typename T>
	void	SaveDataFile(const std::string& filename, const std::vector<T>& saved)
	{
		SmartPointer<::FileHandle> L_File = Platform_fopen(filename.c_str(), "wb");
		if (L_File->mFile)
		{
			Platform_fwrite(saved.data(), 1, saved.size() * sizeof(T), L_File.get());
			Platform_fclose(L_File.get());
		}
	}

	
	static std::string	GetIDString(u64 id);
	static std::string  CleanURL(const std::string& url);

	// get current time once at launch to compare with modified file date
	static time_t		mCurrentTime;
	// if a file is older ( in seconds ) than this limit, then it's considered as not existing ( to recreate )
	static double		mOldFileLimit;
	double				mLastUpdate;

	bool				mWaitQuota = false;
	unsigned int		mWaitQuotaCount = 0;


	unsigned int		myRequestCount = 0;
	double				mStartWaitQuota=0.0;

	// manage wait time between requests

	void	RequestLaunched(double toWait)
	{
		mNextRequestDelay = toWait/ mTwitterBear.size();
		mLastRequestTime = GetApplicationTimer()->GetTime();
	}

	bool	CanLaunchRequest()
	{
		double dt = GetApplicationTimer()->GetTime() - mLastRequestTime;
		if (dt > mNextRequestDelay)
		{
			return true;
		}
		return false;
	}

	static bool checkValidFile(const std::string& fname, SmartPointer<::FileHandle>& filenamehandle, double OldFileLimit);

	double		mNextRequestDelay = 0.0;
	double		mLastRequestTime = 0.0;

	std::vector<u64>			mUserDetailsAsked;

	int							mCurrentYear=0;

	unsigned int				mApiErrorCode=0;
	std::vector<std::pair<CMSP, std::pair<u64, TwitterAccount::UserStruct*>> >		mDownloaderList;


	std::vector<v3f>	mAlreadyFoundColors;

	v3f getRandomColor();

	CMSP	mBitmap;

	void DrawInBitmap();
	void DrawBackground();
	void DrawTweetsTime();
	void DrawTweetsCount();

	void	AddUserLegend();
	bool	mDisplayCount = false;

	WRAP_METHODS(DrawInBitmap);
};
