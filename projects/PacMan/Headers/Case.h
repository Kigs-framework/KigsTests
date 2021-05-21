#pragma once
#include "CoreModifiable.h"

class Case
{
protected:

	u32	mInitType;
	u32	mCurrentType;
public:

	Case() : mInitType(-1), mCurrentType(-1)
	{

	}

	Case(u32 initType) : mInitType(initType), mCurrentType(initType)
	{

	}

	void setInitType(u32 initType)
	{
		if (mInitType != (u32)-1) // already set
		{
			return;
		}
		mInitType = initType;
		mCurrentType = initType;
	}

	void setType(u32 cType)
	{
		if (mCurrentType == (u32)-1) 
		{
			// ERROR
			return;
		}
		mCurrentType = cType;
	}

	u32 getType()
	{
		return mCurrentType;
	}

	void reset()
	{
		mCurrentType = mInitType;
	}
};