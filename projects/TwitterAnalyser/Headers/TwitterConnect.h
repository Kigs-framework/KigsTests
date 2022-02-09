#pragma once
#include "Texture.h"
#include "HTTPConnect.h"
#include "CoreBaseApplication.h"

class TwitterConnect : public CoreModifiable
{
protected:
	// bearer mamangement
	std::vector<std::string>	mTwitterBear;
	unsigned int				mCurrentBearer = 0;
	
	double		mNextRequestDelay = 0.0;
	double		mLastRequestTime = 0.0;

	static double		mOldFileLimit;

	// get current time once at launch to compare with modified file date
	static time_t     mCurrentTime;
	unsigned int		mRequestCount = 0;


public:

	DECLARE_CLASS_INFO(TwitterConnect, CoreModifiable, TwitterConnect);
	DECLARE_CONSTRUCTOR(TwitterConnect);

	class ThumbnailStruct
	{
	public:
		SP<Texture>					mTexture = nullptr;
		std::string					mURL = "";
	};

	struct favoriteStruct
	{
		u64		tweetID;
		u64		userID;
		u32		likes_count;
		u32		retweet_count;
	};

	class UserStruct
	{
	public:
		usString					mName = std::string("");
		unsigned int				mFollowersCount = 0;
		unsigned int				mFollowingCount = 0;
		unsigned int				mStatuses_count = 0;
		std::string					UTCTime = "";
		ThumbnailStruct				mThumb;
		std::vector<u64>			mFollowing;
		float						mW;
		u64							mID;
		// detailed stats
		u32							mLikerCount;
		u32							mLikerFollowerCount;
		u32							mLikerMainUserFollowerCount;
		u32							mLikerBothFollowCount;
		u32							mLikesCount;
	};

	struct Twts
	{
		u64		mTweetID;
		u32		mLikeCount;
		u32		mRetweetCount;
	};

	unsigned int				NextBearer()
	{
		mCurrentBearer = (mCurrentBearer + (unsigned int)1) % mTwitterBear.size();
		return mCurrentBearer;
	}
	unsigned int				CurrentBearer()
	{
		return mCurrentBearer;
	}

	// HTTP Request management
	SP<HTTPConnect>									mTwitterConnect = nullptr;
	SP<HTTPAsyncRequest>							mAnswer = nullptr;

	static CoreItemSP	LoadJSon(const std::string& fname, bool useOldFileLimit=true,bool utf16 = false);
	static void		SaveJSon(const std::string& fname, const CoreItemSP& json, bool utf16=false);

	static bool		checkValidFile(const std::string& fname, SmartPointer<::FileHandle>& filenamehandle, double OldFileLimit);

	// manage wait time between requests
	void	RequestLaunched(double toWait)
	{
		mNextRequestDelay = toWait / mTwitterBear.size();
		mLastRequestTime = KigsCore::GetCoreApplication()->GetApplicationTimer()->GetTime();
	}

	bool	CanLaunchRequest()
	{
		double dt = KigsCore::GetCoreApplication()->GetApplicationTimer()->GetTime() - mLastRequestTime;
		if (dt > mNextRequestDelay)
		{
			return true;
		}
		return false;
	}

	float	getDelay()
	{
		double dt = mNextRequestDelay-(KigsCore::GetCoreApplication()->GetApplicationTimer()->GetTime() - mLastRequestTime);
		return dt;
	}

	void	initBearer(CoreItemSP f);
	void	initConnection(double oldfiletime);


	static void		LaunchDownloader(u64 id, UserStruct& ch);
	static bool		LoadUserStruct(u64 id, UserStruct& ch, bool requestThumb);
	static void		SaveUserStruct(u64 id, UserStruct& ch);
	static std::string	GetUserFolderFromID(u64 id);
	static std::string	GetIDString(u64 id);
	static std::string  CleanURL(const std::string& url);
	static std::vector<u64>		LoadIDVectorFile(const std::string& filename, bool& fileExist,bool oldfilelimit=true);
	static void		SaveFollowingFile(u64 id, const std::vector<u64>& v);
	static void		SaveIDVectorFile(const std::vector<u64>& v, const std::string& filename);

	template<typename T>
	static bool	LoadDataFile(const std::string& filename, std::vector<T>& loaded, bool useOldFileLimit = true)
	{
		SmartPointer<::FileHandle> L_File;

		if (checkValidFile(filename, L_File, useOldFileLimit?mOldFileLimit:0.0))
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
	static void	SaveDataFile(const std::string& filename, const std::vector<T>& saved)
	{
		SmartPointer<::FileHandle> L_File = Platform_fopen(filename.c_str(), "wb");
		if (L_File->mFile)
		{
			Platform_fwrite(saved.data(), 1, saved.size() * sizeof(T), L_File.get());
			Platform_fclose(L_File.get());
		}
	}

	static bool	LoadThumbnail(u64 id, UserStruct& ch);


	void	launchUserDetailRequest(const std::string& UserName, UserStruct& ch, bool requestThumb,const std::string& signal="done");
	void	launchUserDetailRequest(u64 userid,UserStruct& ch, bool requestThumb, const std::string& signal = "done");
	void	launchGetFollowing(UserStruct& ch, const std::string& signal = "done");
	void	launchGetFavoritesRequest(const std::string& user);


	static bool	LoadTweetsFile(std::vector<Twts>& tweetlist, const std::string& username,const std::string& fname="");
	static void	SaveTweetsFile(const std::vector<Twts>& tweetlist, const std::string& username, const std::string& fname = "");
	void	launchGetTweetRequest(u64 userid, const std::string& username);

	u32		getRequestCount()
	{
		return mRequestCount;
	}

	// utility

	template<typename T>
	static void	randomizeVector(std::vector<T>& v)
	{
		unsigned int s = v.size();
		if (s > 1)
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

	static CoreItemSP	LoadLikersFile(u64 tweetid, const std::string& username);
	static void			SaveLikersFile(const std::vector<std::string>& tweetLikers, u64 tweetid, const std::string& username);

	static bool					LoadFavoritesFile(const std::string& username, std::vector<TwitterConnect::favoriteStruct>& fav);
	static void					SaveFavoritesFile(const std::string& username, const std::vector<TwitterConnect::favoriteStruct>& favs);
protected:

	void	sendRequest(); // send waiting request
	void	resendRequest(); // send again same request

	WRAP_METHODS(resendRequest, sendRequest, thumbnailReceived);

	UserStruct* mCurrentUserStruct=nullptr;
	std::string	mSignal;


	DECLARE_METHOD(getUserDetails);
	DECLARE_METHOD(getTweets);
	DECLARE_METHOD(getFollowing);
	DECLARE_METHOD(getFavorites);
	
	COREMODIFIABLE_METHODS(getUserDetails, getFollowing, getTweets, getFavorites);
	CoreItemSP	RetrieveJSON(CoreModifiable* sender);

	std::vector<std::pair<CMSP, std::pair<u64, UserStruct*>> >		mDownloaderList;

	static TwitterConnect* mInstance;

	bool			mWaitQuota = false;
	u32				mWaitQuotaCount = 0;
	unsigned int	mApiErrorCode = 0;

	std::string		mNextCursor = "-1";

	std::vector<u64>	mCurrentIDVector;

	void	thumbnailReceived(CoreRawBuffer* data, CoreModifiable* downloader);

};