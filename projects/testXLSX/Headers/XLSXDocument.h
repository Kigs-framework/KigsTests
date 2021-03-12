#pragma once
#include <vector>
#include <string>
#include <map>
#include "XLSXSheet.h"
#include "XMLArchiveManager.h"
#include "XLSXElementRef.h"

class XLSXContentType
{
protected:
	std::map<std::string, std::string>	mExtensions;
	std::map< std::string, std::string>  mOverride;

public:
	XLSXContentType() {};
	~XLSXContentType() {};

	void initFromXML(XMLBase* xml);

	std::string getContentType(const std::string name);
	std::string getPartName(const std::string contenttype);

	friend class XLSXDocument;
};

class XLSXSharedStrings
{
protected:
	std::vector<std::string>	mSharedStrings;
	std::map<std::string, u32>	mStringIndex;
	u32							mCountAccess = 0;
	friend class XLSXDocument;
	friend class XLSXSheet;

	u32			getIndex(const std::string& s)
	{
		mCountAccess++;
		u32 result = 0;
		auto f = mStringIndex.find(s);
		if (f == mStringIndex.end())
		{
			mSharedStrings.push_back(s);
			result = mSharedStrings.size() - 1;
			mStringIndex[s] = result;
		}
		else
		{
			result = (*f).second;
		}
		return result;
	}

	void reset()
	{
		mSharedStrings.clear();
		mStringIndex.clear();
		mCountAccess = 0;
	}
	void updateXML(XMLBase* toUpdate);

public:
	XLSXSharedStrings() {};
	~XLSXSharedStrings() {};

	void initFromXML(XMLBase* xml);

	std::string getString(u32 i)
	{
		return mSharedStrings[i];
	}
};

class XLSXRelationships
{
protected:

	class RelationStruct
	{
	public:
		RelationStruct()
		{

		}
		RelationStruct(const std::string& t, const std::string& ta) : mType(t), mTarget(ta) {};
		std::string mType;
		std::string mTarget;
	};

	std::map<std::string, RelationStruct>	mRelations;

public:
	XLSXRelationships() {};
	~XLSXRelationships() {};

	std::string	getTarget(const std::string& id)
	{
		auto f = mRelations.find(id);
		if (f != mRelations.end())
		{
			return (*f).second.mTarget;
		}
		return "";
	}

	std::string	getTargetFromType(const std::string& t)
	{
		for(auto f : mRelations)
		{
			auto found = f.second.mType.find(t);
			if (found != std::string::npos)
			{
				return f.second.mTarget;
			}
		}
		return "";
	}

	void initFromXML(XMLBase* xml);

	XMLBase* createXML();

	friend class XLSXDocument;
};

class XLSXDocument : public XMLArchiveManager
{
protected:
	std::vector<XLSXSheet*>						mSheets;
	XLSXContentType								mContentType;
	std::map<std::string, XLSXRelationships>	mRels;
	XLSXSharedStrings							mSharedStrings;

	void		initWorkbook(const std::string& name);
	void		initSharedStrings(const std::string& name);
	XLSXSheet*	initSheet(const std::string& file, std::string name, int id);

	// create folder hierarchy

	void		createFolderHierarchy();

public:

	XLSXDocument();
	virtual ~XLSXDocument();

	// return ref on sheet
	XLSXElementRef operator[](u32 i)
	{
		if (mSheets.size() > i)
		{
			return XLSXElementRef(mSheets[i]);
		}
		return XLSXElementRef();
	}

	XLSXElementRef operator[](const std::string n)
	{
		for(auto s: mSheets)
		{
			if(s->getName() == n)
				return XLSXElementRef(s);
		}
		return XLSXElementRef();
	}

	std::vector<XLSXElementRef>	find(const std::string& content,bool exactmatch=false);
	std::vector<XLSXElementRef>	find(int val);

	virtual bool	open(const std::string& filename) override;

	// create empty document
	void	initEmpty();

	// add one sheet
	void addSheet(std::string n = "");

	virtual CoreRawBuffer* save() override;
};