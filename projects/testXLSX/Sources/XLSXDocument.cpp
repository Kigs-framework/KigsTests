#include "XLSXDocument.h"
#include "FilePathManager.h"
#include "ModuleFileManager.h"
#include "miniz.h"

XLSXDocument::XLSXDocument() : XMLArchiveManager()
{

}

XLSXDocument::~XLSXDocument()
{
	
}


bool	XLSXDocument::open(const std::string& filename)
{
	if (XMLArchiveManager::open(filename))
	{
		XMLArchiveFile* xlsxfile=static_cast<XMLArchiveFile*>(mRoot.getFile("[Content_Types].xml"));
		if ((xlsxfile) && (xlsxfile->getXML()))
		{
			XMLBase* type_xml= xlsxfile->getXML();
			mContentType.initFromXML(type_xml);
		}
		else
		{
			return false;
		}

		// check all contents that can be converted to xml
		for (auto f : (*this))
		{
			if (!f->isFolder())
			{
				std::string path = f->getName(true);
				std::string ctype=mContentType.getContentType(path);

				if (ctype.length()>=3)
				{
					if (ctype.substr(ctype.length() - 3, 3) == "xml")
					{
						static_cast<XMLArchiveFile*>(f)->interpretAsXML();
					}
				}

				// manage rels
				if (f->getExtension() == "rels")
				{
					xlsxfile = static_cast<XMLArchiveFile*>(f);
					XMLBase* rels_xml = xlsxfile->getXML();
					mRels[f->getPath()].initFromXML(rels_xml);
				}
			}
		}
	}
	return false;
}

// content type

void XLSXContentType::initFromXML(XMLBase* xml)
{
	mExtensions.clear();
	mOverride.clear();

	XMLNodeBase* root=xml->getRoot();

	for (u32 i = 0; i < root->getChildCount(); i++)
	{
		XMLNodeBase* e = root->getChildElement(i);
		if (e->getName() == "Default")
		{
			auto ext = e->getAttribute("Extension");
			auto c = e->getAttribute("ContentType");
			if (ext && c)
			{
				mExtensions[ext->getString()] = c->getString();
			}
		}
		else if (e->getName() == "Override")
		{
			auto pn = e->getAttribute("PartName");
			auto c = e->getAttribute("ContentType");
			if (pn && c)
			{
				mOverride[pn->getString()] = c->getString();
			}
		}
	}
}

std::string XLSXContentType::getContentType(const std::string name)
{
	auto f = mOverride.find(name);
	if (f != mOverride.end())
	{
		return (*f).second;
	}

	size_t foundext = name.rfind(".");
	if (foundext != std::string::npos)
	{
		std::string ext = name.substr(foundext + 1);

		f = mExtensions.find(ext);
		if (f != mExtensions.end())
		{
			return (*f).second;
		}
	}
	return "";
}


XMLBase* XLSXContentType::createXML()
{
	// TODO 
	return nullptr;
}

// relations

void XLSXRelationships::initFromXML(XMLBase* xml)
{
	mRelations.clear();

	XMLNodeBase* root = xml->getRoot();

	for (u32 i = 0; i < root->getChildCount(); i++)
	{
		XMLNodeBase* e = root->getChildElement(i);
		if (e->getName() == "Relationship")
		{
			auto aid = e->getAttribute("Id");
			auto t = e->getAttribute("Type");
			auto target = e->getAttribute("Target");
			if (aid && t && target)
			{
				RelationStruct toAdd(t->getString(), target->getString());
				mRelations[aid->getString()] = toAdd;
			}
		}
		
	}
}
