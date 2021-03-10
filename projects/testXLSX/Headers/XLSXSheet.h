#pragma once

#include <string>
#include "XML.h"
#include "CoreItem.h"

class CellName
{
protected:
	std::string	mName;
	v2i			mIndexes;

	void	interpretStringName();

public:
	CellName() {};
	CellName(const std::string& n) : mName(n)
	{
		interpretStringName();
	};
	~CellName() {};

	void	setName(const std::string& n)
	{
		mName = n;
		interpretStringName();
	}

	void	setIndex(const v2i& i);

	std::string getCol();
	std::string	getRow();

	// index starting at 0
	u32			getColIndex()
	{
		return mIndexes[0];
	}


	// index starting at 0
	u32			getRowIndex()
	{
		return mIndexes[1];
	}

	const std::string& getName()
	{
		return mName;
	}

private:

};


class XLSXCell : public CoreItemSP
{
protected:
	CellName	mName;
	friend class XLSXSheet;
public:

	XLSXCell(const std::string& name) : CoreItemSP(), mName(name) {};

	XLSXCell() : CoreItemSP(), mName() {};

	XLSXCell& operator=(const CoreItemSP& other)
	{
		CoreItemSP::operator=(other);
		return *this;
	}

};

class XLSXSheet;

class XLSXRow : public std::vector<XLSXCell>
{
protected:
	u32			mIndex = 0;
	XLSXSheet*	mSheet = nullptr;

	friend class XLSXSheet;

public:

	XLSXRow() : std::vector<XLSXCell>() {};


};

class XLSXSharedStrings;

class XLSXSheet
{
protected:
	std::string						mName;
	s32								mSheedID=0;
	std::vector<XLSXRow>			mRows;
	XLSXSharedStrings*				mSharedStrings=nullptr;

	friend class XLSXDocument;

	XLSXSheet() {};

public:

	XLSXSheet(const std::string& name, s32 id,XLSXSharedStrings* sharedstrings) : mName(name), mSheedID(id), mSharedStrings(sharedstrings) {};
	~XLSXSheet() {};

	void initFromXML(XMLBase* xml);
	XMLBase* createXML();

	v2i	getDimension()
	{
		if (mRows.size())
		{
			return v2i((s32)mRows[0].size(),(s32) mRows.size());
		}
		return v2i(0, 0);
	}

	//XLSXPosRef& operator[](const std::string& col);

	static	std::string	getCellName(u32 col, u32 row);
	static	u32	getColIndex(const std::string& colname);
	static	std::string	getColName(const std::string& cellname);
	static	u32	getRowIndex(const std::string& rowname);
	static	std::string	getRowName(const std::string& cellname);

	static  v2i	getCellPos(const std::string& cellname);

	static  std::pair<v2i, v2i>	getRange(const std::string& rangename);
};

// just a reference to the good column in sheet
class XLSXCol
{
protected:

	u32			mIndex = 0;
	XLSXSheet* mSheet = nullptr;
	friend class XLSXSheet;

public:
	XLSXCol()
	{

	}

};
