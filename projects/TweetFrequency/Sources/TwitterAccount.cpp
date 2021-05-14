#include "TwitterAccount.h"
#include "TweetFrequency.h"
#include "CoreMap.h"
#include <iomanip> 

CoreItemSP TwitterAccount::loadTweetsFile()
{
	std::string filename = "Cache/UserNameTF/";
	filename += mUserStruct.mName.ToString() + "_" + mSettings->getFromDate() + "_" + mSettings->getToDate() +"_tweets.json";
	return TweetFrequency::LoadJSon(filename, true,false);
}
void	TwitterAccount::saveTweetsFile(CoreItemSP toSave)
{
	std::string filename = "Cache/UserNameTF/";
	filename += mUserStruct.mName.ToString() + "_" + mSettings->getFromDate() + "_" + mSettings->getToDate() + "_tweets.json";

	TweetFrequency::SaveJSon(filename, toSave, true);
}


bool	TwitterAccount::needMoreTweets()
{
	
	CoreItemSP tweets = loadTweetsFile();
	if (tweets.isNil())
	{
		return true;
	}
	else
	{
		CoreItemSP meta = tweets["meta"];
		if (!meta.isNil())
		{
			if (!meta["next_token"].isNil())
			{
				mNextTweetsToken = meta["next_token"];
				return true;
			}
		}
	}

	return false;
}

void		TwitterAccount::updateTweetList(CoreItemSP currentTwt)
{
	std::string txt = currentTwt["text"];

	u64 tweetid = currentTwt["id"];
	u32 like_count = currentTwt["public_metrics"]["like_count"];
	u32 rt_count = currentTwt["public_metrics"]["retweet_count"];
	u32 quote_count = currentTwt["public_metrics"]["quote_count"];

	bool needAdd = true;
	tweet toadd;
	toadd.mID = tweetid;
	toadd.mCreatedAt = currentTwt["created_at"];

	mTweetList.push_back(toadd);

}

void		TwitterAccount::addTweets(CoreItemSP json,bool addtofile)
{

	CoreItemSP fromfile = nullptr;
	if(addtofile)
		fromfile=loadTweetsFile();

	CoreItemSP tweetsArray = json["data"];
	if (!tweetsArray.isNil())
	{
		unsigned int tweetcount = tweetsArray->size();
		for (unsigned int i = 0; i < tweetcount; i++)
		{
			CoreItemSP currentTwt = tweetsArray[i];

			// add tweet to previous file
			if (!fromfile.isNil() && addtofile)
				fromfile["data"]->set("", currentTwt);

			if (!addtofile)
				updateTweetList(currentTwt); // update tweet list when all tweets were loaded
		}

	}


	if (!fromfile.isNil() && addtofile)
	{

		if (fromfile["includes"].isNil())
		{
			fromfile->set("includes", CoreItemSP::getCoreMap());
		}

		if (fromfile["includes"]["media"].isNil())
		{
			fromfile["includes"]->set("media", CoreItemSP::getCoreVector());
		}

		CoreItemSP addTo = fromfile["includes"]["media"];

		// add includes
		CoreItemSP includesMediaArray = json["includes"]["media"];
		if (!includesMediaArray.isNil())
		{
			unsigned int mediacount = includesMediaArray->size();
			for (unsigned int i = 0; i < mediacount; i++)
			{
				CoreItemSP currentMedia = includesMediaArray[i];

				addTo->set("", currentMedia);
			}
		}
	}

	std::string nextStr = "-1";

	CoreItemSP meta = json["meta"];
	if (!meta.isNil())
	{
		if (!meta["next_token"].isNil())
		{
			nextStr = meta["next_token"];
			if (nextStr == "0")
			{
				nextStr = "-1";
			}
		}
	}

	if (!fromfile.isNil() && addtofile)
	{
		fromfile->set("meta",meta);
	}
	else
	{
		fromfile = json;
	}
	
	if(addtofile)
		saveTweetsFile(fromfile);

	mNextTweetsToken = nextStr;
}

time_t	TwitterAccount::getDateAndTime(const std::string& d)
{
	std::tm gettm;
	memset(&gettm, 0, sizeof(gettm));
	std::istringstream ss(d);
	ss >> std::get_time(&gettm, "%Y-%m-%dT%H:%M:%S.000Z");
	return mktime(&gettm);
}

u32		TwitterAccount::getWeekDay(const std::string& d)
{
	std::tm gettm;
	memset(&gettm, 0, sizeof(gettm));
	std::istringstream ss(d);
	ss >> std::get_time(&gettm, "%Y-%m-%dT%H:%M:%S.000Z");
	 mktime(&gettm);
	 return gettm.tm_wday;
}


void		TwitterAccount::udateTweetsDate(const std::string& start, const std::string& end)
{
	 
	time_t starttime = getDateAndTime(start+"T00:00:00.000Z");
	time_t endtime = getDateAndTime(end + "T00:00:00.000Z");

	double diffseconds = difftime(endtime, starttime);

	u32 inDays = (u32)(diffseconds / (3600.0 * 24.0));
	inDays++;

	mTweetsDate.resize(inDays);

	for (auto t : mTweetList)
	{
		time_t currentTweetTime = getDateAndTime(t.mCreatedAt);
		diffseconds = difftime(currentTweetTime, starttime);
	
		u32 inDays = (u32)(diffseconds / (3600.0 * 24.0));

		u32 seconds = (u32)(diffseconds - (double)inDays * (3600.0 * 24.0));
		
		mTweetsDate[inDays].push_back(seconds);

	}
}

