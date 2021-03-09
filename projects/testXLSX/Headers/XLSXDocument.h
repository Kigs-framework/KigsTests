#pragma once
#include <vector>
#include <string>
#include <map>
#include "XLSXSheet.h"
#include "XMLArchiveManager.h"

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

	XMLBase* createXML();

	friend class XLSXDocument;
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
public:

	XLSXDocument();
	~XLSXDocument();

	virtual bool	open(const std::string& filename) override;
};