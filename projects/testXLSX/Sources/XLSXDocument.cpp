#include "XLSXDocument.h"
#include "FilePathManager.h"
#include "ModuleFileManager.h"
#include "miniz.h"

XLSXSheet	XLSXDocument::mBadSheet;

XLSXDocument::XLSXDocument() : XMLArchiveManager()
{

}

XLSXDocument::~XLSXDocument()
{
	
}

XLSXSheet* XLSXDocument::initSheet(const std::string& file, std::string name, int id)
{
	XLSXSheet* sheet = nullptr;
	XMLArchiveFile* xlsxfile = static_cast<XMLArchiveFile*>(mRoot.getFile(file));
	if (xlsxfile && xlsxfile->getXML())
	{
		sheet = new XLSXSheet(name, id,&mSharedStrings);
		sheet->initFromXML(xlsxfile->getXML());
	}
	return sheet;
}

void		XLSXDocument::initSharedStrings(const std::string& name)
{
	XMLArchiveFile* xlsxfile = static_cast<XMLArchiveFile*>(mRoot.getFile(name));
	if (xlsxfile && xlsxfile->getXML())
	{
		mSharedStrings.initFromXML(xlsxfile->getXML());
	}
}

void	XLSXDocument::initWorkbook(const std::string& name)
{
	
	XMLArchiveFile* xlsxfile = static_cast<XMLArchiveFile*>(mRoot.getFile(name));
	if (xlsxfile)
	{

		auto frels = mRels.find(name);
		if (frels != mRels.end())
		{
			XLSXRelationships& wb_rels = (*frels).second;

			// init shared strings first
			std::string sharedStrings = wb_rels.getTargetFromType("sharedStrings");
			if (sharedStrings.length())
			{
				sharedStrings = xlsxfile->getPath() + "/" + sharedStrings;
				initSharedStrings(sharedStrings);
			}

			XMLBase* wb_xml = xlsxfile->getXML();
			XMLNodeBase* root = wb_xml->getRoot();

			XMLNodeBase* sheets = root->getChildElement("sheets");

			for (u32 i = 0; i < sheets->getChildCount(); i++)
			{
				XMLNodeBase* e = sheets->getChildElement(i);
				if (e->getName() == "sheet")
				{
					auto sheetname = e->getAttribute("name");
					auto sheetid = e->getAttribute("sheetId");
					auto sheetrid = e->getAttribute("r:id");
					if (sheetname && sheetid && sheetrid)
					{
						std::string target=wb_rels.getTarget(sheetrid->getString());
						if (target.length())
						{
							// add current path

							target = xlsxfile->getPath() + "/" + target;

							XLSXSheet* sheet=initSheet(target, sheetname->getString(), sheetid->getInt());
							if (sheet)
							{
								mSheets.push_back(sheet);
							}

							

						}
					}
				}
			}

			std::sort(mSheets.begin(), mSheets.end(), [](const XLSXSheet* a, const XLSXSheet* b)->bool {
				if (a->mSheedID == b->mSheedID)
				{
					return a < b;
				}
				return a->mSheedID < b->mSheedID;

				});

		}
	}
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

					std::string concernedFile = f->getName(true);

					// remove .rels extension
					concernedFile.erase(concernedFile.length() - 5);

					// remove _rels/

					size_t p = concernedFile.find("_rels/");
					if (p != std::string::npos)
					{
						concernedFile.erase(p,6);
					}


					mRels[concernedFile].initFromXML(rels_xml);
				}
			}
		}

		// search document
		std::string workbook = mRels[""].getTargetFromType("officeDocument");
		if (workbook.length())
		{
			initWorkbook(workbook);
		}

		return true;
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

std::string XLSXContentType::getPartName(const std::string contenttype)
{
	for (auto f : mOverride)
	{
		if (f.second.find(contenttype) != std::string::npos)
		{
			return f.first;
		}
	}
	return "";
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

void  XLSXSharedStrings::initFromXML(XMLBase* xml)
{
	mSharedStrings.clear();

	XMLNodeBase* root = xml->getRoot();

	for (u32 i = 0; i < root->getChildCount(); i++)
	{
		XMLNodeBase* si = root->getChildElement(i);
		XMLNodeBase* t = si->getChildElement(0);
		if (t->getChildCount())
		{
			XMLNodeBase* txt = t->getChildElement(0);
			mSharedStrings.push_back(txt->getString());
		}
		else
		{
			mSharedStrings.push_back("");
		}
	}
}