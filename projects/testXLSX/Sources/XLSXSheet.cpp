#include "XLSXSheet.h"
#include "XLSXDocument.h"

void XLSXSheet::initFromXML(XMLBase* xml)
{
	mRows.clear();

	XMLNodeBase* worksheet = xml->getRoot();
	if (worksheet)
	{
		XMLNodeBase* dimension = worksheet->getChildElement("dimension");
		if (dimension)
		{
			auto ref = dimension->getAttribute("ref");
			std::string dimstr = ref->getString();
			auto range = XLSXSheet::getRange(dimstr);
			mRows.resize(range.second[1]+1);
			int rindex = 0;
			for (auto& r : mRows)
			{
				r.resize(range.second[0] + 1);
				r.mIndex = rindex;
				r.mSheet = this;
				
				int cindex = 0;
				for (auto& c : r)
				{
					c.mName.setIndex({ cindex,rindex });
					++cindex;
				}
				++rindex;
			}
		}
		// now get values
		XMLNodeBase* sheetData = worksheet->getChildElement("sheetData");
		if (sheetData)
		{
			for (u32 i = 0; i < sheetData->getChildCount(); i++)
			{
				XMLNodeBase* e = sheetData->getChildElement(i);
				if (e->getName() == "row")
				{
					auto rindex = e->getAttribute("r");
					int rowIndex = rindex->getInt();
					rowIndex--;
					if (rowIndex < mRows.size())
					{
						XLSXRow& currentR = mRows[rowIndex];
						for (u32 ci = 0; ci < e->getChildCount(); ci++)
						{
							XMLNodeBase* ce = e->getChildElement(ci);
							if (ce && ce->getChildCount())
							{
								XMLNodeBase* val = ce->getChildElement(0);
								if (val->getChildCount())
								{
									val=val->getChildElement(0);
								}
								auto cellname = ce->getAttribute("r");
								if (cellname)
								{
									if (cellname->getString() == currentR[ci].mName.getName())
									{
										auto type = ce->getAttribute("t");

										if (type && (type->getString() == "s"))
										{
											currentR[ci] = CoreItemSP::getCoreValue(mSharedStrings->getString(val->getInt()));
										}
										else
										{
											currentR[ci] = CoreItemSP::getCoreValue(val->getInt());
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void	CellName::interpretStringName()
{
	std::string col = getCol();
	std::string row = getRow();
	
	mIndexes[0] = XLSXSheet::getColIndex(col);
	mIndexes[1] = XLSXSheet::getRowIndex(row);
}

void	CellName::setIndex(const v2i& i)
{
	mIndexes = i;
	mName = XLSXSheet::getCellName(i[0], i[1]);
}

std::string CellName::getCol()
{
	return XLSXSheet::getColName(mName);
}
std::string	CellName::getRow()
{
	return XLSXSheet::getRowName(mName);
}

XMLBase* XLSXSheet::createXML()
{
	// TODO
	return nullptr;
}

// index starting at 0
std::string	XLSXSheet::getCellName(u32 col, u32 row)
{
	std::string name="";

	do
	{
		std::string newletter;
		newletter += ('A' + (col % 26));

		name = newletter + name;
		col /= 26;
	} 	while (col);

	row++;
	name+=std::to_string(row);

	return name;
}

// index starting from 0
u32	XLSXSheet::getColIndex(const std::string& colname)
{
	u32 index=0;
	u32 pos = 0;
	while (pos < colname.size())
	{
		index = index * 26 + (colname[pos] - 'A');
		pos++;
	}
	return index;
}

// index starting from 0
u32	XLSXSheet::getRowIndex(const std::string& rowname)
{
	u32 index=std::stoi(rowname);
	index--;
	return index;
}

std::pair<v2i, v2i>	XLSXSheet::getRange(const std::string& rangename)
{
	std::pair<v2i, v2i> result;
	v2i min(0,0);
	v2i max(0,0);
	size_t found = rangename.find(":");
	if (found != std::string::npos)
	{
		min=XLSXSheet::getCellPos(rangename.substr(0, found));
		max=XLSXSheet::getCellPos(rangename.substr(found+1));
	}
	result.first = min;
	result.second = max;
	return result;
}

v2i	XLSXSheet::getCellPos(const std::string& cellname)
{
	v2i result;
	result[0] = getColIndex(getColName(cellname));
	result[1] = getRowIndex(getRowName(cellname));
	return result;
}

std::string XLSXSheet::getColName(const std::string& cellname)
{
	int pos = 0;
	std::string colName = "";
	while (pos < cellname.length())
	{
		if ((cellname[pos] < 'A') || (cellname[pos] > 'Z'))
		{
			break;
		}
		colName += cellname[pos];
		pos++;
	}
	return colName;
}

std::string	XLSXSheet::getRowName(const std::string& cellname)
{
	int pos = 0;
	std::string rowName = "";
	while (pos < cellname.length())
	{
		if ((cellname[pos] >= 'A') && (cellname[pos] <= 'Z'))
		{
			pos++;
			continue;
		}
		rowName += cellname[pos];
		pos++;
	}
	return rowName;
}